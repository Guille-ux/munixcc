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
