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
			return newTokC(start, 1, g_line, C_TOKEN_WHITE_SPACE);
		case '{': return newTokC(start, 1, g_line, C_TOKEN_LEFT_BRACE);
		case '}': return newTokC(start, 1, g_line, C_TOKEN_RIGHT_BRACE);
		case '[': return newTokC(start, 1, g_line, C_TOKEN_LEFT_BRACKET);
		case ']': return newTokC(start, 1, g_line, C_TOKEN_RIGHT_BRACKET);
		case '(': return newTokC(start, 1, g_line, C_TOKEN_LEFT_PAREN);
		case ')': return newTokC(start, 1, g_line, C_TOKEN_RIGHT_PAREN);
		case '.': return newTokC(start, 1, g_line, C_TOKEN_DOT);
		case ':': return newTokC(start, 1, g_line, C_TOKEN_COLON);
		case ';': return newTokC(start, 1, g_line, C_TOKEN_SEMICOLON);
		case ',': return newTokC(start, 1, g_line, C_TOKEN_COMMA);
		case '?': return newTokC(start, 1, g_line, C_TOKEN_TERNARY);
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
		cursor+=2;
		if (ch=='=') {
			return newTokC(start, 2, g_line, C_TOKEN_EQUAL);
		} else {
			cursor--;
			return newTokC(start, 1, g_line, C_TOKEN_ASSIGN);
		}
	} else if (*cursor=='!') {
		char ch=peekChar();
		cursor+=2;
		if (ch=='=') {
			return newTokC(start, 2, g_line, C_TOKEN_NOT_EQUAL);
		} else {
			cursor--;
			return newTokC(start, 1, g_line, C_TOKEN_BANG);
		}
	} else if (*cursor=='*') {
		char ch=peekChar();
		cursor+=2;
		if (ch=='=') {
			return newTokC(start, 2, g_line, C_TOKEN_MUL_ASSIGN);
		} else {
			cursor--;
			return newTokC(start, 1, g_line, C_TOKEN_STAR);
		}
	} else if (*cursor=='/') {
		char ch=peekChar();
		cursor+=2;
		if (ch=='=') {
			return newTokC(start, 2, g_line, C_TOKEN_DIV_ASSIGN);
		} else {
			cursor--;
			return newTokC(start, 1, g_line, C_TOKEN_DIV);
		}
	} else if (*cursor=='%') {
		char ch=peekChar();
		cursor+=2;
		if (ch=='=') {
			return newTokC(start, 2, g_line, C_TOKEN_MOD_ASSIGN);
		} else {
			cursor--;
			return newTokC(start, 1, g_line, C_TOKEN_MOD);
		}
	} else if (*cursor=='&') {
		char ch=peekChar();
		cursor+=2;
		if (ch=='=') {
			return newTokC(start, 2, g_line, C_TOKEN_BYTEWISE_AND_ASSIGN);
		} else if (ch=='&') {
			return newTokC(start, 2, g_line, C_TOKEN_AND);
		} else {
			cursor--;
			return newTokC(start, 1, g_line, C_TOKEN_BYTEWISE_AND);
		}
	} else if (*cursor=='|') {
		char ch=peekChar();
		cursor+=2;
		if (ch=='|') {
			return newTokC(start, 2, g_line, C_TOKEN_OR);
		} else if (ch=='=') {
			return newTokC(start, 2, g_line, C_TOKEN_BYTEWISE_OR_ASSIGN);
		} else {
			cursor--;
			return newTokC(start, 1, g_line, C_TOKEN_BYTEWISE_OR);
		}
	} else if (*cursor=='^') {
		char ch=peekChar();
		cursor+=2;
		if (ch=='=') {
			return newTokC(start, 2, g_line, C_TOKEN_BYTEWISE_XOR_ASSIGN);
		} else {
			cursor--;
			return newTokC(start, 1, g_line, C_TOKEN_BYTEWISE_XOR);
		}
	}
	// números, va a esperar a que no sea número
	if (isdigit(*cursor)) { // los números mepiezan por 0
		if (*cursor!='0') {
			// Error!
			// Numbers must start with a prefix
		}
		*cursor++;
		if (isdigit(*cursor)) {
			// Error !
			// Numbers must start with a prefix
		}
		*cursor++;
		while (isalnum(*cursor) || *cursor=='.') cursor++;
		if (*cursor=='f') {
			cursor++;
			return newTokC(start, cursor-start-1, g_line, C_TOKEN_DECIMAL);
		} else {
			return newTokC(start, cursor-start, g_line, C_TOKEN_NUMBER);
		}
	} 

	// ahora carácteres
	if (*cursor=='\'') {
		if (*(++cursor)=='\\') {
			cursor++;
		}
		if (*cursor!='\'') {
			// Error, quote missing
		}
		return newTokC(start, cursor-start, g_line, C_TOKEN_CHARACTER);

	}

	// ahora cadenas de texto
	// TODO adaptar para soportar \' y \" y demás símbolos
	if (*cursor=='\"') {
		while (*cursor!='\"' && *cursor!='\0') cursor++;
		if (*cursor=='\0') {
			// Error!
		}
		cursor++;
		return newTokC(start+1, cursor-start-1, g_line, C_TOKEN_STRING);
	}

	// ahora keywords e identificadores
	if (is_identifier_ch(*cursor)) {
		if (munixccPattern("int")) return newTokC(start, cursor-start, g_line, C_TOKEN_INT);
		if (munixccPattern("auto")) return newTokC(start, cursor-start, g_line, C_TOKEN_AUTO);
		if (munixccPattern("double")) return newTokC(start, cursor-start, g_line, C_TOKEN_DOUBLE);
		if (munixccPattern("struct")) return newTokC(start, cursor-start, g_line, C_TOKEN_STRUCT);
		if (munixccPattern("break")) return newTokC(start, cursor-start, g_line, C_TOKEN_BREAK);
		if (munixccPattern("else")) return newTokC(start, cursor-start, g_line, C_TOKEN_ELSE);
		if (munixccPattern("long")) return newTokC(start, cursor-start, g_line, C_TOKEN_LONG);
		if (munixccPattern("switch")) return newTokC(start, cursor-start, g_line, C_TOKEN_SWITCH);
		if (munixccPattern("case")) return newTokC(start, cursor-start, g_line, C_TOKEN_CASE);
		if (munixccPattern("extern")) return newTokC(start, cursor-start, g_line, C_TOKEN_EXTERN);
		if (munixccPattern("return")) return newTokC(start, cursor-start, g_line, C_TOKEN_RETURN);
		if (munixccPattern("union")) return newTokC(start, cursor-start, g_line, C_TOKEN_UNION);
		if (munixccPattern("const")) return newTokC(start, cursor-start, g_line, C_TOKEN_CONST);
		if (munixccPattern("float")) return newTokC(start, cursor-start, g_line, C_TOKEN_FLOAT);
		if (munixccPattern("short")) return newTokC(start, cursor-start, g_line, C_TOKEN_SHORT);
		if (munixccPattern("unsigned")) return newTokC(start, cursor-start, g_line, C_TOKEN_UNSIGNED);
		if (munixccPattern("continue")) return newTokC(start, cursor-start, g_line, C_TOKEN_CONTINUE);
		if (munixccPattern("for")) return newTokC(start, cursor-start, g_line, C_TOKEN_FOR);
		if (munixccPattern("signed")) return newTokC(start, cursor-start, g_line, C_TOKEN_SIGNED);
		if (munixccPattern("void")) return newTokC(start, cursor-start, g_line, C_TOKEN_VOID);
		if (munixccPattern("default")) return newTokC(start, cursor-start, g_line, C_TOKEN_DEFAULT);
		if (munixccPattern("goto")) return newTokC(start, cursor-start, g_line, C_TOKEN_GOTO);
		if (munixccPattern("sizeof")) return newTokC(start, cursor-start, g_line, C_TOKEN_SIZEOF);
		if (munixccPattern("volatile")) return newTokC(start, cursor-start, g_line, C_TOKEN_VOLATILE);
		if (munixccPattern("do")) return newTokC(start, cursor-start, g_line, C_TOKEN_DO);
		if (munixccPattern("if")) return newTokC(start, cursor-start, g_line, C_TOKEN_IF);
		if (munixccPattern("static")) return newTokC(start, cursor-start, g_line, C_TOKEN_STATIC);
		if (munixccPattern("while")) return newTokC(start, cursor-start, g_line, C_TOKEN_WHILE);
		if (munixccPattern("asm")) return newTokC(start, cursor-start, g_line, C_TOKEN_ASM);
		
		// Ahora, los añadidos de Munix
		if (munixccPattern("__attribute__")) return newTokC(start, cursor-start, g_line, C_TOKEN_ATTRIBUTE);
		if (munixccPattern("__asmblock__")) return newTokC(start, cursor-start, g_line, C_TOKEN_ASMBLOCK);
		if (munixccPattern("size_t")) return newTokC(start, cursor-start, g_line, C_TOKEN_SIZE_T);
		if (munixccPattern("u32")) return newTokC(start, cursor-start, g_line, C_TOKEN_U32);
		if (munixccPattern("i32")) return newTokC(start, cursor-start, g_line, C_TOKEN_I32);
		if (munixccPattern("u16")) return newTokC(start, cursor-start, g_line, C_TOKEN_U16);
		if (munixccPattern("i16")) return newTokC(start, cursor-start, g_line, C_TOKEN_I16);
		if (munixccPattern("u8")) return newTokC(start, cursor-start, g_line, C_TOKEN_U8);
		if (munixccPattern("i8")) return newTokC(start, cursor-start, g_line, C_TOKEN_I8);
		if (munixccPattern("as")) return newTokC(start, cursor-start, g_line, C_TOKEN_AS); // nuevo token gente
		else {
			while (is_identifier_ch(*cursor++));
			return newTokC(start, cursor-start, g_line, C_TOKEN_IDENTIFIER);
		}
	}
	return newTokC(NULL, 0, 0, C_TOKEN_INTERN_NULL);
}

static char peekChar(void) {
	return cursor[1];
}

bool munixccLex(const char *text, TokenC *tokens, size_t max_tokens) {
	size_t i=0;
	TokenC tok;
	memset(tokens, 0, sizeof(TokenC)*max_tokens);

	while (i < max_tokens-1) {
		tok = getNextToken();
		if (tok.type == C_TOKEN_WHITE_SPACE) continue;
		if (tok.type == C_TOKEN_EOF) break;
		tokens[i++] = tok;
	}

	tokens[i] = newTokC(0, 1, g_line, C_TOKEN_EOF);
	
	return true; // true si todo va bien
}
