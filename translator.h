#ifndef TP1_TRANSLATOR_H
#define TP1_TRANSLATOR_H
#include <arpa/inet.h>

#define WORD_BUF 130

typedef struct message_t{
	char* header;
	char* body;
	int header_len;
	int body_len;
}message_t;

message_t translator_make_message(char* input_line, int id);

int _translator_separator(char* input_line, char words[][WORD_BUF]);

int _translator_method_separator(char words[][WORD_BUF]);

int _translator_arg_separator(char words[][WORD_BUF], char* arg_ptr);

char* _translator_make_body(char words[][WORD_BUF], int n_arg);

int _translator_append_body(char** cursor, char words[][WORD_BUF], int n_arg);

char* translator_get_body();

char* _translator_msg_maker(char words[][WORD_BUF], 
	int n_arg, int id, message_t *message);

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

#endif
