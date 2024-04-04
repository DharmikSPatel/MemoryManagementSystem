#include "my_vm.h"
#include <stdio.h>
#include <stdlib.h>
#include <math.h>
#include <string.h>
#include "bitmap.h"

#define L1BITMASK ((1U << numOfLevel1Bits) - 1) << (numOfVAdrrBits - numOfLevel1Bits)
#define L2BITMASK ((1U << numOfLevel2Bits) - 1) << (numOfVAdrrBits - numOfLevel1Bits - numOfLevel2Bits)
#define OFFSETBITMASK ((1U << numOfOffsetBits) - 1)
#define VPAGENUMBITMASK ((1U << numOfVPNumBits) - 1) << (numOfVAdrrBits - numOfVPNumBits)

typedef struct TLBEntry
{
    unsigned int vpage;
    unsigned int ppage;
}tlb_entry;

void print_tlb_entry(tlb_entry* entry){
    printf("vpage |%u| -> ppage |%u|\n", entry->vpage, entry->ppage);
}

char* physical_mem = NULL;
struct bitmap* p_bitmap = NULL;
struct bitmap* v_bitmap = NULL;
unsigned int* l1_table = NULL;
int numOfVAdrrBits;
int numOfLevel1Bits;
int numOfLevel2Bits; //num of bits to index into 
int numOfOffsetBits; //num of bits to index into a page of size of PAGE_SIZE
int numOfVPNumBits;

unsigned int tlb_access_amount = 0;
unsigned int tlb_succes_amount = 0;
unsigned int tlb_miss_amount = 0;

tlb_entry* TLB[TLB_ENTRIES]; 

void set_physical_mem(){
    physical_mem = (char*)malloc(MEMSIZE);
    memset(physical_mem, 0, MEMSIZE);
    
    l1_table = (unsigned int*)physical_mem;
    numOfVAdrrBits = log2(MAX_MEMSIZE);
    numOfOffsetBits = log2(PAGE_SIZE);
    numOfVPNumBits = numOfVAdrrBits - numOfOffsetBits;
    numOfLevel2Bits = numOfOffsetBits - (log2(PTE_SIZE));
    numOfLevel1Bits = numOfVPNumBits - numOfLevel2Bits;

    unsigned int num_physical_pages = MEMSIZE/PAGE_SIZE;
    unsigned int num_virtual_page = pow(2,numOfVPNumBits); 
    p_bitmap = init_bm(num_physical_pages);
    v_bitmap = init_bm(num_virtual_page);
    use_page(p_bitmap, 0); //page 0 in physical mem used for outer page table
    use_page(v_bitmap, 0); //page 0 in virtaul mem maps to page 0 in physical mem

    for(int i = 0; i < TLB_ENTRIES; i++){
        TLB[i] = malloc(sizeof(tlb_entry));
    }
}
//used to debug
void decimalToBinary(int n)
{
    for (int i = 31; i >= 0; i--) {
        int k = n >> i; 
        if (k & 1) 
              printf("1");
        else printf("0");
    }
    printf("\n");
}
void debug_va(unsigned int vp){
    unsigned int l1Index = (vp & L1BITMASK) >> (numOfVAdrrBits - numOfLevel1Bits);
    unsigned int l2Index = (vp & L2BITMASK) >> (numOfVAdrrBits - numOfLevel1Bits - numOfLevel2Bits);
    unsigned int offset = (vp & OFFSETBITMASK);
    unsigned int vpage_num = (vp & VPAGENUMBITMASK) >> (numOfOffsetBits);
    printf("---- DEUGING VP ptr\n");
    decimalToBinary(vp);
    printf("MASKS: \n");
    decimalToBinary(L1BITMASK);
    decimalToBinary(L2BITMASK);
    decimalToBinary(OFFSETBITMASK);
    decimalToBinary(VPAGENUMBITMASK);
    printf("Values In Binary\n");
    decimalToBinary(vp);
    decimalToBinary(l1Index);
    decimalToBinary(l2Index);
    decimalToBinary(offset);
    decimalToBinary(vpage_num);
    printf("Values In deicmal\n");
    printf("l1Index: %u\n", l1Index);
    printf("l2Index: %u\n", l2Index);
    printf("offSet: %u\n", offset);
    printf("vp#: %u\n", vpage_num);
    printf("physical adress: |%p|\n", translate(vp));
    printf("----\n");
}
//used to print first n tlb entries
void printTLB(unsigned int n){
    printf("-----\n");
    printf("Printing TLB\n");
    for(int i = 0; i < n; i++){
        print_tlb_entry(TLB[i]);
    }
    printf("-----\n");
}
void * translate(unsigned int vp){
    unsigned int l1Index = (vp & L1BITMASK) >> (numOfVAdrrBits - numOfLevel1Bits);
    unsigned int l2Index = (vp & L2BITMASK) >> (numOfVAdrrBits - numOfLevel1Bits - numOfLevel2Bits);
    unsigned int offset = (vp & OFFSETBITMASK);
    unsigned int vpage_num = (vp & VPAGENUMBITMASK) >> (numOfOffsetBits);
    unsigned int page_num_bp;
    if(check_TLB(vpage_num)) {
        page_num_bp = TLB[vpage_num % TLB_ENTRIES]->ppage;
    } else {
        //else walk the page table 
        unsigned int page_num_l2Table = l1_table[l1Index];
        unsigned int* l2_table = (unsigned int*)(physical_mem + page_num_l2Table * PAGE_SIZE);        
        page_num_bp = l2_table[l2Index];
        add_TLB(vpage_num, page_num_bp);
    }    
    if(page_num_bp == 0) return NULL; //if zero page, then error
    char* bp = physical_mem + page_num_bp * PAGE_SIZE;    
    void* physical_addr = (void*)(bp + offset); 
    return physical_addr;
}

