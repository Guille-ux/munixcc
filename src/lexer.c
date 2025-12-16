#include "../include/lexer.h"
#include "../include/utils.h"

static char *cursor;
static int g_line=0;

static TokenC getNextToken(void);
static char peekChar(void);

bool munixccPattern(const char *pattern) {
	char *restore = cursor;
	char *pat = pattern;
	while ( (*cursor++) == (*pat++) ) {
		if (*pat=='\0') {
			if (is_delimiter(*cursor)) return true;
			else break;
		}
	}
	cursor=restore;
	return false;
}

TokenC newTokC(char *start, size_t len, int line, TokenTypeC type) {
	TokenC newTok;

	newTok.start = start;
	newTok.len = len;
	newTok.line = line;
	newTok.type = type;

	return newTok;
}

static TokenC getNextToken(void) {
	char *start = cursor;

	switch (*cursor++) {
		case '\0':
			return newTokC(start, 1, g_line, C_TOKEN_EOF);
		case '\n':
			g_line++; // Enter=Cambio de Linea
		case ' ':
		case '\t':
			return newTokC(start, 1, g_line, C_TOKEN_WHITESPACE);
		case '{': return newTokC(start, 1, g_line, C_TOKEN_LEFT_BRACE);
		case '}': return newTokC(start, 1, g_line, C_TOKEN_RIGHT_BRACE);
		case '[': return newTokC(start, 1, g_line, C_TOKEN_LEFT_BRACKET);
		case ']': return newTokC(start, 1, g_line, C_TOKEN_RIGHT_BRACE);
		case '(': return newTokC(start, 1, g_line, C_TOKEN_LEFT_PAREN);
		case ')': return newTokC(start, 1, g_line, C_TOKEN_RIGHT_PAREN);
		case '.': return newTokC(start, 1, g_line, C_TOKEN_DOT);
		case ':': return newTokC(start, 1, g_line, C_TOKEN_COLON);
		case ';': return newTokC(start, 1, g_line, C_TOKEN_SEMICOLON);
		case ',': return newTokC(start, 1, g_line, C_TOKEN_COMMA);
	}

	cursor--;

	// cosas que pueden ser doble carácter, empezando por formateados
	if (*cursor=='\\') {
		char ch = peekChar();
		TokenTypeC type;
		if (ch=='n') {
			type=C_TOKEN_NEWLINE;
		} else if (ch=='\\') {
			type=C_TOKEN_BACKSLASH;
		} else if (ch=='b') {
			type=C_TOKEN_BACKSPACE;
		} else if (ch=='\?') {
			type=C_TOKEN_QUESTION_MARK;
		} else if (ch=='\'') {
			type=C_TOKEN_SINGLE_QUOTE;
		} else if (ch=='\"') {
			type=C_TOKEN_DOUBLE_QUOTE;
		} else if (ch=='r') {
			type=C_TOKEN_CARRIAGE_RETURN;
		} else if (ch=='f') {
			type=C_TOKEN_FORMFEED;
		} else if (ch=='a') {
			type=C_TOKEN_AUDIBLE_ALERT;
		} else {
			// Wtf, Error i guess
		}
		cursor+=2;
		return newTokC(start, 2, g_line, type);
	} 
	// ahora los otros, digo, los operadores
	else if (*cursor=='<') {
		char ch=peekChar();
		cursor+=2;
		if (ch=='<') {
			if (peekChar()=='=') {
				cursor++;
				return newTokC(start, 3, g_line, C_TOKEN_LEFT_SHIFT_ASSIGN);
			}	
			return newTokC(start, 2, g_line, C_TOKEN_LEFT_SHIFT); 
		} else if (ch=='=') {
			return newTokC(start, 2, g_line, C_TOKEN_LESS_EQUAL);
		} else {
			cursor--;
			return newTokC(start, 1, g_line, C_TOKEN_LESS);
		}
	} else if (*cursor=='>') {
		char ch=peekChar();
		cursor+=2;
		if (ch=='>') {
			if (peekChar()=='=') {
				cursor++;
				return newTokC(start, 3, g_line, C_TOKEN_RIGHT_SHIFT_ASSIGN);
			}
			return newTokC(start, 2, g_line, C_TOKEN_RIGHT_SHIFT); 
		} else if (ch=='=') {
			return newTokC(start, 2, g_line, C_TOKEN_GREATER_EQUAL);
		} else {
			cursor--;
			return newTokC(start, 1, g_line, C_TOKEN_GREATER);
		}
	} else if (*cursor=='-') {
		char ch=peekChar();
		cursor+=2;
		if (ch=='>') {
			return newTokC(start, 2, g_line, C_TOKEN_ARROW);
		} else if (ch=='-') {
			return newTokC(start, 2, g_line, C_TOKEN_POSTFIX_SUB);
		} else if (ch=='=') {
			return newTokC(start, 2, g_line, C_TOKEN_SUB_ASSIGN);
		} else {
			cursor--;
			return newTokC(start, 1, g_line, C_TOKEN_SUB);
		}
	} else if (*cursor=='+') {
		char ch=peekChar();
		cursor+=2;
		if (ch=='+') {
			return newTokC(start, 2, g_line, C_TOKEN_POSTFIX_ADD);
		} else if (ch=='=') {
			return newTokC(start, 2, g_line, C_TOKEN_ADD_ASSIGN);
		} else {
			cursor--;
			return newTokC(start, 1, g_line, C_TOKEN_ADD);
		}
	} else if (*cursor=='=') {
		char ch=peekChar();

	}

	return newTokC(NULL, 0, 0, C_TOKEN_INTERN_NULL);
}

static char peekChar(void) {
	return cursor[1];
}
