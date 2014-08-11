#include "rle-util.h"
#define REALLOC_INCREMENT        256 

typedef enum 
{ 
	STATE_INITIAL, 
	STATE_NORMAL, 
	STATE_COUNT, 
	NUM_STATES 
} state_t;


typedef struct {
	size_t size;
	uint8_t *buffer;
	uint8_t cur_byte;
	uint8_t prv_byte;	
} compress_data_t;

typedef state_t state_func_t( compress_data_t *data );
static state_t run_state( state_t cur_state, compress_data_t *data ); 

static state_t state_initial( compress_data_t *data );
static state_t state_normal( compress_data_t *data );
static state_t state_count( compress_data_t *data );

static state_func_t* const state_table[ NUM_STATES ] = {
    state_initial, 
    state_normal, 
    state_count, 
};


int write_sequence(compress_data_t *data, uint8_t count);

/* compresses a sequence of bytes by using an escaped run length
 * encoding sequence when beneficial.
 *
 * output is returned as a buffer with a length of wSize
 */
uint8_t *
compress(uint8_t *rBuf, size_t rSize, size_t *wSize) 
{
	size_t i = 0;
	size_t buffer_size = rSize*sizeof(uint8_t);	
	uint8_t *buffer = malloc(buffer_size);
    state_t cur_state = STATE_INITIAL;
    compress_data_t data;

	data.buffer = buffer;
	data.size = 0;

    while (i < rSize+1) {

		/* force end of sequence on last byte */
		if (i < rSize){
			data.cur_byte = rBuf[i++];
		}else{
			data.cur_byte = ~data.cur_byte;
			i++;
		}

        cur_state = run_state( cur_state, &data );
		data.prv_byte = data.cur_byte;

		if (buffer_size-data.size < 100){
			buffer_size += REALLOC_INCREMENT*sizeof(uint8_t);
			buffer = realloc(buffer, buffer_size);
			data.buffer = buffer;
		}

    }

	*wSize = data.size;

	buffer = realloc(buffer, data.size);
	data.buffer = buffer;

	return buffer;
}

/* Executes the state function associated with cur_state */
static state_t 
run_state( state_t cur_state, compress_data_t *data ) {
    return state_table[ cur_state ]( data );
};

/* This is starting state of the state machine.
 *
 * The purpose of the function is to delay one byte read
 * as state_normal() needs to look at the previous byte
 * (which does not exist of only one byte is read)
 */
static state_t 
state_initial( compress_data_t *data )
{ 
	return STATE_NORMAL; 
} 

static state_t state_normal( compress_data_t *data )
{

	/* a sequence was detected, move to count state to get length */
	if (data->prv_byte == data->cur_byte){
		return STATE_COUNT;
	}
	/* no sequence was hit, write bytes individually */
	else{
		data->buffer[data->size++] = data->prv_byte;

		/* write escape character twice to escape it */
	    if (is_escape_char(data->prv_byte)) {
		    data->buffer[data->size++] = data->prv_byte;
		}
	    
		return STATE_NORMAL;
	} 
}

/* state is called when a two byte sequence has been detected
 *
 * If the sequence continues, a counter is incremented
 * If the sequence ends (or count reaches its maximum value),
 * the sequence is written to file and normal state is returned.
 */
static state_t 
state_count( compress_data_t *data )
{
	/* count is always two less than the true sequence length 
	 * (it is mapped upward to improve compression as
	 * sequence lengths cannot be smaller than 2) */
	static uint8_t count = 0;

	/* if at end of sequence is reached or count is filled then
	 * write sequence to buffer */
	if (data->prv_byte != data->cur_byte || count == 0xff){
		write_sequence(data, count);
		count = 0;
		return STATE_NORMAL;
	}
	else{
		count++;
		return STATE_COUNT;
	}
}

/* Writes sequence of bytes to buffer */
int 
write_sequence(compress_data_t *data, uint8_t count)
{

	size_t seq_len = (size_t)count + 2;
	size_t byte = data->prv_byte;

    int byte_is_escape = is_escape_char(data->prv_byte);	
    int count_is_escape = is_escape_char(count);	

	/* do not compress 2 or 3 byte non-escape sequences as they 
	 * will be made longer (or same length) from compression */
	if(!byte_is_escape && seq_len <= 3){

		while(seq_len > 0){
			data->buffer[data->size++] = byte;
			seq_len--;
		}
	} else {

		/* Write escape char to signal start of compressed sequence */
		data->buffer[data->size++] = ESCAPE_CHAR;

		/* if count is the same as the escape character, it will appear
		 * as a double escape to the decodeor. The fix is to decrease
		 * the count by one and append the last character afterwards */
		if(count_is_escape){
			data->buffer[data->size++] = count-1;
			data->buffer[data->size++] = byte;
			data->buffer[data->size++] = byte;

			/* double escape if byte is the escape char */
			if(byte_is_escape)
				data->buffer[data->size++] = byte;

		} else {
			/* standard escape format */
			data->buffer[data->size++] = count;
			data->buffer[data->size++] = byte;
		}
	}

	return STATE_NORMAL;
}
