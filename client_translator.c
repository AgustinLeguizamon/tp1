#include <stdio.h>
#include <string.h>
#include <stdlib.h>
#include <byteswap.h>
#include <arpa/inet.h>
#include "client_translator.h"

#define HEADER_SIGNATURE_LEN 16
#define ARRAY_MIN 8 // 4 bytes + 1 entero 
#define BYTE_ALIGN 8 // para redondeo
#define OFFSET 4 //los primeros 4 parametros
#define SIGNATURE_MIN_LEN 5
#define COMMA ", "
#define PARENTHESIS '('
#define RIGHT_PARENTHESIS ')'

/*crea un array en el heap para almacenar el cuerpo del mensaje*/
static int _translator_make_body(translator_t *self, 
	char* input_line, int n_arg);


/*concatena los parametros y los strings para conformar
el cuerpo del mensaje segun protocolo dbus
*/
static int _translator_append_body(char** cursor, char* input_line, int n_arg);

/*Arma el header concatenando cada uno de los parametros
del array
*/
static int _translator_make_header(translator_t *self, 
	int n_arg, int id, char* input_line);


/*Arma el parametro firma del header y lo agrega al header
*/
static int _translator_append_header_signature(char** cursor, 
	int body_len, int id, int total_header_len);

/* Arma el parametro ruta y lo agrega al header
*/
static int _translator_append_path(char** cursor, char* word);

/* Arma el parametro ruta y lo agrega al header
*/
static int _translator_append_destiny(char** cursor, char* word);


/* Arma el parametro interfaz y lo agrega al header
*/
static int _translator_append_interface(char** cursor, char* interface);


/* Arma el parametro metodo y lo agrega al header
*/
static int _translator_append_method(char** cursor, char* method);


/* Arma el parametro firma y lo agrega al header
*/
static int _translator_append_signature(char** cursor, int n_arg);

/*Convierte valores a little endian independientemente
de su endianess
*/
static uint32_t _translator_value_to_little_endian(uint32_t value);

/*Calcula el largo de un string redondeando a multipo de 8
*/
static int _translator_calculate_len(char* word);

/*Calcula el largo del cuerpo
*/
static int _translator_calculate_body_len(char* input_line);

/*Calcula el largo del parametro firma
*/
static int _translator_calculate_signature_len(int n_arg);

/* Devuelve @param value redondeado a multiplo de 8
*/
static int _translator_round_up(int value);

/*Agrega \0 como separador de cada palabra y argumento del @param input_line
*/
static int _translator_transform_line(char* input_line);


int _translator_transform_line(char* input_line){
	int i=0;
	char* end_of_word = strchr(input_line, ' ');
	while(i < (OFFSET-1)){
		*end_of_word = '\0';
		end_of_word = strchr(end_of_word+1, ' ');
		i++;
	}

	char *cursor = input_line;
	cursor += strlen(cursor)+1; 
	cursor += strlen(cursor)+1;
	cursor += strlen(cursor)+1;
	char* method_and_args = cursor;
	
	//method and arg separator

	end_of_word = strchr(method_and_args, '(');
	*end_of_word = '\0';
	char* method = method_and_args;

	char* save_ptr = end_of_word;
	end_of_word = strchr(end_of_word+1, ',');
	while(end_of_word != NULL){
		*end_of_word = '\0';
		save_ptr = end_of_word;
		end_of_word = strchr(end_of_word+1, ',');
	}
	end_of_word = strchr(save_ptr+1, ')');
	*end_of_word = '\0';
	
	cursor = method_and_args;	
	cursor += strlen(method)+1;

	while(*cursor != '\0'){
		char* arg = cursor;
		cursor += strlen(arg)+1;		
	}

	//cuenta todas las palabras
	cursor = input_line;
	int cant_arg=0;
	while(*cursor != '\0'){
		cursor += strlen(cursor)+1;
		cant_arg++;
	}

	//devuelvo la cantidad de argumentos (resto las 4 primeras palabras)
	return (cant_arg - OFFSET);
}


//ctor
int translator_create(translator_t *self){
	self->header_len = 0;
	self->body_len = 0;

	return 0;
}


