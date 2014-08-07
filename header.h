#include <stdio.h>
#include <stdint.h>
#include <stdlib.h>

#define ESCAPE_CHAR 0xfe


uint8_t *compress(uint8_t *rBuf, size_t rSize, size_t *wSize);
uint8_t *decompress(uint8_t *rBuf, size_t rSize, size_t *wSize);
int is_escape_char(uint8_t byte);
