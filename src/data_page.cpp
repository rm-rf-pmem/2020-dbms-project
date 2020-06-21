#include"pm_ehash.h"

// 数据页表的相关操作实现都放在这个源文件下，如PmEHash申请新的数据页和删除数据页的底层实现

#include <libpmem.h>
#include <cstdlib>
#include <cstdio>
#include <cstring>

static char tems[100];

void makeDataDirectory() {
	sprintf(tems, "mkdir -p %s", PM_EHASH_DIRECTORY);
	system(tems);
}

void newEHashFiles() {
	sprintf(tems, "rm -f %s/*", PM_EHASH_DIRECTORY);
	system(tems);
	sprintf(tems, "%s/%s", PM_EHASH_DIRECTORY, META_NAME);
	size_t map_len;
	int is_pmem;
	ehash_metadata *metadata = (ehash_metadata*)pmem_map_file(tems, sizeof(ehash_metadata), PMEM_FILE_CREATE, 0777, &map_len, &is_pmem);
	metadata->max_file_id = 1;
	metadata->catalog_size = DEFAULT_CATALOG_SIZE;
	metadata->global_depth = 4;
	pmem_persist(metadata, map_len);
	pmem_unmap(metadata, map_len);

	sprintf(tems, "%s/0", PM_EHASH_DIRECTORY);
	data_page *page = (data_page*)pmem_map_file(tems, sizeof(data_page), PMEM_FILE_CREATE, 0777, &map_len, &is_pmem);
	page->bitmap = 0xFFFF;
	for (int i = 0; i < DATA_PAGE_SLOT_NUM; ++i) {
		page->slot[i].local_depth = 4;
		memset(page->slot[i].bitmap, 0, sizeof page->slot[0].bitmap);
	}
	pmem_persist(page, map_len);
	pmem_unmap(page, map_len);

	sprintf(tems, "%s/%s", PM_EHASH_DIRECTORY, CATALOG_NAME);
	pm_address *catalog = (pm_address*)pmem_map_file(tems, sizeof(pm_address) * DEFAULT_CATALOG_SIZE, PMEM_FILE_CREATE, 0777, &map_len, &is_pmem);
	for (int i = 0; i < DEFAULT_CATALOG_SIZE; ++i) {
		catalog[i].fileId = 0;
		catalog[i].offset = i;
	}
	pmem_persist(catalog, map_len);
	pmem_unmap(catalog, map_len);
}

void* mapFile(const char *path) {
	return pmem_map_file(path, 0, 0, 0, nullptr, nullptr);
}

void* mapMetadata() {
	sprintf(tems, "%s/%s", PM_EHASH_DIRECTORY, META_NAME);
	return mapFile(tems);
}

void* mapCatalog() {
	sprintf(tems, "%s/%s", PM_EHASH_DIRECTORY, CATALOG_NAME);
	return mapFile(tems);
}

void clearAll() {
	sprintf(tems, "rm -f %s/*", PM_EHASH_DIRECTORY);
	system(tems);
}
