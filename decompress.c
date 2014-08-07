#include "header.h"
#define REALLOC_INCREMENT        512

typedef enum 
{ 
	STATE_NORMAL, 
	STATE_ESCAPE, 
	STATE_WRITE_SEQUENCE, 
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

static state_t state_normal( compress_data_t *data );
static state_t state_escape( compress_data_t *data );
static state_t state_write_sequence( compress_data_t *data );

static state_func_t* const state_table[ NUM_STATES ] = {
    state_normal, 
    state_escape, 
    state_write_sequence, 
};



uint8_t *
decompress(uint8_t *rBuf, size_t rSize, size_t *wSize) 
{
	size_t i = 0;
	size_t buffer_size = (2*rSize)*sizeof(uint8_t);	
	uint8_t *buffer = malloc(buffer_size); /* TODO: Free */

    state_t cur_state = STATE_NORMAL;
    compress_data_t data;

	data.buffer = buffer;
	data.size = 0;

    while (i < rSize) {

		data.cur_byte = rBuf[i++];

        cur_state = run_state( cur_state, &data );

		data.prv_byte = data.cur_byte;

        /* increases the write buffer size when nearly filled */
		if ((buffer_size-data.size) < REALLOC_INCREMENT*sizeof(uint8_t)){
			buffer_size += REALLOC_INCREMENT*sizeof(uint8_t);
			buffer = realloc(buffer, buffer_size);
			if (!buffer)
				return 0;
			data.buffer = buffer;
		}

    }

	*wSize = data.size;

	buffer = realloc(buffer, data.size);
	data.buffer = buffer;
	return buffer;
}

static state_t 
run_state( state_t cur_state, compress_data_t *data ) {
    return state_table[ cur_state ]( data );
};


static state_t 
state_normal( compress_data_t *data )
{


	/* an escape char was detected, move to escape state to handle it */
	if (is_escape_char(data->cur_byte)){ 
		return STATE_ESCAPE;
	}
	/* no sequence was hit, write byte individually */
	else{
		data->buffer[data->size++] = data->cur_byte;
		return STATE_NORMAL;
	} 
}

static state_t 
state_escape( compress_data_t *data )
{
	/* detect double escapes */
	if (is_escape_char(data->cur_byte)){
		data->buffer[data->size++] = data->cur_byte;
		return STATE_NORMAL;

	} else{
		/* a sequence has been detected */
		return STATE_WRITE_SEQUENCE;
	}
}

static state_t 
state_write_sequence(compress_data_t *data)
{

 	size_t seq_len = data->prv_byte+2;
	size_t byte = data->cur_byte;

	/* write byte to buffer seq_len times */
	while(seq_len > 0){
		data->buffer[data->size++] = byte;
		seq_len--;
	}
	
	return STATE_NORMAL;
}

