# Embedded device memory

For embedded device there are usually exist some kind of configuration
parameters needed for firmware to work.
Usually they are stored ether in device FLASH on separate page separated
from firmware code ether in device EEPROM.

[Code](https://github.com/kalleva/KallevaEmbeddedNotes/tree/master/Note00C_IntegersInDeviceMemory) for this post

There is number of ways that parameters usually can be stored in non-volatile
memory and I tried to go through the most common cases I usually meet in the
wild. For modeling memory of the device I used several byte arrays.
When working with memory there is usually some limit on how firmware can read
and write to that memory. Sometimes there is only possibility to write and read
32bit words, sometimes firmware can only write in 64bit double words.
Here I decided to limit myself to the scope of reading 32bit words from memory.
The device doesn't allow access to memory that is not on 4 bytes boundaries.
And I also assumed that platform on which this program would be run is
little endian.

But first two words about how integers can be store big or little endian in
device memory. Lets look at ```uint32 val = 1```, val can be stored in memory
in little endian way like this

```C
Memory address | value on that address
0x00000000     | 0x01
0x00000001     | 0x00
0x00000002     | 0x00
0x00000003     | 0x00
```

Or in a big endian way:

```C
Memory address | value on that address
0x00000000     | 0x00
0x00000001     | 0x00
0x00000002     | 0x00
0x00000003     | 0x01
```

Usually CPU is little endian, and stuff that goes on the network are big endian,
sadly there is no one way to store integers and both are used. The most
important thing is not to mix both of them at the same time.

Following are the two arrays I will use for demonstration:

```C
uint8_t device_memory_little_endian[] = {
  0x01, 0x00, 0x00, 0x00, /* First word contains u32 = 1 */
  0x02, 0x00, 0x00, 0x00, /* Second word contains u32 = 2 */
  0x03, 0x00, 0x04, 0x00, /* Third word contains u16 = 3 and u16 = 4 */
  0x05, 0x00, 0x00, 0x00  /* Fourth word contains u8 = 5 */
}
```

```C
uint8_t device_memory_big_endian[] = {
  0x00, 0x00, 0x00, 0x01, /* First word contains u32 = 1 */
  0x00, 0x00, 0x00, 0x02, /* Second word contains u32 = 2 */
  0x00, 0x03, 0x00, 0x04, /* Third word contains u16 = 3 and u16 = 4 */
  0x05, 0x00, 0x00, 0x00 /* Fourth word contains u8 = 5 */
```

## Read uint32_t little endian
To read uint32_t value from the memory address, just get the value,
to which pointer to uint32_t points. This is the simplest case.

```C
uint32_t word = *(uint32_t *)device_memory_little_endian;
```

First four bytes in ```device_memory_little_endian``` array are
```0x01, 0x00, 0x00, 0x00``` which in little endian will mean just 1.
uin32_t integers are placed in memory one by one. It's straightforward to write
them to the memory and to read them back afterwards.

## Read uint16_t little endian

To read uin16_t values stored in little endian from memory will require a
little more work. Basically problem is that we want to get 2 byte integers, but
can only read memory by 4 byte chunks. There will be needed some additional post
processing to get value from that 4 byte word. This will be common theme for
other cases where we will want to get integer values smaller then the size of
chunk we read from memory.

```C
word = *(uint32_t *)(device_memory_little_endian + 8);
u16 = word & 0xFFFF;
assert(u16 == 3);
/* Next stored uint16_t */
u16 = word >> 16 & 0xFFFF;
assert(u16 == 4);
```

Here first read the whole 4 byte word. Then get first u16 integer take
first 2 bytes of that word. And to get the next u16 shift that word by two
bytes.

One catch is that there are two ways to pack two uint16_t into
uin32_t. Important thing is to be consistent between packing and unpacking
them. Real confusion can arise when writing arrays of multibyte integers
to memory, because then there is a need to not only watch how to pack
individual integers but also how to store order of elements in the array.
In my model of memory here I choose to store elements in a way that resembles
little endian.
Although I should note that it is wrong to call it little endian outright
because this name is reserved for packing individual integers and not for order
of elements in the stored array.
So if I have this array:

```C
uint8_t key[8] = {
  0x00, 0x01, 0x02, 0x03,
  0x04, 0x05, 0x06, 0x07
}
```

I will store it in non-volatile memory like this:

```
Memory address | value on that address
0x00000000     | 0x00
0x00000001     | 0x01
0x00000002     | 0x02
0x00000003     | 0x03
0x00000004     | 0x04
0x00000005     | 0x05
0x00000006     | 0x06
0x00000007     | 0x07
```

This seems like an obvious thing, but watch out for this in a wild, sometimes
people prefer to store arrays other way and it can bite you.

## Read uin8_t little endian

For reading uint8_t values get the 4 byte word and get it's 4 bytes from it
by shifting word to get the needed byte from it.
The thing to remember here is, as I mentioned earlier, to get stored values
in a proper order from memory (proper is relative, here it means the way you
stored them to memory before)

```C
word = *(uint32_t *)(device_memory_little_endian + 12);
u8 = word & 0xFF;
assert(u8 == 5);
u8 = word >> 8 & 0xFF;
assert(u8 == 6);
u8 = word >> 16 & 0xFF;
assert(u8 == 7);
u8 = word >> 24 & 0xFF;
assert(u8 == 8);
```

## Read uint32_t big endian

With the assumption that architecture of our processor on which the
program will be running is little endian, this means that multibyte integers
in words that will be read from memory would have their bytes reversed.

Here are two macros to reverse the order of bytes in two byte and four byte
integers.

```C
#define REVERSE_BYTES_INT(x)                                                   \
  (((x) >> 24) | (uint32_t)((x) >> 16 & 0xFF) << 8 |                           \
   (uint32_t)((x) >> 8 & 0xFF) << 24 | (uint32_t)((x)&0xFF) << 24)

#define REVERSE_BYTES_SHORT(x) (((x) >> 8) | (uint16_t)((x)&0xFF) << 8)
```

So uint32_t integer stored in a big endian will have its bytes reversed
and to get proper stored value from it, there is a need to use
REVERSE_BYTES_INT macro.

```C
word = *(uint32_t *)(device_memory_big_endian);
u32 = REVERSE_BYTES_INT(word);
assert(u32 == 1);
```

## Read uint16_t big endian

Separate uin16_t integers are stored the same way as previously (first two byte
integer is stored in lower two bytes of word read from memory) but order of
bytes inside these two byte integers are reversed. So after getting establishing
which two bytes belong to the current two byte integer there is a need to
reverse byte order in that integer with REVERSE_BYTES_SHORT macro.

```C
word = *(uint32_t *)(device_memory_big_endian + 8);
u16 = REVERSE_BYTES_SHORT(word & 0xFFFF);
assert(u16 == 3);
u16 = REVERSE_BYTES_SHORT(word >> 16 & 0xFFFF);
assert(u16 == 4);
```

## Read uin8_t big endian

There is no byte order in one byte integers so the only thing that should be
watched for is that the bytes are retrieved from the word read from memory in
the same way as they were written to it. As can bee seen the code is the same
as for reading one byte values from ```device_memory_little_endian```.

```C
word = *(uint32_t *)(device_memory_big_endian + 12);
u8 = word & 0xFF;
assert(u8 == 5);
u8 = word >> 8 & 0xFF;
assert(u8 == 6);
u8 = word >> 16 & 0xFF;
assert(u8 == 7);
u8 = word >> 24 & 0xFF;
assert(u8 == 8);
```

## Read mixed size integers from memory little and big endian

Basically there are no new concepts here. Just shift word read from memory by
proper amount to get the bytes needed. As always the order in which integers
were written to memory should be preserved.

To illustrate my point I created two arrays that mimic one word of memory of the
device, each contain one uint8_t and next to it uin16_t:

```C
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
```

This is how to get stored integer values where multibyte integers stored in the
little endian way:

```C
word = *(uint32_t *)(device_memory_little_endian_mixed);
u8 = word & 0xFF;
assert(u8 == 1);
u16 = word >> 8 & 0xFFFF;
assert(u16 == 2);
```

And this is the same thing but for a case where multibyte integers stored in the
big endian way:

```C
word = *(uint32_t *)(device_memory_big_endian_mixed);
u8 = word & 0xFF;
assert(u8 == 1);
u16 = REVERSE_BYTES_SHORT(word >> 8 & 0xFFFF);
assert(u16 == 2);
```
