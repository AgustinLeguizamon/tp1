#include <stdio.h>
#include <string.h>
#include <stdlib.h>
#include <byteswap.h>
#include <arpa/inet.h>
#include "translator.h"

#define HEADER_SIGNATURE_LEN 16
#define ARRAY_MIN 8 // 4 bytes + 1 entero 
#define BYTE_ALIGN 8 // para redondeo
#define OFFSET 4 //los primeros 4 parametros
#define SIGNATURE_MIN_LEN 5
#define COMMA ", "
#define PARENTHESIS '('
#define RIGHT_PARENTHESIS ')'

#define N_WORDS 36 //4 + 32 parametros cantidad de palabras para buffer

int translator_create(translator_t *self){
	self->header_len = 0;
	self->body_len = 0;

	return 0;
}

int _translator_method_separator(char words[][WORD_BUF]){	
	const char delim = PARENTHESIS;
	char* arg_ptr = strchr(words[3], delim);
	char method[WORD_BUF]="";

	int word_len = strlen(words[3]);
	int arg_len= strlen(arg_ptr);
	int dif = word_len - arg_len;
	
	memcpy(method, words[3], dif);
	memset(words[3], 0, WORD_BUF);
	memcpy(words[3], method, dif);

	return 0;
}

int _translator_arg_separator(char words[][WORD_BUF], char* arg_ptr){
	int j = OFFSET, n_arg = 0;
	char word_buffer[WORD_BUF]="";
	char c = 0;
	int i = 0, k=0;
	
	i++; //salteo parentesis

	c = arg_ptr[i]; 
	while(c != ')'){
		if (c != ','){
			word_buffer[k] = c;
			k++;
		}
		if (c == ','){
			strncpy(words[j], word_buffer, WORD_BUF);
			memset(word_buffer, 0 ,WORD_BUF);
			j++;
			n_arg++;
			k = 0;
		}
		i++;
		c = arg_ptr[i];
	}

	if (i > 1){
		n_arg++;
		strncpy(words[j], word_buffer, WORD_BUF);
	}
	
	return n_arg;
}

int _translator_separator(char* input_line, char words[][WORD_BUF]){
	char destiny[WORD_BUF], path[WORD_BUF];
	char interface[WORD_BUF], method_and_args[WORD_BUF]; 	
	const char delim = PARENTHESIS;
	char* arg_ptr = strchr(input_line, delim);
	int n_arg;

	sscanf(input_line, "%s %s %s %s",
		destiny, path, interface, method_and_args);
	
	strncpy(words[0], destiny, WORD_BUF);
	strncpy(words[1], path, WORD_BUF);
	strncpy(words[2], interface, WORD_BUF);
	strncpy(words[3], method_and_args, WORD_BUF);

	_translator_method_separator(words);
	n_arg = _translator_arg_separator(words, arg_ptr);

	return n_arg;
}

int translator_make_message(translator_t *self, file_reader_t *file_reader, 
		int id){
	char words[N_WORDS][WORD_BUF];
	int n_arg=0;

	char* input_line = file_reader->input_line;

	/*inicializao matriz*/
	for (int i = 0; i < N_WORDS; ++i){
		memset(words[i],0,sizeof(words[i]));
	}

	n_arg = _translator_separator(input_line, words);
	_translator_make_header(self, n_arg, id, words);
	_translator_make_body(self, words, n_arg);

	return 0;
}

int _translator_make_body(translator_t *self, char words[][WORD_BUF], 
		int n_arg){
	int body_len = _translator_calculate_body_len(words, n_arg);
	char* body = malloc(body_len);
    char* cursor = body;

    memset(body, 0, body_len);

	_translator_append_body(&cursor, words, n_arg);

	self->body = body;

	return 0;
}

int _translator_append_body(char** cursor, char words[][WORD_BUF], int n_arg){
	char* a_parameter;

	for (int i = OFFSET; i < OFFSET + n_arg; ++i){
		a_parameter = words[i];
		*((uint32_t*)(*cursor)) = 
			_translator_value_to_little_endian(strlen(a_parameter));
		(*cursor)+= sizeof(uint32_t);
		for (int j = 0; j < strlen(a_parameter); ++j){
			**cursor = a_parameter[j];
			(*cursor)++;
		}
		(*cursor)++;
	}
	return 0;
}


int _translator_calculate_signature_len(int n_arg){
	int signature_len=0;

	if (n_arg > 0){
		signature_len = SIGNATURE_MIN_LEN + n_arg + 1;
		signature_len = _translator_round_up(signature_len);
	} else {
		signature_len = 0;
	}

	return signature_len;
}


int _translator_make_header(translator_t *self, int n_arg, 
		int id, char words[][WORD_BUF]){
	int dest_len, path_len, intf_len, method_len, body_len, signature_len = 0;
	int total_header_len;
	char* cursor;

	dest_len = _translator_calculate_len(words[0]);
	path_len = _translator_calculate_len(words[1]);
	intf_len = _translator_calculate_len(words[2]);
	method_len = _translator_calculate_len(words[3]);
	body_len = _translator_calculate_body_len(words, n_arg);

	/*para los ultimos 8 bytes del header*/
	signature_len = _translator_calculate_signature_len(n_arg);
	
	total_header_len = dest_len + path_len + intf_len + method_len + signature_len;

	char* header = malloc(total_header_len + HEADER_SIGNATURE_LEN);
    memset(header, 0, total_header_len + HEADER_SIGNATURE_LEN);

    cursor = header;

	_translator_append_header_signature(&cursor, body_len, id, total_header_len);
	_translator_append_path(&cursor, words[1]);
	_translator_append_destiny(&cursor, words[0]);
	_translator_append_interface(&cursor, words[2]);
	_translator_append_method(&cursor, words[3]);

	if(n_arg > 0){
		_translator_append_signature(&cursor, n_arg);
	}

	self->header= header;
	self->header_len = (total_header_len+HEADER_SIGNATURE_LEN);
	self->body_len = body_len;

	return 0;
}


