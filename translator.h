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

int translator_create(translator_t *self);

int translator_make_message(translator_t *self, 
	file_reader_t *file_reader, int id);

int _translator_separator(char* input_line, char words[][WORD_BUF]);

int _translator_method_separator(char words[][WORD_BUF]);

int _translator_arg_separator(char words[][WORD_BUF], char* arg_ptr);

int _translator_make_body(translator_t *self, 
	char words[][WORD_BUF], int n_arg);

int _translator_append_body(char** cursor, char words[][WORD_BUF], int n_arg);

int _translator_make_header(translator_t *self, 
	int n_arg, int id, char words[][WORD_BUF]);

int	_translator_append_header_signature(char** cursor, 
	int body_len, int id, int total_header_len);

int _translator_append_path(char** cursor, char* word);

int _translator_append_destiny(char** cursor, char* word);

int _translator_append_interface(char** cursor, char* interface);

int _translator_append_method(char** cursor, char* method);

int _translator_append_signature(char** cursor, int n_arg);

uint32_t _translator_value_to_little_endian(uint32_t value);

int _translator_calculate_len(char* word);

int _translator_calculate_body_len(char words[][WORD_BUF], int n_arg);

int _translator_calculate_signature_len(int n_arg);

int _translator_round_up(int value);

int translator_free(translator_t *self);

#endif
