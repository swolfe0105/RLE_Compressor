
EXEC=rle-util
SOURCES=rle-util.c rle-compress.c rle-decompress.c

all:
	gcc ${SOURCES} -Wall -o ${EXEC} 


