#include <stdio.h>
#include <string.h>
#include <stdlib.h>
#include "client_file_reader.h"

#define NEW_LINE '\n'
#define ERROR -1

int file_reader_create(file_reader_t *self, int argc, const char* filename){
	if(argc > 3){
		self->input = fopen(filename, "r");
	} else {
		self->input = stdin;
	}
	if(!self->input){
		return ERROR;
	} else {
		self->file_status = 0;
	}
	memset(self->next_line_buffer,0,READ_SIZE);
	return 0;
}

int file_reader_read_line(file_reader_t *self){
		char* input_line = malloc(READ_SIZE);
	memset(input_line,0,READ_SIZE);
	char buffer[READ_SIZE]="";
	int size = READ_SIZE;
	
	strncpy(input_line, self->next_line_buffer, strlen(self->next_line_buffer));
	input_line = realloc(input_line,size+strlen(self->next_line_buffer));
	size +=READ_SIZE;

	bool is_last_line = false;
	do{
		if(fread(buffer, READ_SIZE-1, 1, self->input)==0){
			is_last_line = true;
		}
		strncat(input_line, buffer, strlen(buffer));
		input_line = realloc(input_line,size+READ_SIZE);
		size += READ_SIZE;
	} while ((strchr(buffer, '\n') == NULL) && !is_last_line);

	char* next_line_ptr = strchr(buffer, '\n');
	if(next_line_ptr == NULL){
		next_line_ptr = strchr(buffer, ')');
		next_line_ptr++;
	}

	int dif = strlen(input_line) - strlen(next_line_ptr);
	memset(input_line+dif,0,strlen(next_line_ptr));

	memset(self->next_line_buffer, 0,READ_SIZE);
	strncpy(self->next_line_buffer, next_line_ptr+1, strlen(next_line_ptr));	

	if(fgetc(self->input) == EOF){
		self->file_status = EOF;
	} else {
		fseek(self->input, -1, SEEK_CUR);
	}

	self->input_line = input_line;

	return 0;
}


bool file_reader_status(file_reader_t *self){
	return (self->file_status != EOF);
}


int file_reader_destroy(file_reader_t *self){
	if (self->input != stdin){
		fclose(self->input);
	}
	return 0;
}

int file_reader_free_input(file_reader_t *self){
	free(self->input_line);

	return 0;
}
