#include <stdlib.h>
#include <string.h>
#include <math.h>
#include <stdio.h>
#include "bitmap.h"

#define BYTE_INDEX(X) (X/8)
#define BIT_INDEX(X) (X%8)
void debug_bitmap(struct bitmap* bitmap){
    printf("%p\n", bitmap);
    printf("\tBitmap->num_of_pages: %u\n", bitmap->num_of_pages);
    printf("\tBitmap->_size: %u\n", bitmap->_bitmap_size);
}
struct bitmap* init_bm(unsigned int num_of_pages) {
    struct bitmap* bitmap = malloc(sizeof(struct bitmap));
    bitmap->num_of_pages = num_of_pages;
    bitmap->_bitmap_size = num_of_pages/8;
    bitmap->bm = malloc(bitmap->_bitmap_size);
    memset(bitmap->bm, 0, bitmap->_bitmap_size);
    return bitmap;
}
void free_bm(struct bitmap* bitmap) {
    free(bitmap->bm);
}
unsigned int next_free_page(struct bitmap* bitmap){
    //loop through whole bitmap at character level
    for(unsigned int i = 0; i < bitmap->_bitmap_size; i++){
        //BUT per char is bitwise operations
        char negated = ~(bitmap->bm[i]);
        unsigned int loc = log2(negated & -negated);
        if(negated != 0){
            return loc + i * 8;
        }
    }
    printf("No Free Page Found\n");
    return 0;
}
// ONLY USE WITH VIRTUAL BITMAP
unsigned int next_n_free_pages(struct bitmap* bitmap, unsigned int n){
    unsigned int num_of_free_found = 0;
    unsigned int i = next_free_page(bitmap);
    unsigned int start = i;
    while(num_of_free_found != n){
        if(i > bitmap->num_of_pages){
            return 0;
        }
        if(get_page(bitmap, i) == 0){
            num_of_free_found++;
        }
        else{
            num_of_free_found = 0;
            start = i+1;
        }
        i++;
    }
    return start;
}


void use_page(struct bitmap* bitmap, unsigned int page_num){
    if (page_num >= 0 && page_num < bitmap->num_of_pages) {
        int byteIndex = BYTE_INDEX(page_num);  // Determine the index of the byte in the array
        int bitIndex = BIT_INDEX(page_num);   // Determine the index of the bit within the byte
        bitmap->bm[byteIndex] |= 1 << bitIndex; //mark as in use
    } else {
        printf("Invalid page number %u\n", page_num);
    } 
}

void free_page(struct bitmap* bitmap, unsigned int page_num){
    if (page_num >= 0 && page_num < bitmap->num_of_pages) {
        int byteIndex = BYTE_INDEX(page_num);  // Determine the index of the byte in the array
        int bitIndex = BIT_INDEX(page_num);   // Determine the index of the bit within the byte
        bitmap->bm[byteIndex] &= ~(1 << bitIndex); //mark as free
    } else {
        printf("Invalid page number %u\n", page_num);
    }  
}

unsigned int get_page(struct bitmap* bitmap, unsigned int page_num){
    if (page_num >= 0 && page_num < bitmap->num_of_pages) {
        int byteIndex = BYTE_INDEX(page_num);  // Determine the index of the byte in the array
        int bitIndex = BIT_INDEX(page_num);   // Determine the index of the bit within the byte
       return (bitmap->bm[byteIndex] >> bitIndex) & 1;  // get the bit at the specified index
    } else {
        printf("Invalid page number %u\n", page_num);
        return -1;
    }
}
void print_bitmap(struct bitmap* bitmap, unsigned int n){
    printf("-------\n");
    //for(int i = bitmap->_bitmap_size-1; i >= 0; i--){
    for(unsigned int i = 0; i < n; i++){
        char a = bitmap->bm[i];
        int i;
        for (i = 7; i >= 0; i--) {
            printf("%d", !!((a << i) & 0x80));
        }
        printf(" ");
    }
    printf("\n");
    for(unsigned int i = 0; i < n; i++){
        char a = bitmap->bm[i];
        printf("|%c| ", a);
    }
    printf("-------\n");
}


