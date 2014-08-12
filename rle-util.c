/* RLE Compression Utility
 * Stephen Wolfe
 *
 * This utility uses finite state machines to compress or decompress
 * files using run-length encoding when beneficial. The compressed runs
 * are a three byte sequence (1 escape char, 2 data bytes). The first
 * data byte is the length of the run minus 2. The second is the byte
 * that the run consisted of.
 *
 * e.g.: 
 * original:     0x?? 0x?? 0x65  0x65 0x65 0x65 0x65 0x?? 0x??
 * compressed:   0x?? 0x?? <ESC> 0x03 0x65 0x?? 0x??
 */


#include <string.h>
#include "rle-util.h"

void show_usage(void);

/* returns 0 on success, 1 on failure */
int main(int argc, char *argv[])
{
    int err = 0;
    FILE *rFile, *wFile;
    size_t rSize, wSize;

	enum {
        OPT_INFO,
		OPT_COMPRESS,
		OPT_DECOMPRESS,
	} option = OPT_INFO;

    /* parse command-line options */
	if (argc == 4 && !strcmp(argv[1], "compress")) {
		option = OPT_COMPRESS;
    } else if (argc == 4 && !strcmp(argv[1], "decompress")) {
		option = OPT_DECOMPRESS;
	} else {
   		show_usage(); 
		return 1;
    }


    rFile = fopen(argv[2], "rb");
    wFile = fopen(argv[3], "wb");

    fseek(rFile, 0, SEEK_END);	
	rSize = ftell(rFile);
	fseek(rFile, 0, SEEK_SET);

	uint8_t *wBuf = NULL; 
	uint8_t *rBuf = malloc(rSize); 
	fread(rBuf, rSize, 1, rFile);

    switch(option){

    case OPT_COMPRESS:
       wBuf = compress(rBuf, rSize, &wSize);
	   break;

    case OPT_DECOMPRESS: 
        wBuf = decompress(rBuf, rSize, &wSize);
		break;

	default: break;
    }

    if (wBuf)
		/* write buffer to output file */
		fwrite(wBuf, 1, wSize, wFile);
	else 
		err = 1;
	
	free(wBuf);

    fclose(rFile);
    fclose(wFile);

    return err;
}

void show_usage(void)
{
	printf(
	    "Run-Length Encoding Compression Utility Usage\n"
	    "	prog <command> <rFile> <wFile>\n"
	    "	command - 'compress' or 'decompress'\n"
	    "	rFile - File to be (de)compressed\n"
	    "	rFile - Output of (de)compression\n"
	     );
}

int 
is_escape_char(uint8_t byte)
{
	return (byte == ESCAPE_CHAR) ? 1 : 0;
}