int translator_make_message(translator_t *self, file_reader_t *file_reader, 
		int id){
	int n_arg=0;

	char* input_line = file_reader->input_line;

	n_arg = _translator_transform_line(input_line);
	_translator_make_header(self, n_arg, id, input_line);
	_translator_make_body(self, input_line, n_arg);

	return 0;
}

int _translator_make_body(translator_t *self, char* input_line, 
		int n_arg){
	int body_len = _translator_calculate_body_len(input_line);
	char* body = malloc(body_len);
    char* cursor = body;

    memset(body, 0, body_len);

	_translator_append_body(&cursor, input_line, n_arg);

	self->body = body;

	return 0;
}

int _translator_append_body(char** cursor, char* input_line, int n_arg){
	char* word_cursor = input_line;
	int i=0;
	while(i < OFFSET){
		word_cursor += strlen(word_cursor)+1;
		i++;
	}

	char* a_parameter;
	for (int i = OFFSET; i < OFFSET + n_arg; ++i){
		a_parameter = word_cursor;
		*((uint32_t*)(*cursor)) = 
			_translator_value_to_little_endian(strlen(a_parameter));
		(*cursor)+= sizeof(uint32_t);
		for (int j = 0; j < strlen(a_parameter); ++j){
			**cursor = a_parameter[j];
			(*cursor)++;
		}
		(*cursor)++;
		word_cursor += strlen(word_cursor)+1;
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
		int id, char* input_line){
	char *cursor = input_line;
	char* destiny = cursor;
	
	cursor += strlen(destiny)+1; 
	char* path = cursor;
	
	cursor += strlen(path)+1;
	char* interface = cursor;
	
	cursor += strlen(interface)+1;
	char* method = cursor;

	int dest_len = _translator_calculate_len(destiny);
	int path_len = _translator_calculate_len(path);
	int intf_len = _translator_calculate_len(interface);
	int method_len = _translator_calculate_len(method);
	int body_len = _translator_calculate_body_len(input_line);

	/*para los ultimos 8 bytes del header*/
	int signature_len = _translator_calculate_signature_len(n_arg);
	
	int total_header_len = dest_len + path_len + 
		intf_len + method_len + signature_len;

	char* header = malloc(total_header_len + HEADER_SIGNATURE_LEN);
    memset(header, 0, total_header_len + HEADER_SIGNATURE_LEN);

    char* header_cursor = header;

	_translator_append_header_signature(&header_cursor, 
			body_len, id, total_header_len);
	_translator_append_path(&header_cursor, path);
	_translator_append_destiny(&header_cursor, destiny);
	_translator_append_interface(&header_cursor, interface);
	_translator_append_method(&header_cursor, method);

	if(n_arg > 0){
		_translator_append_signature(&header_cursor, n_arg);
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
		if(i > strlen(word)){
			**cursor = 0;
		} else {
			**cursor = word[i];
		}
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
		if(i > strlen(word)){
			**cursor = 0;
		} else {
			**cursor = word[i];
		}
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
		if(i > strlen(interface)){
			**cursor = 0;
		} else {
			**cursor = interface[i];
		}
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
		if(i > strlen(method)){
			**cursor = 0;
		} else {
			**cursor = method[i];
		}
		(*cursor)++;
	}

	return 0;
}

int _translator_append_signature(char** cursor, int n_arg){
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

	for (int i = 0; i < n_arg; ++i){
		**cursor = 's';
		(*cursor)++;
	}

	int signature_len_with_padding = 
		_translator_round_up(SIGNATURE_MIN_LEN+n_arg+1);
	int padding = signature_len_with_padding - (SIGNATURE_MIN_LEN + n_arg);

	for (int i = 0; i < padding; ++i){
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

int _translator_calculate_body_len(char* input_line){
	char* cursor = input_line;
	int i =0;
	//salteo primeros 4
	while(i < OFFSET){
		cursor += strlen(cursor)+1;
		i++;
	}
	
	int body_len = 0;
	while(*cursor != '\0'){
		body_len += sizeof(uint32_t) + strlen(cursor) + 1; //trailing '/0'
		cursor += strlen(cursor)+1;
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
