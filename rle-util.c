#include "rle-util.h"

void show_usage(void);

int main(int argc, char *argv[])
{
    int err = 0, fd;
	int open_flag;
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
		return 0;
    }


    rFile = fopen(argv[2], "rb");
    wFile = fopen(argv[3], "wb");

    fseek(rFile, 0, SEEK_END);	
	rSize = ftell(rFile);
	fseek(rFile, 0, SEEK_SET);

	uint8_t *wBuf; 
	uint8_t *rBuf = malloc(rSize); 
	fread(rBuf, rSize, 1, rFile);

    switch(option){

    case OPT_COMPRESS:

       wBuf = compress(rBuf, rSize, &wSize);
	   if (!wBuf){
		   err = 1;
		   goto label_err;
	   }
	   break;

    case OPT_DECOMPRESS: 

        wBuf = decompress(rBuf, rSize, &wSize);
		if (!wBuf){
		   err = 1;
		   goto label_err;
		}
		break;

	default:
		goto label_err; break;
    }

	/* write buffer to output file */
	fwrite(wBuf, 1, wSize, wFile);
	free(wBuf);

label_err:
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
