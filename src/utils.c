#include "../include/utils.h"
#include <stdbool.h>

/*
 *
 * Implementaciones de utilidades
 *
 */

bool is_identifier_ch(char c) {
	return (isalnum(c) || c == '_'); // solo son válidos letras, números
					 // y '_'
}

bool is_delimiter(char c) {
	return !is_identifier_ch(c); // si no es válido, entonces delimita fin
}

char *int2char(char *string, size_t len, size_t num) {
	string[--len] = '\0';
	while (len-- > 0) {
		string[len] = '0' + num % 10;
		if (num < 10) break; // llegamos al final del número
		num /= 10;
	}
	return string;
}
