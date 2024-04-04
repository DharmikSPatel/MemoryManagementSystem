#include <stddef.h>

#define MAX_MEMSIZE (1UL<<32)
#define MEMSIZE (1UL<<30)
#define TLB_ENTRIES 256
#define PAGE_SIZE (8192)  //8K = 8192 
#define PTE_SIZE 4 //4 Byte so use usigned int for indexing into pages. can represent 2^32 values
#define ELEM_SIZE (sizeof(int)) //for matmul

void set_physical_mem();

void * translate(unsigned int vp);

unsigned int page_map(unsigned int vp);

void * t_malloc(size_t n);

int t_free(unsigned int vp, size_t n);

int put_value(unsigned int vp, void *val, size_t n);

int get_value(unsigned int vp, void *dst, size_t n);

void mat_mult(unsigned int a, unsigned int b, unsigned int c, size_t l, size_t m, size_t n);

void add_TLB(unsigned int vpage, unsigned int ppage);

int check_TLB(unsigned int vpage);

void print_TLB_missrate();

unsigned int index_into(unsigned int matrix, int i, int j, int num_cols);
