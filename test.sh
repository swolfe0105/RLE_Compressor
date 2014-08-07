#!/bin/bash

# This script compresses, then decompresses a test file and
# verifies that the decompressed file is the same as the original
# displays size reduction statistics

if [ ! $# -eq 1 ]; then
	echo  "Usage:"
	echo  "	  prog <file>"
	echo  "	  test file - file to be used for testing"
	exit
fi

if [ ! -e $1 ]; then
	echo "Test: file does not exist"
	exit
fi

COMP="/tmp/${1##*/}.compress"
DECOMP="/tmp/${1##*/}.decompress"


if ! ./rle compress $1 $COMP; then
	echo "Test: error during compression"
	rm $COMP
	exit
fi

if ! ./rle decompress $COMP $DECOMP; then
	echo "Test: error during decompression"
	rm $COMP $DECOMP
	exit
fi

if cmp -s $1 $DECOMP; then

	ORIG_SIZE=$(stat -c%s "$1")
	COMP_SIZE=$(stat -c%s "$COMP")
	RATIO=$(echo "scale=2; $COMP_SIZE/$ORIG_SIZE" | bc)

	echo "Compression Test Completeed Successfully"
	echo "   Original Size  : $ORIG_SIZE"
	echo "   Compressed Size: $COMP_SIZE"
	echo "   ratio          : $RATIO"

else
	echo "ERROR: Reconstructed file does not match original"
fi

rm $COMP $DECOMP
