#include"pm_ehash.h"
#include <cstring>

/**
 * @description: construct a new instance of PmEHash in a default directory
 * @param NULL
 * @return: new instance of PmEHash
 */
PmEHash::PmEHash() {
	makeDataDirectory();
	if (recover()) {
		printf("WHAT???\n");
		return;
	}
	newEHashFiles();
	recover();
}
/**
 * @description: persist and munmap all data in NVM
 * @param NULL 
 * @return: NULL
 */
PmEHash::~PmEHash() {

}

/**
 * @description: 插入新的键值对，并将相应位置上的位图置1
 * @param kv: 插入的键值对
 * @return: 0 = insert successfully, -1 = fail to insert(target data with same key exist)
 */
int PmEHash::insert(kv new_kv_pair) {
	uint64_t val;
	if (search(new_kv_pair.key, val) == 0) {
		return -1;
	}
	pm_bucket *bucket = getFreeBucket(new_kv_pair.key);
	kv *slot = getFreeKvSlot(bucket);
	*slot = new_kv_pair;
	int idx = slot - bucket->slot;
	bucket->set(idx);
    return 0;
}

/**
 * @description: 删除具有目标键的键值对数据，不直接将数据置0，而是将相应位图置0即可
 * @param uint64_t: 要删除的目标键值对的键
 * @return: 0 = removing successfully, -1 = fail to remove(target data doesn't exist)
 */
int PmEHash::remove(uint64_t key) {
	uint64_t bid = getCatalogIdx(key);
	pm_bucket *bucket = catalog.buckets_virtual_address[bid];
	int idx = getKeyIdx(bucket, key);
	if (idx == -1) {
		return -1;
	}
	bucket->reset(idx);
    return 0;
}
/**
 * @description: 更新现存的键值对的值
 * @param kv: 更新的键值对，有原键和新值
 * @return: 0 = update successfully, -1 = fail to update(target data doesn't exist)
 */
int PmEHash::update(kv kv_pair) {
	uint64_t bid = getCatalogIdx(kv_pair.key);
	pm_bucket *bucket = catalog.buckets_virtual_address[bid];
	int idx = getKeyIdx(bucket, kv_pair.key);
	if (idx == -1) {
		return -1;
	}
	bucket->slot[idx].value = kv_pair.value;
    return 0;
}
/**
 * @description: 查找目标键值对数据，将返回值放在参数里的引用类型进行返回
 * @param uint64_t: 查询的目标键
 * @param uint64_t&: 查询成功后返回的目标值
 * @return: 0 = search successfully, -1 = fail to search(target data doesn't exist) 
 */
int PmEHash::search(uint64_t key, uint64_t& return_val) {
	uint64_t bid = getCatalogIdx(key);
	const pm_bucket *bucket = catalog.buckets_virtual_address[bid];
	int idx = getKeyIdx(bucket, key);
	if (idx == -1) {
		return -1;
	}
	return_val = bucket->slot[idx].value;
    return 0;
}

int PmEHash::getKeyIdx(const pm_bucket *bucket, uint64_t key) const {
	for (int i = 0; i < BUCKET_SLOT_NUM; ++i) {
		if (bucket->get(i) && bucket->slot[i].key == key) {
			return i;
		}
	}
	return -1;
}

uint64_t PmEHash::hashFunc(uint64_t key) {
	// TODO: 用一个更好的哈希
	return key;
}

uint64_t PmEHash::getLowBits(uint64_t target, size_t numBits) {
	return target & ((1ULL << numBits) - 1);
}

/**
 * @description: 用于对输入的键产生哈希值，然后取模求桶号(自己挑选合适的哈希函数处理)
 * @param uint64_t: 输入的键
 * @return: 返回键所属的桶号
 */
uint64_t PmEHash::getCatalogIdx(uint64_t key) {
	return getLowBits(hashFunc(key), metadata->global_depth);
}