unsigned int page_map(unsigned int vp){
    unsigned int l1Index = (vp & L1BITMASK) >> (numOfVAdrrBits - numOfLevel1Bits);
    unsigned int l2Index = (vp & L2BITMASK) >> (numOfVAdrrBits - numOfLevel1Bits - numOfLevel2Bits);
    unsigned int page_num_l2Table = l1_table[l1Index];
    if(page_num_l2Table == 0) {
        if((page_num_l2Table = next_free_page(p_bitmap)) == 0){
            printf("Not Enough Physical Pages\n");
            exit(EXIT_FAILURE);
        }
        use_page(p_bitmap, page_num_l2Table);
        l1_table[l1Index] = page_num_l2Table;
    }
    unsigned int* l2_table = (unsigned int*)(physical_mem + page_num_l2Table * PAGE_SIZE);
    unsigned int page_num_bp = l2_table[l2Index];
    if(page_num_bp == 0) {
        if((page_num_bp = next_free_page(p_bitmap)) == 0){
            printf("Not Enough Physical Pages\n");
            exit(EXIT_FAILURE);
        }
        // use_page(p_bitmap, page_num_bp);
        l2_table[l2Index] = page_num_bp;
    }
    return page_num_bp;
}
unsigned int calculate_amount_pages_needed(size_t n){
    unsigned int needed;
    if (n % PAGE_SIZE == 0) 
        needed = n / PAGE_SIZE;
    else
        needed = n / PAGE_SIZE + 1;

    if(needed >= p_bitmap->num_of_pages){
        printf("Not Enough Physical Mem to Allocate\n");
        exit(EXIT_FAILURE);
    }
    return needed;
}
void * t_malloc(size_t n){
    unsigned int pages_needed = calculate_amount_pages_needed(n);
    // printf("Amount Of Pages Needed |%u| for |%d|Bytes\n", pages_needed, n);
    unsigned int v_page_num_start = next_n_free_pages(v_bitmap, pages_needed);
    if(v_page_num_start == 0){
        printf("Not Enough Virtual Pages\n");
        exit(EXIT_FAILURE);
    }
    unsigned int v_address_start = (v_page_num_start << numOfOffsetBits);
    for(unsigned int i = 0; i < pages_needed; i++){
        unsigned int vpage_num = v_page_num_start + i;
        use_page(v_bitmap, vpage_num);
        unsigned int ppage_num = page_map((vpage_num << numOfOffsetBits));
        use_page(p_bitmap, ppage_num);
    }
    return (void *)v_address_start;
}
int page_unmap(unsigned int vp){
    //mark vpage as free
    //break the connection from l2 at l2index to physical memory
    //mark ppage as free
    unsigned int l1Index = (vp & L1BITMASK) >> (numOfVAdrrBits - numOfLevel1Bits);
    unsigned int l2Index = (vp & L2BITMASK) >> (numOfVAdrrBits - numOfLevel1Bits - numOfLevel2Bits);
    unsigned int vpage_num = (vp & VPAGENUMBITMASK) >> (numOfOffsetBits);

    unsigned int page_num_l2Table = l1_table[l1Index];
    if(page_num_l2Table == 0) {
        printf("Attempting Free on non malloc ptr\n");
        return -1;
    }
    unsigned int* l2_table = (unsigned int*)(physical_mem + page_num_l2Table * PAGE_SIZE);
    unsigned int ppage_num = l2_table[l2Index];
    l2_table[l2Index] = 0;
    free_page(v_bitmap, vpage_num);
    free_page(p_bitmap, ppage_num);
    return 0;
}
int t_free(unsigned int vp, size_t n){
    unsigned int v_page_num_start = (vp & VPAGENUMBITMASK) >> (numOfOffsetBits);
    unsigned int pages_needed = calculate_amount_pages_needed(n);
    // printf("Going to free |%u|pages for |%d|Bytes\n", pages_needed, n);
    for(int i = 0; i < pages_needed; i++){
        unsigned int vp = (v_page_num_start + i) << numOfOffsetBits;
        if(page_unmap(vp) != 0){
            printf("Free Failed\n");
            return -1;
        }
    }
    return 0;
}

