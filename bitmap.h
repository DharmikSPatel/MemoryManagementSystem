struct bitmap {
    unsigned int num_of_pages;
    unsigned int _bitmap_size; //will be num_of_pages/8
    char* bm;
};
struct bitmap* init_bm(unsigned int num_of_pages);
void free_bm(struct bitmap* bitmap);
unsigned int next_free_page(struct bitmap* bitmap);
unsigned int next_n_free_pages(struct bitmap* bitmap, unsigned int n);
void use_page(struct bitmap* bitmap, unsigned int page_num);
void free_page(struct bitmap* bitmap, unsigned int page_num);
unsigned int get_page(struct bitmap* bitmap, unsigned int page_num);
void print_bitmap(struct bitmap* bitmap, unsigned int n);

