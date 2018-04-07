#include "bmp.h"

FILE *open_bmp_file(char *bmp_file_name);
bmp_image *read_bmp_file(char *n_bmp_file);
void write_bmp_file(bmp_image *image, char *bmp_file);
void to_gray_scale(bmp_image *image);
void to_binary_image(bmp_image *image, int umbral, char *bmp_file);
int sort_out_image_binary(bmp_image * image, int c);
void write_result(char *bmp_file, int value);
void execute_task(int amount_images, int umbral, int cat, int flag);
int get_padding_size(int width);