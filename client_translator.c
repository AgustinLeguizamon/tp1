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

int _translator_method_separator(char words[][WORD_BUF]){	
	const char delim = PARENTHESIS;
	char* arg_ptr = strchr(words[3], delim);
	char method[WORD_BUF]="";

	printf("PRIntING WORD 3: %s\n", words[3]);
	int word_len = strlen(words[3]);
	int arg_len= strlen(arg_ptr);
	int dif = word_len - arg_len;
	
	memcpy(method, words[3], dif);
	memset(words[3], 0, WORD_BUF);
	memcpy(words[3], method, dif);

	return 0;
}

int _translator_arg_separator(char words[][WORD_BUF], char* arg_ptr){
	int i = OFFSET, n_arg = 0;
	const char ct[3] = COMMA;
	char* token;
	char copy_arg_ptr[WORD_BUF]="";
	char word_buffer[WORD_BUF]="";

	strncpy(copy_arg_ptr, arg_ptr, WORD_BUF);

	//borra el primer y ultimo parenteses
	char* s = strchr(copy_arg_ptr, RIGHT_PARENTHESIS);
	memcpy(word_buffer, copy_arg_ptr+1, 
		strlen(copy_arg_ptr)-(strlen(s)+1)); //remuevo parentesis

	token = strtok(word_buffer, ct);
	while (token != NULL){	
		strncpy(words[i], token, WORD_BUF);
		printf("words_arg[%d]: %s\n",i, words[i]);
		token = strtok(NULL, ct);
		i++;
		n_arg++;
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
	
	printf("PRINTING METHODS_AND_ARGS:%s\n", method_and_args);

	strncpy(words[0], destiny, WORD_BUF);
	strncpy(words[1], path, WORD_BUF);
	strncpy(words[2], interface, WORD_BUF);
	strncpy(words[3], method_and_args, WORD_BUF);

	printf("PRINTING WORDS[3] afcpy:%s\n", words[3]);
	_translator_method_separator(words);
	n_arg = _translator_arg_separator(words, arg_ptr);

/*
	for (int i = 0; i < (OFFSET + n_arg); i++) {
		printf("Word[%d]: %s\n",i , words[i]);//TEST
	}
*/	printf("N_ARG: %d\n", n_arg );
	return n_arg;
}

message_t translator_dbus(char* input_line, int id){
	char words[N_WORDS][WORD_BUF];
	message_t message;
	int n_arg=0;
	/*inicializao matriz*/
	for (int i = 0; i < N_WORDS; ++i){
		memset(words[i],0,sizeof(words[i]));
	}

	n_arg = _translator_separator(input_line, words);
	char* header = _translator_msg_maker(words, n_arg, id, &message);
	char* body = _translator_make_body(words, n_arg);
	message.header = header;
	message.body = body;


	return message;
}

char* _translator_make_body(char words[][WORD_BUF], int n_arg){
	int body_len = _translator_calculate_body_len(words, n_arg);
	char* body = malloc(body_len);
    char* cursor = body;

    memset(body, 0, body_len);

	_translator_append_body(&cursor, words, n_arg);

	printf("long cuerpo: %u\n", body_len);
	printf("\n");
	printf("BODY: \n");
	for (int i = 0; i < body_len ; ++i){
		printf("%02x ", body[i]);
	}
	printf("\n");

	return body;
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



char* _translator_msg_maker(char words[][WORD_BUF], int n_arg, 
		int id, message_t *message){
	int dest_len, path_len, intf_len, method_len, body_len, signature_len = 0;
	int total_header_len;

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

    char* cursor = header;

	_translator_append_header_signature(&cursor, body_len, id, total_header_len);
	_translator_append_path(&cursor, path_len, words[1]);
	_translator_append_destiny(&cursor, words[0]);
	_translator_append_interface(&cursor, words[2]);
	_translator_append_method(&cursor, words[3]);
	if(n_arg > 0){
		_translator_append_signature(&cursor, n_arg);
	}

	printf("HEXA\n");
	for (int i = 0; i < total_header_len + HEADER_SIGNATURE_LEN; i++){
		printf("%02x ", header[i]);
	}
	printf("Total len: %d\n", (total_header_len + HEADER_SIGNATURE_LEN) );

	message->header_len = (total_header_len+HEADER_SIGNATURE_LEN);
	message->body_len = body_len;

	return header;
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


int _translator_append_path(char** cursor, int path_len, char* word){
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
	printf("destiny: %d\n", destiny_with_padding);
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
