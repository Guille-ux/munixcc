#include "../include/compat.h"
#include "../include/parser.h"

size_t label = 0;
size_t tok_index = 0;
// usaremos etiquetas con hasta 4 dígitos por función!

// definiciones de funciones internas
static int parseStatement(TokenC *tokens, BufferI *buffer);
static int parsePrototype(TokenC *tokens, BufferI *buffer);

static TokenC *eat(TokenC *tokens) { // básicamente se come el token,rico rico
	return &tokens[tok_index++];
}

static TokenC *peek(TokenC *tokens) { // te devuelve el actual sin avanzar
	return &tokens[tok_index];
}

int mcc_parse_program(TokenC *tokens, BufferI *buffer) {
	while (tokens[i].type!=TOKEN_EOF) {
		// llamamos a parse statement
		if (parseStatement(tokens, buffer) != 0) {
			// ERROR!
		}
	}
	return 0;	
}

// TODO: AÑADIR CÓDIGO DE parsePrototype y acabar parseStatement

static int parseStatement(TokenC *tokens, BufferI buffer) {
	switch (eat(tokens)->type) {
		case C_TOKEN_EXTERN: return parsePrototype(tokens, buffer);
		default: break; // BRUH, NO SE NADA
	}
	return -1; // no deberia haber llegado hasta aqui
}