int put_value(unsigned int vp, void *val, size_t n){
    //caller must have called malloc b4
    void* physicalAddress = translate(vp);
    if(physicalAddress == NULL) return -1;
    memcpy(physicalAddress, val, n);
    return 0;
}

int get_value(unsigned int vp, void *dst, size_t n){
    //caller must have called malloc b4
    void* physicalAddress = translate(vp);
    if(physicalAddress == NULL) return -1;
    memcpy(dst, physicalAddress, n);
    return 0;
}
unsigned int index_into(unsigned int matrix, int i, int j, int num_cols){
    return matrix + (i * ELEM_SIZE * num_cols) + j * ELEM_SIZE; 
}
void mat_mult(unsigned int a, unsigned int b, unsigned int c, size_t l, size_t m, size_t n){
    //A = l x m
    //B = m x n
    //C = l x n 
    // i,j = ArrName + (i * INT * NUM_COLS) + j * INT
    // NUM_COLS == SIZE_OF_ROWS == WIDTH OF 2DARRAY
    // printf("Resultant Matrix is:\n");
    for (int i = 0; i < l; i++) {
        for (int j = 0; j < n; j++) {
            // c[i][j] = 0;
            int zero = 0;
            put_value(
                index_into(c, i, j, n), 
                &zero, 
                ELEM_SIZE);
            
            for (int k = 0; k < m; k++) {
                // c[i][j] += a[i][k] * b[k][j];
                int a_ik;
                int b_kj;
                int c_ij;
                get_value(
                    index_into(a, i, k, m),
                    &a_ik, 
                    ELEM_SIZE);
                get_value(
                    index_into(b, k, j, n),
                    &b_kj, 
                    ELEM_SIZE);
                get_value(
                    index_into(c, i, j, n),
                    &c_ij, 
                    ELEM_SIZE);
                c_ij += a_ik * b_kj;
                put_value(
                    index_into(c, i, j, n), 
                    &c_ij, 
                    ELEM_SIZE);
            }
            // printf("%d ", c[i][j]);
        }
        // printf("\n");
    }
}

void add_TLB(unsigned int vpage, unsigned int ppage){
    int index = vpage % TLB_ENTRIES;
    //no matter what is there, evict the old value, 
    //bc this is only called if checktlb has falied
    tlb_entry* entry = TLB[index];
    entry->vpage = vpage;
    entry->ppage = ppage;
}

int check_TLB(unsigned int vpage){
    int index = vpage % TLB_ENTRIES;
    tlb_entry* entry = TLB[index];
    tlb_access_amount++;
    if(entry->vpage == vpage){
        tlb_succes_amount++;
        return 1;
    }
    else {
        tlb_miss_amount++;
        return 0;
    }
}

void print_TLB_missrate(){
    printf("-----\n");
    printf("Hits |%u| Miss |%u| Total |%u|\n", tlb_succes_amount, tlb_miss_amount, tlb_access_amount);
    printf("TLB Hit Rate: %f\n", (float)tlb_succes_amount/tlb_access_amount*100);
    printf("TLB Miss Rate: %f\n", (float)tlb_miss_amount/tlb_access_amount*100);
    printf("-----\n");
}
void printVABreakdown(){
    printf("-----\n");
    printf("VAdrr Size %d\n", numOfVAdrrBits);
    printf("L1 Bits %d\n", numOfLevel1Bits);
    printf("L2 Bits %d\n", numOfLevel2Bits);
    printf("Offset Bits %d\n", numOfOffsetBits);
    printf("-----\n");
}
// int main(int argc, char const *argv[])
// {
//     printf("COMIPLED\n");
//     return 0;
// }
