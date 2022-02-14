#include "stego.h"
#include "bmp.h"
#include <stdio.h>
#include <stdlib.h>
#include <assert.h>
#include <string.h>

static int crop_rotate(char *input_file_name, char *output_file_name, int x, int y, int width, int height) {
  struct bmp_image *image = malloc(sizeof(struct bmp_image));
  if (image == NULL) {
    return 1;
  }

  int result = load_bmp(input_file_name, image);

  if (result != 0) {
    return result;
  }

  result = crop(image, x, y, width, height);

  if (result == 0) {
    result = rotate(image);
  }

  if (result == 0) {
    result = save_bmp(output_file_name, image);
  }

  free_image(image);

  return result;
}

int main(int argc, char **argv) {
  if (argc < 2) {
    return 1;
  }

  if (strcmp(argv[1], "crop-rotate") == 0 && argc >= 8) {
    return crop_rotate(argv[2], argv[3], atoi(argv[4]), atoi(argv[5]), atoi(argv[6]), atoi(argv[7]));
  }

  if (strcmp(argv[1], "extract") == 0 && argc >= 5) {
    return extract_message(argv[2], argv[3], argv[4]);
  }

  if (strcmp(argv[1], "insert") == 0 && argc >= 6) {
    return insert_message(argv[2], argv[3], argv[4], argv[5]);
  }

  return 1;  
}

