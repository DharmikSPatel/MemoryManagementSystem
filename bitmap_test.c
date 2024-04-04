#include <string.h>
#include <stdio.h>
#include "bitmap.h"
int main(int argc, char const *argv[])
{
    struct bitmap* bitmap = init_bm(64);
    print_bitmap(bitmap, 64);
    printf("Next Free: %u\n", next_free_page(bitmap));

    for(int i = 0; i < 64; i++){
        use_page(bitmap, i);
    }
    print_bitmap(bitmap, 64);
    printf("Next Free: %u\n", next_free_page(bitmap));
    for(int i = 10; i < 50; i++){
        free_page(bitmap, i);
    }
    for(int i = 2; i < 5; i++){
        free_page(bitmap, i);
    }
    for(int i = 15; i < 41; i++){
        use_page(bitmap, i);
    }
    print_bitmap(bitmap, 64);
    printf("Next Free: %u\n", next_free_page(bitmap));
    printf("Next Free: %u\n", next_n_free_pages(bitmap, 1));
    return 0;
}
