#pragma once

#include <stdint.h>
#include <stdio.h>

struct bmp_file_header {
  uint16_t bf_type;
  uint32_t bf_size;
  uint16_t bf_reserved1;
  uint16_t bf_reserved2;
  uint32_t bf_off_bits;
} __attribute__((packed));

struct bmp_info_header {
  uint32_t bi_size;
  uint32_t bi_width;
  uint32_t bi_height;
  uint16_t bi_planes;
  uint16_t bi_bit_count;
  uint32_t bi_compression;
  uint32_t bi_size_image;
  uint32_t bi_x_pels_per_meter;
  uint32_t bi_y_pels_per_meter;
  uint32_t bi_clr_used;
  uint32_t bi_clr_important;
} __attribute__((packed));

struct pixel {
  unsigned char blue;
  unsigned char green;
  unsigned char red;
};

struct image_info {
  int width;
  int height;
  int padding;
  int image_size;
  int file_size;
};

struct bmp_image {
  struct bmp_file_header *file_header;
  struct bmp_info_header *info_header;
  struct pixel **array;
  struct image_info *info;
};

int load_bmp(char *filename, struct bmp_image *image);
int save_bmp(char *filename, struct bmp_image *image);

int crop(struct bmp_image *image, int x, int y, int width, int height);
int rotate(struct bmp_image *image);

void free_image(struct bmp_image *image);

