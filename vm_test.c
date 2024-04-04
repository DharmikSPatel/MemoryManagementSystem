
#include <stdio.h>
#include <stdlib.h>
#include "my_vm.h"


// void test_malloc(){
//     unsigned int* v_ptr = t_malloc(4);
//     debug_va((unsigned int) v_ptr);
//     v_ptr = t_malloc(PAGE_SIZE);
//     debug_va((unsigned int) v_ptr);
//     v_ptr = t_malloc(PAGE_SIZE+1);
//     debug_va((unsigned int) v_ptr);
//     v_ptr = t_malloc(4);
//     debug_va((unsigned int) v_ptr);
// }
// void test_malloc_with_get_put_vals(){
//     int* v_ptr = t_malloc(sizeof(float));
//     debug_va((unsigned int) v_ptr);
//     float val = 10.25;
//     float dst;
//     put_value((unsigned int)v_ptr, &val, sizeof(float));
//     get_value((unsigned int)v_ptr, &dst, sizeof(float));
//     printf("Got |%f|\n", dst);
// }
// void test_free(){
//     // print_bitmap(p_bitmap, 8);
//     // print_bitmap(v_bitmap, 8);
//     int* ptr = t_malloc(sizeof(int));
//     debug_va((unsigned int) ptr);
//     // t_free((unsigned int)ptr, sizeof(int));

//     int* ptr1 = t_malloc(PAGE_SIZE*2);
//     debug_va((unsigned int) ptr1);
//     // t_free((unsigned int)ptr, sizeof(int));

//     int* ptr2 = t_malloc(PAGE_SIZE);
//     debug_va((unsigned int) ptr2);

//     // print_bitmap(p_bitmap, 8);
//     // print_bitmap(v_bitmap, 8);
//     t_free((unsigned int)ptr, sizeof(int));
//     t_free((unsigned int)ptr1, PAGE_SIZE*2);
//     t_free((unsigned int)ptr2, PAGE_SIZE);
//     // print_bitmap(p_bitmap, 8);
//     // print_bitmap(v_bitmap, 8);
// }
void fill_matrix_rand(unsigned int vp, int rows, int cols){
    for(int i = 0; i < rows; i++){
        for(int j = 0; j < cols; j++){
            int rand_val = rand() % 100 + 0;
            put_value(
                index_into(vp, i, j, cols), 
                &rand_val, 
                ELEM_SIZE);
        }
    }
}
void print_matrix(unsigned int vp, int rows, int cols){
    printf("Printing %dx%d Matrix\n", rows, cols);
    printf("---------------------\n");
    for(int i = 0; i < rows; i++){
        for(int j = 0; j < cols; j++){
            int ij;
            get_value(
                index_into(vp, i, j, cols), 
                &ij, 
                ELEM_SIZE);
            printf("%d\t", ij);
        }
        printf("\n");
    }
    printf("---------------------\n");
}
void test_matmult(int l, int m, int n){
    //A = l x m
    //B = m x n
    //C = l x n 
    
    // i,j = ArrName + (i * ELEM_SIZE * NUM_COLS) + j * ELEM_SIZE
    // NUM_COLS == SIZE_OF_ROWS == WIDTH OF 2DARRAY
    int* a = t_malloc(l * m * ELEM_SIZE);
    int* b = t_malloc(m * n * ELEM_SIZE);
    int* c = t_malloc(l * n * ELEM_SIZE);
    // debug_va((unsigned int) a);
    // debug_va((unsigned int) b);
    // debug_va((unsigned int) c);
    fill_matrix_rand((unsigned int)a, l, m);
    fill_matrix_rand((unsigned int)b, m, n);
    print_matrix((unsigned int)a, l, m);
    print_matrix((unsigned int)b, m, n);

    mat_mult((unsigned int) a, (unsigned int) b, (unsigned int) c, l, m, n);
    print_matrix((unsigned int)c, l, n);
}


int main(int argc, char const *argv[])
{
    set_physical_mem();
    test_matmult(2, 2, 2);
    print_TLB_missrate();
    return 0;
}