/**
 * @description: 获得供插入的空闲的桶，无空闲桶则先分裂桶然后再返回空闲的桶
 * @param uint64_t: 带插入的键
 * @return: 空闲桶的虚拟地址
 */
pm_bucket* PmEHash::getFreeBucket(uint64_t key) {
	uint64_t bid = getCatalogIdx(key);
	pm_bucket *bucket = catalog.buckets_virtual_address[bid];
	while (bucket->isFull()) {
		splitBucket(getLowBits(bid, bucket->local_depth));
		bid = getCatalogIdx(key);
		bucket = catalog.buckets_virtual_address[bid];
	}
	return bucket;
}

/**
 * @description: 获得空闲桶内第一个空闲的位置供键值对插入
 * @param pm_bucket* bucket
 * @return: 空闲键值对位置的虚拟地址
 */
kv* PmEHash::getFreeKvSlot(pm_bucket* bucket) {
	// TODO: can be optimized
	for (int i = 0; i < BUCKET_SLOT_NUM; ++i) {
		if (!bucket->get(i)) {
			return bucket->slot + i;
		}
	}
	return nullptr;
}

/**
 * @description: 桶满后进行分裂操作，可能触发目录的倍增
 * @param uint64_t: 目标桶在目录中的序号
 * @return: NULL
 */
void PmEHash::splitBucket(uint64_t bucket_id) {
	pm_bucket *bucket = catalog.buckets_virtual_address[bucket_id];
	if (bucket->local_depth == metadata->global_depth) {
		extendCatalog();
	}
	++bucket->local_depth;
	const uint64_t ldepth = bucket->local_depth;
	const uint64_t cucket_id = bucket_id | (1 << (ldepth - 1));

	pm_bucket *cucket = (pm_bucket*)getFreeSlot(catalog.buckets_pm_address[cucket_id]);
	cucket->local_depth = ldepth;
	memset(cucket->bitmap, 0, sizeof(cucket->bitmap));

	/* 新增的桶的子树全部指向新增的桶 */
	uint64_t lmt = 1ULL << (metadata->global_depth - ldepth);
	uint64_t tid;
	for (uint64_t hi = 0; hi < lmt; ++hi) {
		tid = (hi << ldepth) | cucket_id;
		catalog.buckets_pm_address[tid] = catalog.buckets_pm_address[cucket_id];
		catalog.buckets_virtual_address[tid] = cucket;
	}

	/* 把原来的桶中部分元素移入新桶 */
	int num = 0;
	for (int i = 0; i < BUCKET_SLOT_NUM; ++i) {
		if (!bucket->get(i)) {
			continue;
		}
		if (getLowBits(hashFunc(bucket->slot[i].key), ldepth) == bucket_id) {
			continue;
		}
		cucket->slot[num] = bucket->slot[i];
		bucket->reset(i);
		cucket->set(num);
		++num;
	}
}

/**
 * @description: 桶空后，回收桶的空间，并设置相应目录项指针
 * @param uint64_t: 桶号
 * @return: NULL
 */
void PmEHash::mergeBucket(uint64_t bucket_id) {
    
}

/**
 * @description: 对目录进行倍增，需要重新生成新的目录文件并复制旧值，然后删除旧的目录文件
 * @param NULL
 * @return: NULL
 */
void PmEHash::extendCatalog() {
	++metadata->global_depth;
	metadata->catalog_size *= 2;
	doubleCatalog(catalog.buckets_pm_address, metadata->catalog_size);

	/* 倍增内存中的虚拟地址 */
	pm_bucket **new_virtual_address = new pm_bucket*[metadata->catalog_size];
	memcpy(new_virtual_address, catalog.buckets_virtual_address, sizeof(pm_bucket*) * metadata->catalog_size / 2);
	memcpy(new_virtual_address + metadata->catalog_size / 2, catalog.buckets_virtual_address, sizeof(pm_bucket*) * metadata->catalog_size / 2);
	delete[] catalog.buckets_virtual_address;
	catalog.buckets_virtual_address = new_virtual_address;
}

