#ifndef DATA_PAGE
#define DATA_PAGE

#define DATA_PAGE_SLOT_NUM 16

// use pm_address to locate the data in the page

// uncompressed page format design to store the buckets of PmEHash
// one slot stores one bucket of PmEHash
typedef struct data_page
{
    // fixed-size record design
    // uncompressed page format
    pm_bucket slot[DATA_PAGE_SLOT_NUM];
    uint16_t bitmap;
} data_page;

void makeDataDirectory();
void newEHashFiles();
void *mapFile(const char *path);
void *mapMetadata();
void *mapCatalog();

#endif
