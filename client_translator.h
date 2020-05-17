#ifndef TP1_TRANSLATOR_H
#define TP1_TRANSLATOR_H
#include <arpa/inet.h>
#include "client_file_reader.h"

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



/*libera la memoria del array header y el body*/
int translator_free(translator_t *self);

/*Destructor*/
int translator_destroy(translator_t *self);


#endif
