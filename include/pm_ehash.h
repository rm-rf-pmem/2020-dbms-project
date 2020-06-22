#ifndef _PM_E_HASH_H
#define _PM_E_HASH_H

#include<cstdint>
#include<queue>
#include<map>
#include <unordered_map>
// #include"data_page.h"

#define BUCKET_SLOT_NUM               15
#define DEFAULT_CATALOG_SIZE      16
#define META_NAME                                "pm_ehash_metadata"
#define CATALOG_NAME                        "pm_ehash_catalog"
#define PM_EHASH_DIRECTORY        "./data"        // add your own directory path to store the pm_ehash

using std::queue;
using std::map;
using std::unordered_map;

/* 
---the physical address of data in NVM---
fileId: 1-N, the data page name
offset: data offset in the file
*/
typedef struct pm_address
{
    uint32_t fileId;
    uint32_t offset;
	bool operator<(const pm_address & rhs) const {
		if (fileId != rhs.fileId) {
			return fileId < rhs.fileId;
		}
		return offset < rhs.offset;
	}
} pm_address;

/*
the data entry stored by the  ehash
*/
typedef struct kv
{
    uint64_t key;
    uint64_t value;
} kv;

typedef struct pm_bucket
{
    uint64_t local_depth;
    uint8_t  bitmap[BUCKET_SLOT_NUM / 8 + 1];      // one bit for each slot
    kv       slot[BUCKET_SLOT_NUM];                                // one slot for one kv-pair
	bool get(int p) const {
		return bitmap[p / 8] >> (p % 8) & 1;
	}
	void set(int p) {
		bitmap[p / 8] |= (1 << (p % 8));
	}
	void reset(int p) {
		bitmap[p / 8] &= (~(1 << (p % 8)));
	}
	bool isFull() const {
		const int lmt = sizeof(bitmap) / sizeof(bitmap[0]) - 1;
		for (int i = 0; i < lmt; ++i) {
			if (bitmap[i] != 0xFF) {
				return false;
			}
		}
		int left = BUCKET_SLOT_NUM - lmt * 8;
		if (bitmap[lmt] != ((1 << left) - 1)) {
			return false;
		}
		return true;
	}
} pm_bucket;

// in ehash_catalog, the virtual address of buckets_pm_address[n] is stored in buckets_virtual_address
// buckets_pm_address: open catalog file and store the virtual address of file
// buckets_virtual_address: store virtual address of bucket that each buckets_pm_address points to
typedef struct ehash_catalog
{
    pm_address* buckets_pm_address;         // pm address array of buckets
    pm_bucket** buckets_virtual_address;    // virtual address of buckets that buckets_pm_address point to
} ehash_catalog;

typedef struct ehash_metadata
{
    uint64_t max_file_id;      // next file id that can be allocated
    uint64_t catalog_size;     // the catalog size of catalog file(amount of data entry)
    uint64_t global_depth;   // global depth of PmEHash
} ehash_metadata;

struct data_page;

class PmEHash
{
private:
    
    ehash_metadata*                               metadata;                    // virtual address of metadata, mapping the metadata file
    ehash_catalog                                      catalog;                        // the catalog of hash

    queue<pm_bucket*>                         free_list;                      //all free slots in data pages to store buckets
    map<pm_bucket*, pm_address> vAddr2pmAddr;       // map virtual address to pm_address, used to find specific pm_address
    map<pm_address, pm_bucket*> pmAddr2vAddr;       // map pm_address to virtual address, used to find specific virtual address
	data_page **pages;
    
	uint64_t hashFunc(uint64_t key);
	uint64_t getLowBits(uint64_t target, size_t numBits);
    uint64_t getCatalogIdx(uint64_t key);

    pm_bucket* getFreeBucket(uint64_t key);
    pm_bucket* getNewBucket();
    void freeEmptyBucket(pm_bucket* bucket);
    kv* getFreeKvSlot(pm_bucket* bucket);

    void splitBucket(uint64_t bucket_id);
    void mergeBucket(uint64_t bucket_id);

    void extendCatalog();
    void* getFreeSlot(pm_address& new_address);
    void allocNewPage();

    bool recover();
    bool mapAllPage();

	int getKeyIdx(const pm_bucket * bucket, uint64_t key) const;

public:
    PmEHash();
    ~PmEHash();

    int insert(kv new_kv_pair);
    int remove(uint64_t key);
    int update(kv kv_pair);
    int search(uint64_t key, uint64_t& return_val);

    void selfDestory();
};









#define DATA_PAGE_SLOT_NUM 16

// use pm_address to locate the data in the page

// uncompressed page format design to store the buckets of PmEHash
// one slot stores one bucket of PmEHash
typedef struct data_page {
    // fixed-size record design
    // uncompressed page format
	pm_bucket slot[DATA_PAGE_SLOT_NUM];
	uint16_t bitmap;
} data_page;

void makeDataDirectory();
void newEHashFiles();
void* mapFile(const char *path);
void* mapMetadata();
void* mapCatalog();
void clearAll();
void doubleCatalog(pm_address *&addr, uint64_t new_size );
data_page* newPage(uint64_t id);

#endif
