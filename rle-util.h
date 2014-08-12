#include <stdio.h>
#include <stdint.h>
#include <stdlib.h>

#define ESCAPE_CHAR 0xfe

typedef struct {
	uint8_t cur_byte;
	uint8_t prv_byte;	
	size_t size;
	uint8_t *buffer;
} rle_data_t;

uint8_t *compress(uint8_t *rBuf, size_t rSize, size_t *wSize);
uint8_t *decompress(uint8_t *rBuf, size_t rSize, size_t *wSize);
int is_escape_char(uint8_t byte);
