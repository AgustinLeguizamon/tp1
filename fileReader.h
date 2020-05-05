#ifndef TP1_FILEREADER_H
#define TP1_FILEREADER_H

#include <stdio.h>
#include <stdbool.h>

typedef struct file_reader_t{
	FILE *input;
	char* input_line;
}file_reader_t;

/*Asigna a input la ruta del archivo o stdin por defecto
*/
int file_reader_create(file_reader_t *self, int argc, const char* file_name);

/*Lee el input hasta un newline y devuelve un puntero a la linea leida
*/
int file_reader_read_line(file_reader_t *self, int *file_status);

/*Lee los siguientes 32 bytes del input o hasta encontrar \n y
los coloca en @param buffer
*/
int _file_reader_read_block(file_reader_t *self, char buffer[]);

/*Setea 0 en os bytes del buffer desde el \n en adelante
*/
int _file_reader_clean_buffer(char buffer[], int index);


/*libera el buffer input_line*/
int file_reader_free(file_reader_t *self);


/*libera el archivo o no hace nada si el input es stdin
*/
int file_reader_destroy(file_reader_t *self);

#endif
