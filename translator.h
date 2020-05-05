#ifndef TP1_TRANSLATOR_H
#define TP1_TRANSLATOR_H
#include <arpa/inet.h>
#include "fileReader.h"
#define WORD_BUF 130

typedef struct translator_t{
	char* header;
	char* body;
	int header_len;
	int body_len;
}translator_t;

/*Inicializa el header_len y el body_len
*/

int translator_create(translator_t *self);

/*Divide el la linea del input de @param file_reader en header y body
*/

int translator_make_message(translator_t *self, 
	file_reader_t *file_reader, int id);

/*separa por espacios a la linea del input y los guarda
en un array de strings @param words
*/

int _translator_separator(char* input_line, char words[][WORD_BUF]);

/*separa al metodo de los argumentos y los guarda en
@param words
*/
int _translator_method_separator(char words[][WORD_BUF]);

/*separa cada uno de los argumentos y los guarda
en @param words
*/
int _translator_arg_separator(char words[][WORD_BUF], char* arg_ptr);

/*crea un array en el heap para almacenar el cuerpo del mensaje*/
int _translator_make_body(translator_t *self, 
	char words[][WORD_BUF], int n_arg);


/*concatena los parametros y los strings para conformar
el cuerpo del mensaje segun protocolo dbus
*/
int _translator_append_body(char** cursor, char words[][WORD_BUF], int n_arg);

/*Arma el header concatenando cada uno de los parametros
del array
*/
int _translator_make_header(translator_t *self, 
	int n_arg, int id, char words[][WORD_BUF]);

/*Arma el parametro firma del header y lo agrega al header
*/
int	_translator_append_header_signature(char** cursor, 
	int body_len, int id, int total_header_len);

/* Arma el parametro ruta y lo agrega al header
*/
int _translator_append_path(char** cursor, char* word);

/* Arma el parametro ruta y lo agrega al header
*/
int _translator_append_destiny(char** cursor, char* word);


/* Arma el parametro interfaz y lo agrega al header
*/
int _translator_append_interface(char** cursor, char* interface);


/* Arma el parametro metodo y lo agrega al header
*/
int _translator_append_method(char** cursor, char* method);


/* Arma el parametro firma y lo agrega al header
*/
int _translator_append_signature(char** cursor, int n_arg);

/*Convierte valores a little endian independientemente
de su endianess
*/
uint32_t _translator_value_to_little_endian(uint32_t value);

/*Calcula el largo de un string redondeando a multipo de 8
*/
int _translator_calculate_len(char* word);

/*Calcula el largo del cuerpo
*/
int _translator_calculate_body_len(char words[][WORD_BUF], int n_arg);

/*Calcula el largo del parametro firma
*/
int _translator_calculate_signature_len(int n_arg);

/* Devuelve @param value redondeado a multiplo de 8
*/
int _translator_round_up(int value);

/*libera la memoria del array header y el body*/
int translator_free(translator_t *self);

/*Destructor*/
int translator_destroy(translator_t *self);


#endif
