#include <stdbool.h>
#include <stdint.h>
#include <stdio.h>

/* First three bytes of my IMEIs are the same this is what allows me to look for
 * IMEI pattern in a data */
#define FIRST_BYTE 0xFF
#define SECOND_BYTE 0xFF
#define THIRD_BYTE 0xFF

/* This shoudl be bigger than possible number of IMEIs */
#define UNIQUE_IMEI_SIZE 256

static void parser(FILE *fin);
static void unique_imei_insert(uint64_t imei);

typedef struct {
  uint64_t *data;
  uint32_t length;
} ImeiArray;

static uint64_t imei_buffer[UNIQUE_IMEI_SIZE];

ImeiArray unique_imei = {.data = imei_buffer, .length = 0};

int main(void) {
  FILE *fin = fopen("long.pcap", "rb");
  parser(fin);
  fclose(fin);
  FILE *fout = fopen("imei.txt", "w");
  for (int i = 0; i < unique_imei.length; i++) {
    fprintf(fout, "%llu\n", unique_imei.data[i]);
  }

  fclose(fout);
  return 0;
}

static void parser(FILE *fin) {
  if (fin == NULL) {
    return;
  }

  uint8_t imei_buffer[4] = {0};
  /* Byte values can be in range 0x00 - 0xFF */
  /* To distinguish from EOF = -1 use int */
  int b;
  bool imei_flag = false;
  int cnt = 0;
  while ((b = fgetc(fin)) != EOF) {
    if (b == FIRST_BYTE) {
      cnt = 1;
    } else if (b == SECOND_BYTE && cnt == 1) {
      ++cnt;
    } else if (b == THIRD_BYTE && cnt == 2) {
      ++cnt;
    } else if (cnt == 3 || cnt == 4 || cnt == 5) {
      imei_buffer[cnt - 3] = b;
      ++cnt;
    } else if (cnt == 6) {
      imei_buffer[cnt - 3] = b;
      unique_imei_insert(
          (uint64_t)FIRST_BYTE << 48 | (uint64_t)SECOND_BYTE << 40 |
          (uint64_t)THIRD_BYTE << 32 | (uint64_t)imei_buffer[0] << 24 |
          (uint64_t)imei_buffer[1] << 16 | (uint64_t)imei_buffer[2] << 8 |
          (uint64_t)imei_buffer[3]);
			/* Start looking for pattern again because match was finished */
      cnt = 0;
    } else {
			/* Start looking for pattern again because it wasn't proper match */
      cnt = 0;
    }
  }
}

static void unique_imei_insert(uint64_t imei) {
  for (int i = 0; i < unique_imei.length && i < UNIQUE_IMEI_SIZE; i++) {
    if (unique_imei.data[i] == imei) {
      return;
    }
  }
  unique_imei.data[unique_imei.length++] = imei;
}
