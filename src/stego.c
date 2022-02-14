#include "stego.h"
#include "bmp.h"
#include <stdbool.h>
#include <stdio.h>
#include <stdlib.h>

static const int bits_in_char = 5;
static const int max_msg_len = 256;

static char bits_to_char(bool bits[bits_in_char]) {
  int number = 0;
  for (int i = 0; i < bits_in_char; ++i) {
    number += bits[i] * (1 << i);
  }

  switch (number) {
    case 28: return ',';
    case 27: return '.';
    case 26: return ' ';
    default: return 'A' + number;
  }
}

void char_to_bits(char c, bool bits[bits_in_char]) {
  int number = 0;
  switch (c) {
    case ',': number = 28; break;
    case '.': number = 27; break;
    case ' ': number = 26; break;
    default : number = c - 'A';
  }

  for (int i = 0; i < bits_in_char; ++i) {
    bits[i] = number & 1;
    number >>= 1;
  }
}

int extract_message(char *in_filename, char *key_filename, char *msg_filename) {
  struct bmp_image *image = malloc(sizeof(struct bmp_image));
  if (image == NULL) {
    return 1;
  }

  if (load_bmp(in_filename, image) != 0) {
    return 1;
  }

  FILE *key_file = fopen(key_filename, "r");
  if (key_file == NULL) {
    free_image(image);
    return 1;
  }

  FILE *msg_file = fopen(msg_filename, "w");
  if (msg_file == NULL) {
    free_image(image);
    fclose(key_file);
    return 1;
  }

  struct pixel **arr = image->array;
  bool eof = false;
  bool bits[bits_in_char];
  int x = 0;
  int y = 0;
  char color = 'R';
  while (!eof) {
    for (int i = 0; i < bits_in_char; ++i) {
      if (fscanf(key_file, "%d %d %c", &x, &y, &color) != 3) {
        eof = true;
        break;
      }

      switch (color) {
        case 'R': bits[i] = arr[x][y].red   & 1; break;
        case 'G': bits[i] = arr[x][y].green & 1; break;
        default : bits[i] = arr[x][y].blue  & 1;
      }
    }

    if (!eof) {
      putc(bits_to_char(bits), msg_file);
    } else {
      putc('\n', msg_file);
    }
  }

  free_image(image);
  fclose(key_file);
  fclose(msg_file);

  return 0;
}

int insert_message(char *in_filename, char *out_filename, char *key_filename, char *msg_filename) {
  struct bmp_image *image = malloc(sizeof(struct bmp_image));
  if (image == NULL) {
    return 1;
  }

  if (load_bmp(in_filename, image) != 0) {
    return 1;
  }

  FILE *key_file = fopen(key_filename, "r");
  if (key_file == NULL) {
    free_image(image);
    return 1;
  }

  FILE *msg_file = fopen(msg_filename, "r");
  if (msg_file == NULL) {
    free_image(image);
    fclose(key_file);
    return 1;
  }

  char msg[max_msg_len];
  fgets(msg, max_msg_len, msg_file);

  struct pixel **arr = image->array;
  int msg_index = 0;
  bool eof = false;
  bool bits[bits_in_char];
  int x = 0;
  int y = 0;
  char color = 'R';
  while (msg[msg_index] != 0 && !eof) {
    char_to_bits(msg[msg_index], bits);
    for (int i = 0; i < bits_in_char; ++i) {
      if (fscanf(key_file, "%d %d %c", &x, &y, &color) != 3) {
        eof = true;
        break;
      }

      switch (color) {
        case 'R': arr[x][y].red   = ((arr[x][y].red   >> 1) << 1) | bits[i]; break;
        case 'G': arr[x][y].green = ((arr[x][y].green >> 1) << 1) | bits[i]; break;
        default : arr[x][y].blue  = ((arr[x][y].blue  >> 1) << 1) | bits[i];
      }
    }

    ++msg_index;
  }

  int result = save_bmp(out_filename, image);

  free_image(image);
  fclose(key_file);
  fclose(msg_file);

  return result;
}

