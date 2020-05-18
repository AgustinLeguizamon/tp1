#ifndef TP1_FILEREADER_H
#define TP1_FILEREADER_H

#include <stdio.h>
#include <stdbool.h>

#define READ_SIZE 32

typedef struct file_reader_t{
	FILE *input;
	char* input_line;
	int file_status;
	char next_line_buffer[READ_SIZE];
}file_reader_t;

/*Asigna a input la ruta del archivo o stdin por defecto
*/
int file_reader_create(file_reader_t *self, int argc, const char* file_name);

/*Lee el input hasta un newline y devuelve un puntero a la linea leida
*/
int file_reader_read_line(file_reader_t *self);

/*devuelve true si no se llego al EOF
*/
bool file_reader_status(file_reader_t *self);

/*libera el buffer input_line*/
int file_reader_free_input(file_reader_t *self);

/*libera el archivo o no hace nada si el input es stdin
*/
int file_reader_destroy(file_reader_t *self);

#endif
