#ifndef RINGBUFFER_H_
#define RINGBUFFER_H_

#include <stdint.h>

typedef struct Ringbuffer_
{
  uint32_t len;
  volatile uint8_t *buffer;
  volatile uint32_t head;
  volatile uint32_t tail;
} Ringbuffer;

#define ringbuffer_write(rb, x) \
  rb.buffer[rb.head] = x; \
  if ((rb.head + 1) >= rb.len) rb.head = 0; \
  else rb.head = rb.head + 1;

/* Read from a buffer. Returns '\0' if there is nothing to read. */
static inline char ringbuffer_read(Ringbuffer* buf) {
  if (buf->tail == buf->head) return '\0';
  uint8_t read = buf->buffer[buf->tail];
  buf->tail = (buf->tail < ( buf->len - 1 )) ? (buf->tail + 1) : 0;
  return read;
}

#endif /* RINGBUFFER_H_ */
