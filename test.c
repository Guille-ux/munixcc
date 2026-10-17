

#include <stdio.h>
#include <stdlib.h>
#include <string.h>

#include "include/parser.h"

#define MAX_TOKENS 4096

const char *token_name(TokenTypeC type)
{
	switch (type) {
		case C_TOKEN_EOF: return "EOF";
		case C_TOKEN_AUTO: return "AUTO";
		case C_TOKEN_DOUBLE: return "DOUBLE";
		case C_TOKEN_INT: return "INT";
		case C_TOKEN_STRUCT: return "STRUCT";
		case C_TOKEN_BREAK: return "BREAK";
		case C_TOKEN_ELSE: return "ELSE";
		case C_TOKEN_LONG: return "LONG";
		case C_TOKEN_SWITCH: return "SWITCH";
		case C_TOKEN_CASE: return "CASE";
		case C_TOKEN_ENUM: return "ENUM";
		case C_TOKEN_REGISTER: return "REGISTER";
		case C_TOKEN_TYPEDEF: return "TYPEDEF";
		case C_TOKEN_CHAR: return "CHAR";
		case C_TOKEN_EXTERN: return "EXTERN";
		case C_TOKEN_RETURN: return "RETURN";
		case C_TOKEN_UNION: return "UNION";
		case C_TOKEN_CONST: return "CONST";
		case C_TOKEN_FLOAT: return "FLOAT";
		case C_TOKEN_SHORT: return "SHORT";
		case C_TOKEN_UNSIGNED: return "UNSIGNED";
		case C_TOKEN_CONTINUE: return "CONTINUE";
		case C_TOKEN_FOR: return "FOR";
		case C_TOKEN_SIGNED: return "SIGNED";
		case C_TOKEN_VOID: return "VOID";
		case C_TOKEN_DEFAULT: return "DEFAULT";
		case C_TOKEN_GOTO: return "GOTO";
		case C_TOKEN_SIZEOF: return "SIZEOF";
		case C_TOKEN_VOLATILE: return "VOLATILE";
		case C_TOKEN_DO: return "DO";
		case C_TOKEN_IF: return "IF";
		case C_TOKEN_STATIC: return "STATIC";
		case C_TOKEN_WHILE: return "WHILE";
		case C_TOKEN_ASM: return "ASM";
		case C_TOKEN_NUMBER: return "NUMBER";
		case C_TOKEN_DECIMAL: return "DECIMAL";
		case C_TOKEN_STRING: return "STRING";
		case C_TOKEN_CHARACTER: return "CHARACTER";
		case C_TOKEN_IDENTIFIER: return "IDENTIFIER";
		case C_TOKEN_NEWLINE: return "NEWLINE";
		case C_TOKEN_BACKSLASH: return "BACKSLASH";
		case C_TOKEN_HORIZONTAL_TAB: return "HORIZONTAL_TAB";
		case C_TOKEN_BACKSPACE: return "BACKSPACE";
		case C_TOKEN_QUESTION_MARK: return "QUESTION_MARK";
		case C_TOKEN_SINGLE_QUOTE: return "SINGLE_QUOTE";
		case C_TOKEN_DOUBLE_QUOTE: return "DOUBLE_QUOTE";
		case C_TOKEN_CARRIAGE_RETURN: return "CARRIAGE_RETURN";
		case C_TOKEN_FORMFEED: return "FORMFEED";
		case C_TOKEN_AUDIBLE_ALERT: return "AUDIBLE_ALERT";
		case C_TOKEN_WHITE_SPACE: return "WHITE_SPACE";
		case C_TOKEN_LEFT_PAREN: return "LEFT_PAREN";
		case C_TOKEN_RIGHT_PAREN: return "RIGHT_PAREN";
		case C_TOKEN_LEFT_BRACKET: return "LEFT_BRACKET";
		case C_TOKEN_RIGHT_BRACKET: return "RIGHT_BRACKET";
		case C_TOKEN_LEFT_BRACE: return "LEFT_BRACE";
		case C_TOKEN_RIGHT_BRACE: return "RIGHT_BRACE";
		case C_TOKEN_COLON: return "COLON";
		case C_TOKEN_SEMICOLON: return "SEMICOLON";
		case C_TOKEN_COMMA: return "COMMA";
		case C_TOKEN_DOT: return "DOT";
		case C_TOKEN_TERNARY: return "TERNARY";
		case C_TOKEN_ADD: return "ADD";
		case C_TOKEN_SUB: return "SUB";
		case C_TOKEN_STAR: return "STAR";
		case C_TOKEN_DIV: return "DIV";
		case C_TOKEN_BANG: return "BANG";
		case C_TOKEN_AND: return "AND";
		case C_TOKEN_OR: return "OR";
		case C_TOKEN_MOD: return "MOD";
		case C_TOKEN_BYTEWISE_AND: return "BYTEWISE_AND";
		case C_TOKEN_BYTEWISE_OR: return "BYTEWISE_OR";
		case C_TOKEN_BYTEWISE_NOT: return "BYTEWISE_NOT";
		case C_TOKEN_BYTEWISE_XOR: return "BYTEWISE_XOR";
		case C_TOKEN_LESS: return "LESS";
		case C_TOKEN_GREATER: return "GREATER";
		case C_TOKEN_EQUAL: return "EQUAL";
		case C_TOKEN_ASSIGN: return "ASSIGN";
		case C_TOKEN_NOT_EQUAL: return "NOT_EQUAL";
		case C_TOKEN_RIGHT_SHIFT: return "RIGHT_SHIFT";
		case C_TOKEN_LEFT_SHIFT: return "LEFT_SHIFT";
		case C_TOKEN_LESS_EQUAL: return "LESS_EQUAL";
		case C_TOKEN_GREATER_EQUAL: return "GREATER_EQUAL";
		case C_TOKEN_ARROW: return "ARROW";
		case C_TOKEN_POSTFIX_ADD: return "POSTFIX_ADD";
		case C_TOKEN_POSTFIX_SUB: return "POSTFIX_SUB";
		case C_TOKEN_SUB_ASSIGN: return "SUB_ASSIGN";
		case C_TOKEN_ADD_ASSIGN: return "ADD_ASSIGN";
		case C_TOKEN_MUL_ASSIGN: return "MUL_ASSIGN";
		case C_TOKEN_DIV_ASSIGN: return "DIV_ASSIGN";
		case C_TOKEN_MOD_ASSIGN: return "MOD_ASSIGN";
		case C_TOKEN_RIGHT_SHIFT_ASSIGN: return "RIGHT_SHIFT_ASSIGN";
		case C_TOKEN_LEFT_SHIFT_ASSIGN: return "LEFT_SHIFT_ASSIGN";
		case C_TOKEN_BYTEWISE_OR_ASSIGN: return "BYTEWISE_OR_ASSIGN";
		case C_TOKEN_BYTEWISE_AND_ASSIGN: return "BYTEWISE_AND_ASSIGN";
		case C_TOKEN_BYTEWISE_XOR_ASSIGN: return "BYTEWISE_XOR_ASSIGN";
		case C_TOKEN_ATTRIBUTE: return "ATTRIBUTE";
		case C_TOKEN_ASMBLOCK: return "ASMBLOCK";
		case C_TOKEN_SIZE_T: return "SIZE_T";
		case C_TOKEN_U32: return "U32";
		case C_TOKEN_I32: return "I32";
		case C_TOKEN_U16: return "U16";
		case C_TOKEN_I16: return "I16";
		case C_TOKEN_U8: return "U8";
		case C_TOKEN_I8: return "I8";
		case C_TOKEN_AS: return "AS";
		case C_TOKEN_INTERN_NULL: return "INTERN_NULL";
		default: return "UNKNOWN_TOKEN";
	}
}

void print_tokens(TokenC *tokens, int max_tokens)
{
	printf("=== TOKENS ===\n");

	for (int i = 0; i < max_tokens; i++) {
		TokenC *tok = &tokens[i];

		printf("[%d] %-22s type=%d line=%d text=\"%.*s\"\n",
			i,
			token_name(tok->type),
			(int)tok->type,
			tok->line,
			(int)tok->len,
			tok->start ? tok->start : "");

		if (tok->type == C_TOKEN_EOF)
			break;
	}

	printf("=== FIN TOKENS ===\n");
}

int main(void) {
	const char *source = "int main() { return 0d0; }";
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