/**
 * @description: 获得一个可用的数据页的新槽位供哈希桶使用，如果没有则先申请新的数据页
 * @param pm_address&: 新槽位的持久化文件地址，作为引用参数返回
 * @return: 新槽位的虚拟地址
 */
void* PmEHash::getFreeSlot(pm_address& new_address) {
	if (free_list.empty()) {
		allocNewPage();
	}
	pm_bucket *bucket = free_list.front();
	free_list.pop();
	new_address = vAddr2pmAddr[bucket];
	pages[new_address.fileId]->bitmap |= (1 << new_address.offset);
	return bucket;
}

/**
 * @description: 申请新的数据页文件，并把所有新产生的空闲槽的地址放入free_list等数据结构中
 * @param NULL
 * @return: NULL
 */
void PmEHash::allocNewPage() {
	const uint64_t n = metadata->max_file_id;
	data_page **new_pages = new data_page*[n * 2];
	memcpy(new_pages, pages, sizeof(data_page*) * n);
	delete[] pages;
	pages = new_pages;
	pages[n] = newPage(n);

	pm_address pmaddr;
	pmaddr.fileId = n;

	for (int j = 0; j < DATA_PAGE_SLOT_NUM; ++j) {
		pmaddr.offset = j;
		pm_bucket *vaddr = pages[n]->slot + j;
		pmAddr2vAddr[pmaddr] = vaddr;
		vAddr2pmAddr[vaddr] = pmaddr;
		free_list.push(pages[n]->slot + j);
	}

	++metadata->max_file_id;
}

/**
 * @description: 读取旧数据文件重新载入哈希，恢复哈希关闭前的状态
 * @param NULL
 * @return: 是否成功
 */
bool PmEHash::recover() {
	metadata = (ehash_metadata*)mapMetadata();
	if (metadata == nullptr) {
		return false;
	}
	catalog.buckets_pm_address = (pm_address*)mapCatalog();
	if (catalog.buckets_pm_address == nullptr) {
		return false;
	}
	mapAllPage();
	catalog.buckets_virtual_address = new pm_bucket*[metadata->catalog_size];
	int i;
	for (i = 0; i < metadata->catalog_size; ++i) {
		catalog.buckets_virtual_address[i] = pmAddr2vAddr[catalog.buckets_pm_address[i]];
	}
	return true;
}

/**
 * @description: 重启时，将所有数据页进行内存映射，设置地址间的映射关系，空闲的和使用的槽位都需要设置 
 * @param NULL
 * @return: 是否成功
 */
bool PmEHash::mapAllPage() {
	char tems[15];
	int i;
	pm_address pmaddr;
	pages = new data_page*[metadata->max_file_id];
	for (i = 0; i < metadata->max_file_id; ++i) {
		sprintf(tems, "%s/%d", PM_EHASH_DIRECTORY, i);
		pages[i] = (data_page*)mapFile(tems);
		if (pages[i] == nullptr) {
			return false;
		}
		pmaddr.fileId = i;
		int j;
		for (j = 0; j < DATA_PAGE_SLOT_NUM; ++j) {
			pmaddr.offset = j;
			pmAddr2vAddr[pmaddr] = pages[i]->slot + j;
			vAddr2pmAddr[pages[i]->slot + j] = pmaddr;
			if ((pages[i]->bitmap >> j & 1) ^ 1) {
				free_list.push(pages[i]->slot + j);
			}
		}
	}
	return true;
}

/**
 * @description: 删除PmEHash对象所有数据页，目录和元数据文件，主要供gtest使用。即清空所有可扩展哈希的文件数据，不止是内存上的
 * @param NULL
 * @return: NULL
 */
void PmEHash::selfDestory() {
	clearAll();
}
