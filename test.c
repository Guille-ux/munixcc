

#include <stdio.h>
#include <stdlib.h>
#include <string.h>

#include "include/parser.h"

#define MAX_TOKENS 4096

void print_tokens(TokenC *tokens, int max_tokens)
{
	int i;

	printf("=== TOKENS ===\n");

	for (i = 0; i < max_tokens; i++)
	{
		printf("[%d] type = %d\n", i, tokens[i].type);

		/*
		 * Detenerse cuando aparezca el token
		 * que representa el final de la entrada.
		 */
		if (tokens[i].type == C_TOKEN_EOF)
			break;
	}

	printf("=== FIN TOKENS ===\n");
}

int main(void) {
	const char *source = "int main() { return 0; }";
	TokenC tokens[MAX_TOKENS];
	BufferI output;

	munixcc_initDynamicBuffer(&output);

	if (!munixccLex(source, tokens, MAX_TOKENS)) {
		fprintf(stderr, "Error en el lexer\n");
		output.freeBuff(&output);
		return 1;
	}

	print_tokens(tokens, MAX_TOKENS);

	if (mcc_parse_program(tokens, &output) != 0) {
		fprintf(stderr, "Error en el parser\n");
		output.freeBuff(&output);
		return 1;
	}

	FILE *file = fopen("output.asm", "w");
	if (file == NULL) {
		perror("output.asm");
		output.freeBuff(&output);
		return 1;
	}

	if (fwrite(output.buffer, 1, output.size, file) != output.size) {
		perror("Escribiendo output.asm");
		fclose(file);
		output.freeBuff(&output);
		return 1;
	}

	fclose(file);
	output.freeBuff(&output);

	puts("Ensamblador generado: output.asm");
	return 0;
}
