#include <stdio.h>
#include <string.h>
#include <stdlib.h>
#include "fileReader.h"

#define READ_SIZE 32
#define NEW_LINE '\n'

int file_reader_create(file_reader_t *self, int argc, const char* filename){
	if(argc > 1){
		self->input = fopen(filename, "r");
	} else {
		self->input = stdin;
	}
	return 0;
}

char* file_reader_read_file(file_reader_t *self, int *file_status){
	int found_new_line = 0;
	int size = READ_SIZE;
	char* input_line = malloc(READ_SIZE);
	memset(input_line, 0, READ_SIZE);
	char buffer[READ_SIZE];

	while (found_new_line == 0){
		found_new_line = _file_reader_read_block(self, buffer);	
		strncat(input_line, buffer, sizeof(buffer)-1);	
		if (found_new_line != 1){
			input_line = realloc(input_line, size + READ_SIZE);
			size += READ_SIZE;
		}			
	}

	if (found_new_line == EOF){
		*file_status = EOF;
	}
	
	/*Ve el siguiente caracter por si este es EOF
	caso que se da si la siguiente linea esta vacia
	*/
	if(fgetc(self->input) == EOF){
		*file_status = EOF;
	} else {
		fseek(self->input, -1, SEEK_CUR);
	}

	return input_line;
}

int _file_reader_read_block(file_reader_t *self, char buffer[]){
	int character, i=0, found_new_line = 0;

	while ((i < READ_SIZE-1) && (found_new_line == 0)){
		character = fgetc(self->input);
		if(character != EOF){
			buffer[i] = character;
			if(character == NEW_LINE){
				found_new_line = 1;
				_file_reader_clean_buffer(buffer, i);
			}
		} else {
			found_new_line = EOF;
			_file_reader_clean_buffer(buffer, i);
		}
		i++;
	}
	return found_new_line;
}

int _file_reader_clean_buffer(char buffer[], int index){
	for (int i = index; i < READ_SIZE; ++i){
		buffer[i]=0;
	}

	return 0;
}

int file_reader_destroy(file_reader_t *self){
	if (self->input != stdin){
		fclose(self->input);
	}
	return 0;
}