int	_translator_append_header_signature(char** cursor, int body_len, 
		int id, int total_header_len ){
	**cursor = 'l';
	(*cursor)++;
	**cursor = 1;
	(*cursor)++;
	**cursor = 0;
	(*cursor)++;
	**cursor = 1;

	(*cursor)++;
	*((uint32_t*)(*cursor)) = _translator_value_to_little_endian(body_len);
	(*cursor) += 4;
	*((uint32_t*)(*cursor)) = _translator_value_to_little_endian(id);
	(*cursor) += 4;
	//-1 es pq long del array no toma en cuenta el ultimo byte de padding 
	*((uint32_t*)(*cursor)) = 
		_translator_value_to_little_endian(total_header_len-1);
	(*cursor) += 4;

	return 0;
}


int _translator_append_path(char** cursor, char* word){
	**cursor = 1;
	(*cursor)++;
	**cursor = 1;
	(*cursor)++;
	**cursor = 'o';
	(*cursor)++;
	**cursor = 0;
	(*cursor)++;

	*((uint32_t*)(*cursor)) = _translator_value_to_little_endian(strlen(word));
	(*cursor) += 4;

	int path_with_padding = _translator_round_up(strlen(word)+1);
	for (int i = 0; i < path_with_padding; ++i){
		**cursor = word[i];
		(*cursor)++;
	}
	
	return 0;
}

int _translator_append_destiny(char** cursor, char* word){
	**cursor = 6;
	(*cursor)++;
	**cursor = 1;
	(*cursor)++;
	**cursor = 's';
	(*cursor)++;
	**cursor = 0;
	(*cursor)++;

	*((uint32_t*)(*cursor)) = _translator_value_to_little_endian(strlen(word));
	(*cursor) += 4;
	
	int destiny_with_padding = _translator_round_up(strlen(word)+1);
	for (int i = 0; i < destiny_with_padding; ++i){
		**cursor = word[i];
		(*cursor)++;
	}
	
	return 0;
}

int _translator_append_interface(char** cursor, char* interface){
	**cursor = 2;
	(*cursor)++;
	**cursor = 1;
	(*cursor)++;
	**cursor = 's';
	(*cursor)++;
	**cursor = 0;
	(*cursor)++;

	*((uint32_t*)(*cursor)) = 
		_translator_value_to_little_endian(strlen(interface));
	(*cursor) += 4;
	
	// +1 /0 al final
	int interface_with_padding = 
		_translator_round_up(strlen(interface)+1); 
	for (int i = 0; i < interface_with_padding; ++i){
		**cursor = interface[i];
		(*cursor)++;
	}

	return 0;	
}

int _translator_append_method(char** cursor, char* method){
	**cursor = 3;
	(*cursor)++;
	**cursor = 1;
	(*cursor)++;
	**cursor = 's';
	(*cursor)++;
	**cursor = 0;
	(*cursor)++;

	*((uint32_t*)(*cursor)) = _translator_value_to_little_endian(strlen(method));
	(*cursor) += 4;
	
	// +1 /0 al final
	int method_with_padding = 
		_translator_round_up(strlen(method)+1); 
	for (int i = 0; i < method_with_padding; ++i){
		**cursor = method[i];
		(*cursor)++;
	}

	return 0;
}

int _translator_append_signature(char** cursor, int n_arg){
	int i=0, signature_len_with_padding, padding;

	**cursor = 8;
	(*cursor)++;
	**cursor = 1;
	(*cursor)++;
	**cursor = 'g';
	(*cursor)++;
	**cursor = 0;
	(*cursor)++;
	**cursor = n_arg;
	(*cursor)++;

	for (i = 0; i < n_arg; ++i){
		**cursor = 's';
		(*cursor)++;
	}

	signature_len_with_padding = _translator_round_up(SIGNATURE_MIN_LEN+n_arg+1);
	padding = signature_len_with_padding - (SIGNATURE_MIN_LEN + n_arg);

	for (i = 0; i < padding; ++i){
		(*cursor)++; //salto los bytes dejando 00
	}

	return 0;
}


uint32_t _translator_value_to_little_endian(uint32_t value){
	uint32_t big_end, little_end;
	big_end = htonl(value); //pasa a big endian
	little_end = bswap_32(big_end);

	return little_end;
}

int _translator_calculate_len(char* word){
	int len = strlen(word);
	len += ARRAY_MIN;
	len += 1; // trailing \0
	len = _translator_round_up(len);

	return len;
}

int _translator_calculate_body_len(char words[][WORD_BUF], int n_arg){
	int body_len = 0;

	for (int i = OFFSET; i < n_arg+OFFSET; i++){
		body_len += sizeof(int) + strlen(words[i]) + 1; //trailing '/0'
	}

	return body_len;
}

int _translator_round_up(int value){
	int res = value % BYTE_ALIGN;

	if(res != 0){
		value = value + BYTE_ALIGN - res;
	}

	return value;
}

int translator_free(translator_t *self){
	free(self->header);
	free(self->body);
	self->header=0;
	self->body=0;

	return 0;
}

int translator_destroy(translator_t *self){
	return 0;
}
