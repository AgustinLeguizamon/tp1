#ifndef TP1_FILEREADER_H
#define TP1_FILEREADER_H

#include <stdio.h>

typedef struct file_reader_t{
	FILE *input;
}file_reader_t;

int file_reader_create(file_reader_t *self, int argc, const char* file_name);

char* file_reader_read_file(file_reader_t *self, int *file_status);

int _file_reader_read_block(file_reader_t *self, char buffer[]);

int _file_reader_clean_buffer(char buffer[], int index);

int file_reader_destroy(file_reader_t *self);

#endif
