#include <assert.h>
#include <stdint.h>
#include <stdio.h>

#define REVERSE_BYTES_INT(x)                                                   \
  (((x) >> 24) | (uint32_t)((x) >> 16 & 0xFF) << 8 |                           \
   (uint32_t)((x) >> 8 & 0xFF) << 24 | (uint32_t)((x)&0xFF) << 24)

#define REVERSE_BYTES_SHORT(x) (((x) >> 8) | (uint16_t)((x)&0xFF) << 8)

uint8_t device_memory_little_endian[] = {
	0x01, 0x00, 0x00, 0x00, /* First word contains u32 = 1 */
	0x02, 0x00, 0x00, 0x00, /* Second word contains u32 = 2 */
	0x03, 0x00, 0x04, 0x00, /* Third word contains u16 = 3 and u16 = 4 */
	0x05, 0x06, 0x07, 0x08  /* Fourth word contains u8 = 5, 6, 7, 8 */
};

uint8_t device_memory_big_endian[] = {
	0x00, 0x00, 0x00, 0x01, /* First word contains u32 = 1 */
	0x00, 0x00, 0x00, 0x02, /* Second word contains u32 = 2 */
	0x00, 0x03, 0x00, 0x04, /* Third word contains u16 = 3 and u16 = 4 */
	0x05, 0x06, 0x07, 0x08 /* Fourth word contains u8 = 5, 6, 7, 8 */
};

uint8_t device_memory_little_endian_mixed[] = {
	0x01, /* u8 = 1 */
	0x02, 0x00, /* u16 = 2 */
	0x00
};

uint8_t device_memory_big_endian_mixed[] = {
	0x01, /* u8 = 1 */ 
	0x00, 0x02, /* u16 = 2 */
	0x00
};

int main(void) {
  uint32_t u32 = 0;
  uint16_t u16 = 0;
  uint8_t u8 = 0;

  /* Read uint32_t little endian */

  uint32_t word = *(uint32_t *)device_memory_little_endian;
  assert(word == 1);
  word = *(uint32_t *)(device_memory_little_endian + 4);
  assert(word == 2);

  /* Read uint16_t little endian */

  word = *(uint32_t *)(device_memory_little_endian + 8);
  u16 = word & 0xFFFF;
  assert(u16 == 3);
  u16 = word >> 16 & 0xFFFF;
  assert(u16 == 4);

  /* Read uint8_t little endian */

  word = *(uint32_t *)(device_memory_little_endian + 12);
  u8 = word & 0xFF;
	assert(u8 == 5);
  u8 = word >> 8 & 0xFF;
	assert(u8 == 6);
  u8 = word >> 16 & 0xFF;
	assert(u8 == 7);
  u8 = word >> 24 & 0xFF;
	assert(u8 == 8);
  
	/* Read uin32_t big endian */

  word = *(uint32_t *)(device_memory_big_endian);
  u32 = REVERSE_BYTES_INT(word);
  assert(u32 == 1);
  word = *(uint32_t *)(device_memory_big_endian + 4);
  u32 = REVERSE_BYTES_INT(word);
  assert(u32 == 2);

  /* Read uin16_t big endian */

  word = *(uint32_t *)(device_memory_big_endian + 8);
  u16 = REVERSE_BYTES_SHORT(word & 0xFFFF);
  assert(u16 == 3);
  u16 = REVERSE_BYTES_SHORT(word >> 16 & 0xFFFF);
  assert(u16 == 4);

  /* Read uin8_t big endian */

  word = *(uint32_t *)(device_memory_big_endian + 12);
  u8 = word & 0xFF;
  assert(u8 == 5);
  u8 = word >> 8 & 0xFF;
	assert(u8 == 6);
  u8 = word >> 16 & 0xFF;
	assert(u8 == 7);
  u8 = word >> 24 & 0xFF;
	assert(u8 == 8);

	/* Read mixed size integers from memory little endian */

  word = *(uint32_t *)(device_memory_little_endian_mixed);
  u8 = word & 0xFF;
  assert(u8 == 1);
  u16 = word >> 8 & 0xFFFF;
  assert(u16 == 2);

  /* Read mixed size integers from memory big endin */

  word = *(uint32_t *)(device_memory_big_endian_mixed);
  u8 = word & 0xFF;
  assert(u8 == 1);
  u16 = REVERSE_BYTES_SHORT(word >> 8 & 0xFFFF);
  assert(u16 == 2);

  return 0;
}
