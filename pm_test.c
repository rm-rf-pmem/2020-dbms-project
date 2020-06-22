#include<libpmem.h>
#include<stdint.h>
#include<stdio.h>

typedef struct data
{
    int v1;
    int v2;
} data;


int main() {
    size_t map_len;
    int is_pmem;
	data *tdata;
    tdata = pmem_map_file("file", sizeof(data), PMEM_FILE_CREATE, 0777, &map_len, &is_pmem);
	printf("%lu %d\n", map_len, is_pmem);
    tdata->v1 = 10;
    tdata->v2 = 20;
    pmem_persist(tdata, map_len);
    pmem_unmap(tdata, map_len);

//    tdata = pmem_map_file("file", sizeof(data), PMEM_FILE_CREATE, 0777, &map_len, &is_pmem);
    tdata = pmem_map_file("file", 0, 0, 0, NULL, NULL);
	printf(" b4 ++\n");
	tdata->v1++;
//	int tem = tdata->v2;
//	tdata->v1 = tem + 1;
	printf(" aft ++\n");
	printf("%d %d\n", tdata->v1, tdata->v2);
	pmem_persist(tdata, map_len);
    pmem_unmap(tdata, map_len);
    return 0;
}
