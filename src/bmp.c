#include "bmp.h"
#include <assert.h>
#include <stdlib.h>
#include <stdio.h>

static void update_info(struct image_info *info, int width, int height) {
  if (info == NULL) {
    return ;
  }

  info->width = width;
  info->height = height;
  info->padding = width % 4;
  info->image_size = sizeof(struct pixel) * width * height + height * info->padding; 
  info->file_size = sizeof(struct bmp_file_header) + sizeof(struct bmp_info_header) + info->image_size;
}

int load_bmp(char *filename, struct bmp_image *image) {
  size_t file_header_size = sizeof(struct bmp_file_header);
  size_t info_header_size = sizeof(struct bmp_info_header);
  size_t pixel_size = sizeof(struct pixel);

  image->file_header = malloc(file_header_size);
  image->info_header = malloc(info_header_size);
  image->info = malloc(sizeof(struct image_info));

  struct bmp_file_header *file_header = image->file_header;
  struct bmp_info_header *info_header = image->info_header;
  struct image_info *info = image->info;

  if (file_header == NULL || info_header == NULL || info == NULL) {
    free(file_header);
    free(info_header);
    free(info);
    free(image);
    return 1;
  }

  FILE *file = fopen(filename, "rb");
  if (file == NULL) {
    free(file_header);
    free(info_header);
    free(info);
    free(image);
    return 1;
  }

  fread(file_header, file_header_size, 1, file);
  fread(info_header, info_header_size, 1, file);

  int width = info_header->bi_width;
  int height = info_header->bi_height;

  update_info(info, width, height);

  int padding = info->padding;

  struct pixel *tmp_arr = malloc(pixel_size * width * height);
  struct pixel **arr = malloc(sizeof(struct pixel*) * height);
  char *useless = malloc(padding);
  if (tmp_arr == NULL || arr == NULL || useless == NULL) {
    free(tmp_arr);
    free(arr);
    free(useless);
    free(file_header);
    free(info_header);
    free(info);
    free(image);
    return 1;
  }

  struct pixel *line_p = tmp_arr;
  for (int line = 0; line < height; ++line) {
    fread(line_p, pixel_size, width, file);
    fread(useless, padding, 1, file);
    arr[line] = line_p;
    line_p += width;
  }

  free(useless);
  fclose(file);

  image->array = arr;

  return 0;
}

int save_bmp(char *filename, struct bmp_image *image) {
  struct bmp_file_header *file_header = image->file_header;
  struct bmp_info_header *info_header = image->info_header;
  struct pixel **src = image->array;
  struct image_info *info = image->info;

  if (src == NULL || file_header == NULL || info_header == NULL || info == NULL) {
    return 1;
  }

  FILE *file = fopen(filename, "wb");
  if (file == NULL) {
    return 1;
  }

  size_t pixel_size = sizeof(struct pixel);

  int width = info->width;
  int height = info->height;
  int padding = info->padding;
  int image_size = info->image_size;
  int file_size = info->file_size;

  info_header->bi_width = width;
  info_header->bi_height = height;
  info_header->bi_size_image = image_size;
  file_header->bf_size = file_size;

  fwrite(file_header, sizeof(struct bmp_file_header), 1, file);
  fwrite(info_header, sizeof(struct bmp_info_header), 1, file);

  char *useless = malloc(padding);
  if (useless == NULL) {
    fclose(file);
    return 1;
  }

  for (int i = 0; i < padding; ++i) {
    useless[i] = 0;
  }

  for (int line = 0; line < height; ++line) {
    fwrite(src[line], pixel_size, width, file);
    fwrite(useless, padding, 1, file);
  }

  free(useless);
  fclose(file);

  return 0;
}

int crop(struct bmp_image *image, int x, int y, int width, int height) {
  struct pixel **src = image->array;
  struct image_info *info = image->info;

  if (src == NULL || info == NULL) {
    return 1;
  }

  int orig_width = info->width;
  int orig_height = info->height;

  if (x < 0 || width <= 0 || x + width > orig_width ||
      y < 0 || height <= 0 || y + height > orig_height) {
    return 1;
  }

  int real_y = orig_height - y - height;
  int real_x = x;

  struct pixel *tmp_arr = malloc(sizeof(struct pixel) * width * height);
  struct pixel **arr = malloc(sizeof(struct pixel*) * height);
  if (tmp_arr == NULL || arr == NULL) {
    free(tmp_arr);
    free(arr);
    return 1;
  }

  for (int i = 0; i < height; ++i) {
    for (int j = 0; j < width; ++j) {
      tmp_arr[i * width + j] = src[real_y + i][real_x + j];
    }
  }

  struct pixel *line_p = tmp_arr;
  for (int line = 0; line < height; ++line) {
    arr[line] = line_p;
    line_p += width;
  }

  free(src[0]);
  free(src);

  update_info(info, width, height);
  image->array = arr;

  return 0;
}

int rotate(struct bmp_image *image) {
  struct pixel **src = image->array;
  struct image_info *info = image->info;

  if (src == NULL || info == NULL) {
    return 1;
  }

  int height = info->width;
  int width = info->height;

  struct pixel *tmp_arr = malloc(sizeof(struct pixel) * width * height);
  struct pixel **arr = malloc(sizeof(struct pixel*) * height);
  if (tmp_arr == NULL || arr == NULL) {
    free(tmp_arr);
    free(arr);
    return 1;
  }

  for (int i = 0; i < height; ++i) {
    for (int j = 0; j < width; ++j) {
      tmp_arr[i * width + j] = src[j][height - i - 1];
    }
  }

  struct pixel *line_p = tmp_arr;
  for (int line = 0; line < height; ++line) {
    arr[line] = line_p;
    line_p += width;
  }

  free(src[0]);
  free(src);

  update_info(info, width, height);
  image->array = arr;

  return 0;
}

void free_image(struct bmp_image *image) {
  free(image->file_header);
  free(image->info_header);
  if (image->array != NULL) {
    free(image->array[0]);
    free(image->array);
  }
  free(image->info);
  free(image);
}

