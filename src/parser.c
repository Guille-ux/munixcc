#include "../include/compat.h"
#include "../include/parser.h"

size_t label = 0;
size_t tok_index = 0;
// usaremos etiquetas con hasta 4 dígitos por función!

// TODO: parsePrototype

// definiciones de funciones internas
static int parseStatement(TokenC *tokens, BufferI *buffer);
static int parsePrototype(TokenC *tokens, BufferI *buffer);
static int parseIf(TokenC *tokens, BufferI *buffer);
static int parseScope(TokenC *tokens, BufferI *buffer);

// definiciones para parseo de expresiones
static int parseExpression(TokenC *tokens, BufferI *buffer);
static int parsePrimary(TokenC *tokens, BufferI *buffer);
static int parseUnary(TokenC *tokens, BufferI *buffer);
static int parseLogical(TokenC *tokens, BufferI *buffer);
static int parseAdditive(TokenC *tokens, BufferI *buffer);
static int parseMultiplicative(TokenC *tokens, BufferI *buffer);


static int parseCallArg(TokenC *tokens, BufferI *buffer);

// cosas de utilidad
static TokenC *eat(TokenC *tokens) { // básicamente se come el token,rico rico
	return &tokens[tok_index++];
}

static TokenC *peek(TokenC *tokens) { // te devuelve el actual sin avanzar
	return &tokens[tok_index];
}

int mcc_parse_program(TokenC *tokens, BufferI *buffer) {
	tok_index = 0;
	label = 0;
	while (tokens[i].type!=TOKEN_EOF) {
		// llamamos a parse statement
		if (parseStatement(tokens, buffer) != 0) {
			// ERROR!
		}
	}
	return 0;	
}

// CONVENCIÓN DE OPERANDOS
// EL IZQUIERDO VA EN ECX
// Y EL DERECHO EN EAX
// USAMOS EL STACK (push, pop etc)
// EL RESULTADO SE GUARDA EN EAX
// en expresiones
// empujamos el resultado de eax a la pila, calculamos el otro resultado y
// bajamos el valor de la pila a ecx

/*
 * Aquí voy a dedicar un momento a pensar los statements
 *
 * definition
 * for
 * while
 * if - else
 * switch
 * function
 * 
 *
 */


// TODO: AÑADIR CÓDIGO DE parsePrototype y acabar parseStatement

static int parseStatement(TokenC *tokens, BufferI buffer) {
	switch (peek(tokens)->type) {
		case C_TOKEN_EXTERN: return parsePrototype(tokens, buffer);
		case C_TOKEN_IF: return parseIf(tokens, buffer);
		case C_TOKEN_LEFT_BRACE: return parseScope(tokens, buffer);
		case C_TOKEN_RIGHT_BRACE: return 0; // IMPORTANTE
		default: break; // BRUH, NO SE NADA
	}
	return -1; // no deberia haber llegado hasta aqui
}

// las 3 funciones para expresiones aritméticas

static int parseExpression(TokenC *tokens, BufferI *buffer) {
	return parseLogical(TokenC *tokens, BufferI *buffer);
}


// NOTA, RECORDAR QUE EN MUNIXCC SI USAS BOOLEANOS TIENES QUE PASAR BOOLEAN

/*
 * PRECEDENCIA DE OPERACIONES EN MUNIXCC
 *
 * (de menor a mayor precedencia)
 *
 * OperadoresLógicos : &, ^, |, ||, &&, ==, !=, <, >, >= y <=
 * Operadores de Adición : +, -, >> y <<
 * Operadores de Multiplicación : *, /, %
 * Operadores Unarios : -, *, !, ~
 * Expresiones primarias: &num, (), l-value....
 * 
 */

static int parseLogical(TokenC *tokens, BufferI *buffer) {
	parseAdditive(tokens, buffer);
	while  (peek(tokens)->type == C_TOKEN_AND || C_TOKEN_BYTEWISE_AND ||
		peek(tokens)->type == C_TOKEN_OR || C_TOKEN_BYTEWISE_OR ||
		peek(tokens)->type == C_TOKEN_BYTEWISE_XOR ||
		peek(tokens)->type == C_TOKEN_EQUAL ||
		peek(tokens)->type == C_TOKEN_LESS ||
		peek(tokens)->type == C_TOKEN_GREATER ||
		peek(tokens)->type == C_TOKEN_NOT_EQUAL ||
		peek(tokens)->type == C_TOKEN_LESS_EQUAL ||
		peek(tokens)->type == C_TOKEN_GREATER_EQUAL) {
		

		// usar push eax para guardarlo en la pila.
		buffer->emitText(buffer, "push eax\n");
		
		TokenC *tok = eat(tokens);

		if (parseAdditive(tokens, buffer)!=0) {
			// ERROR
			// esto es porque hubo un error durante el parseo
			// de aditivos
		}
		
		// extraemos a ecx, IMPORTANTE
		buffer->emitText(buffer, "pop ecx\n");

		// ahora distinguimos la operación
		switch (tok->type) {
			case C_TOKEN_AND:
			case C_TOKEN_BYTEWISE_AND:
				// comparamos con and
				buffer->emitText(buffer, "and eax, ecx\n");
				break;
			case C_TOKEN_OR:
			case C_TOKEN_BYTEWISE_OR:
				// comparamos con or
				buffer->emitText(buffer, "or eax, ecx\n");
				break;
			case C_TOKEN_BYTEWISE_XOR:
				// comparamos con xor
				buffer->emitText(buffer, "xor ecx, eax\n");
				break;
			case C_TOKEN_EQUAL:
				// hace comparación usando cmp
				buffer->emitText(buffer, "cmp ecx, eax\n");
				// mueve a eax 1 en caso de que sean iguales
				// (flag 0 activa)
				buffer->emitText(buffer, "cmovz eax, 0d1\n");
				break;
			case C_TOKEN_LESS:
				// comparamos usando cmp
				buffer->emitText(buffer, "cmp ecx, eax\n");
				// ahora movemos si eax era mayor
				// (la resta dio negativo)
				buffer->emitText(buffer, "cmovl eax, 0d1\n");
				break;
			case C_TOKEN_GREATER:
				// comparamos usando cmp
				buffer->emitText(buffer, "cmp ecx, eax\n");
				// ahora movemos 1 a ecx si era mayor
				// la resta dio positivo
				buffer->emitText(buffer, "cmovg eax, 0d1\n");
				break;
			case C_TOKEN_NOT_EQUAL:
				// comparamos con cmp
				buffer->emitText(buffer, "cmp ecx, eax\n");
				// ahora movemos 1 a ecx si no son iguales
				// (si la resta no da 0)
				buffer->emitText(buffer, "cmovnz eax, 0d1\n");
				break;
			case C_TOKEN_LESS_EQUAL:
				// primero comparamos
				buffer->emitText(buffer, "cmp ecx, eax\n");
				// movemos si son iguales a ecx
				buffer->emitText(buffer, "cmovz ecx, 0d1\n");
				// movemos si eax menor a eax
				buffer->emitText(buffer, "cmovl eax, 0d1\n");
				// hacemos or
				buffer->emitText(buffer, "or eax, ecx\n");
				break;
			case C_TOKEN_GREATER_EQUAL:
				// primero comparamos
				buffer->emitText(buffer, "cmp ecx, eax\n");
				// movemos si son iguales a ecx
				buffer->emitText(buffer, "cmovz ecx, 0d1\n");
				// movemos 1 a eax si eax > ecx
				buffer->emitText(buffer, "cmovl eax, 0d1\n");
				// hacemos or
				buffer->emitText(buffer, "or eax, ecx\n");
				break;
		}

		
	}
	return 0;
}
static int parseAdditive(TokenC *tokens, BufferI *buffer) {
	if (parseMultiplicative(tokens, buffer)!=0) /* ERROR */;
	while  (peek(tokens)->type == C_TOKEN_ADD ||
		peek(tokens)->type == C_TOKEN_SUB ||
		peek(tokens)->type == C_TOKEN_RIGHT_SHIFT ||
		peek(tokens)->type == C_TOKEN_LEFT_SHIFT) {
	
		buffer->emitText(buffer, "push eax\n"); // dejamos eax en stack

		TokenC *tok = eat(tokens);

		// llamamos a parseMultiplicative para que haga el siguiente
		// cálculo
		if (parseMultiplicative(tokens, buffer)!=0) /* ERROR */;
		
		// extraemos el operando izquierdo del stack
		buffer->emitText(buffer, "pop ecx\n");
		
		// recuerda que el resultado debe acabar en eax
		switch (tok->type) {
			case C_TOKEN_ADD:
				// HACEMOS SUMA
				buffer->emitText(buffer, "add eax, ecx\n");
				break;
			case C_TOKEN_SUB:
				// hacemos la resta, recuerda ecx - eax
				// ecx es el izquierdo
				buffer->emitText(buffer, "sub ecx, eax\n");
				// movemos resultado a eax
				buffer->emitText(buffer, "mov eax, ecx\n");
				break;
			case C_TOKEN_RIGHT_SHIFT:
				// hacemos shift hacia derecha
				buffer->emitText(buffer, "shr ecx, eax\n");
				// movemos el resultado a eax
				buffer->emitText(buffer, "mov eax, ecx\n");
				break;
			case C_TOKEN_LEFT_SHIFT:
				// hacemos shift a la izquierda
				buffer->emitText(buffer, "shl ecx, eax\n");
				// movemos resultado a eax
				buffer->emitText(buffer, "mov eax, ecx\n");
				break;
		}

	}
	return 0;
}
static int parseMultiplicative(TokenC *tokens, BufferI *buffer) {
	if (parseUnary(tokens, buffer)!=0) /*ERROR*/;
	while  (peek(tokens)->type == C_TOKEN_STAR ||
		peek(tokens)->type == C_TOKEN_DIV ||
		peek(tokens)->type == C_TOKEN_MOD) {
		
		// empujamos el resultado
		buffer->emitText(buffer, "push eax\n");

		TokenC *tok = eat(tokens);
		if (parseUnary(tokens, buffer)!=0) /*ERROR*/;
		// bajamos el operando izquierdo
		buffer->emitText(buffer, "pop ecx\n");
		switch (tok->type) {
			case C_TOKEN_STAR:
				// hacemos multiplicación
				// como se guarda en eax, ahí se acaba
				// el trabajo
				buffer->emitText(buffer, "imul ecx\n");
				break;
			case C_TOKEN_DIV:
				// primero intercambiamos ecx con eax
				// es por como funciona la división
				buffer->emitText(buffer, "push ecx\n");
				buffer->emitText(buffer, "mov ecx, eax\n");
				buffer->emitText(buffer, "pop eax\n");
				// hacemos división
				// esto es más difícil
				// porque en imul el resultado se guarda
				// en EDX:EAX siendo EDX la parte alta
				// aqui como es la inversa
				// hay que pasar hacer mov a EDX de eax
				// y luego usar SAR para llenarlo del signo
				// entonces, usaria CDQ si lo tuviese
				// pero no lo tengo
				buffer->emitText(buffer, "mov edx, eax\n");
				buffer->emitText(buffer, "sar edx, 0d31\n");
				// ahora ya podemos dividir
				buffer->emitText(buffer, "idiv ecx\n");
				break;
			case C_TOKEN_MOD:
				// toca hacer un poco lo de antes, pero nos llevaremos el resto
				// primero intercambiar ECX con EAX
				buffer->emitText(buffer, "push eax\n");
				buffer->emitText(buffer, "mov eax, ecx\n");
				buffer->emitText(buffer, "pop ecx\n");
				// ahora tenemos que preparar edx
				buffer->emitText(buffer, "mov edx, eax\n");
				buffer->emitText(buffer, "sar edx, 0d31\n");
				// ahora dividimos
				buffer->emitText(buffer, "idiv ecx\n");
				// y ahora, finalmente movemos el resto a eax
				buffer->emitText(buffer, "mov eax, edx\n");
				break;
		}

	}
	return 0;
}

static int parseUnary(TokenC *tokens, BufferI *buffer) {
	TokenC *tok = eat(tokens);
	switch (tok->type) {
		case C_TOKEN_STAR: // desreferenciación
			// movemos el contenido de la dirección de eax en eax
			buffer->emitText(buffer, "mov eax, Meax\n");
			break;
		case C_TOKEN_SUB: // cambiarle signo
			// solo usamos neg y ya
			buffer->emitText(buffer, "neg eax\n");
			break;
		case C_TOKEN_BANG: // cambiar de true a false y viceversa
		case C_TOKEN_BYTEWISE_NOT: // hacer not
			// el lógico y el bytewise van en el mismo
			// si quieres booleanos usa booleanos
			buffer->emitText(buffer, "not eax\n");
			break;
		default: // continuamos con nuestra vida
			tok_index--;
			parsePrimary(tokens, buffer);
			return 0;
	}

	parseUnary(tokens, buffer); // tiene que ser recursivo
	return 0;
}

// remember
/*
 * typedef struct {
 *	TokenTypeC type;
 *	char *start;
 *	size_t len;
 *	int line;
 * } TokenC;
 */

static int parsePrimary(TokenC *tokens, BufferI *buffer) {
	TokenC *tok = eat(tokens);
	switch (tok->type) {
		case C_TOKEN_LEFT_PAREN:
			parseExpression(tokens, buffer);
			if (eat(tokens)->type != C_TOKEN_RIGHT_PAREN) {
				// ERROR, btw
				// idk, just say something
				return -1;
			}
			break;
		case C_TOKEN_NUMBER:
			// mov the number to eax
			buffer->emitText(buffer, "mov eax, ");
			// trying to make this work
			{
			char *tmp = (char*)malloc(tok->len+2);
			memcpy(tmp, tok->start, tok->len);
			tmp[tok->len]='\n';
			tmp[tok->len+1]='\0';
			buffer->emitText(buffer, tmp);
			free(tmp); // i have almost forgotten that!
			}
			break;
		case C_TOKEN_DECIMAL:
			// idk, just tell that float and double aren't
			// supported yet.
			return -1;
		case C_TOKEN_IDENTIFIER:
			// something that handles_identifiers i guess.
			tok_index--;
			return handle_identifier(tokens, buffer);
		case C_TOKEN_BYTEWISE_AND:
			// similar	
			handle_address(tokens, buffer); // esta función devuelve tamaño
			return 0;
		case C_TOKEN_CHARACTER:
			// something like the other, but this time is easier
			// i guess
			buffer->emitText(buffer, "mov eax, ");
			{
			char *tmp = (char*)malloc(tok->len+2);
			memcpy(tmp, tok->start, tok->len);
			tmp[tok->len]='\n';
			tmp[tok->len+1]='\0';
			buffer->emitText(buffer, tmp);
			free(tmp);
			}
			// oh, it's the same
			break;

	}
	tok = eat(tokens);
	if (tok->type == C_TOKEN_LEFT_PAREN) {
		char *arr;
		// OH, estamos ante una llamada a función
		buffer->emitText(buffer, "push eax\n");
		int nargs = parseCallArg(TokenC *tokens, BufferI *buffer);
		buffer->emitText(buffer, "mov eax, "); 
		arr = (char*)malloc(64);
		buffer->emitText(buffer, int2char(arr, 64, nargs*4));
		buffer->emitText(buffer, "\n");
		buffer->emitText(buffer, "add eax, esp\n");
		buffer->emitText(buffer, "mov eax, Meax\n"); 
		buffer->emitText(buffer, "call eax\n");
		buffer->emitText(buffer, "mov ecx, eax\n");
		buffer->emitText(buffer, "mov eax, ");
		buffer->emitText(buffer, int2char(arr, 64, nargs*4+4));
		buffer->emitText(buffer, "\n");
		free(arr);
		buffer->emitText(buffer, "add esp, eax\n");
		buffer->emitText(buffer, "mov eax, ecx\n");
	} else {
		tok_index--;
	}
	return 0;
}

static int parseCallArg(TokenC *tokens, BufferI *buffer) {
	int nargs = 1;
	TokenC *tok = eat(tokens);
	while (1) {
		if (tok->type == C_TOKEN_RIGHT_PAREN) {
			nargs = (nargs > 1) ? nargs : 0;
			break;
		} else if (tok->type = C_TOKEN_COMMA) {
			nargs++;
		} else {
			if (parseExpression(tokens, buffer)!=0) {
				// ERROR
			}
			buffer->emitText(buffer, "push eax\n");
		}
	}
	return nargs;
}

static int parsePrototype(TokenC *tokens, BufferI *buffer) {

}

static int parseIf(TokenC *tokens, BufferI *buffer) {
	tok_index++;
	TokenC *tok = eat(tokens);
	
	if (tok->type != C_TOKEN_LEFT_PAREN) {
		// ERROR
		return -1;
	}
	
	if (0 != parseExpression(tokens, buffer)) {
		// ERROR
		return -1;
	}

	if (eat(tokens)->type!=C_TOKEN_RIGHT_PAREN) {
		// ERROR, FALTA EL PARENTESIS
		return -1;
	}

	if (parseStatement(tokens, buffer)!=0) {
		// ERROR
		return -1;
	}


	// emitimos código si se cumple
	

	// ahora, si se cumple, hacemos salto a fin, bueno, no, hacemos salto
	// si no se cumpliese hubiesemos ido a ELSE

	char *arr = (char*)malloc(MCC_MAX_SYMBOL_NAME+1);
	arr[MCC_MAX_SYMBOL_NAME] = '\0';
	arr[0] = 'L';
	arr[1] = '_';
	int2char(arr, 9, label++); // el 9 esta ahi porque permito hasta 6
				   // cifras, + 2 de L_ y + 1 de '\0'
	arr[9] = '_';
	memcpy(&arr[10], "FIN", 4);

	// AHORA HACEMOS SALTO AL FIN, de hecho, creo que haremos primero
	buffer->emitText(buffer, "loax ");
	buffer->emitText(buffer, arr);
	buffer->emitText(buffer, "\njmp eax\n"); // hacemos salto a eax

	// AHORA USAMOS ELSE
	memcpy(&arr[10], "ELSE", 5);

	// TRABAJAMOS CON LA ETIQUETA ELSE, si no hay else, sigue siendo
	// necesario, asi es más fácil



	// YA ACABAMOS CON ELSE, AHORA TOCA END
	memcpy(&arr[10], "END", 4);
	// trabajamos con la etiqueta end


	free(arr);
	

	return 0;
}

static int parseScope(TokenC *tokens, BufferI *buffer) {
	eat(tokens);
	CVarTable.current_scope++;
	while (peek(tokens)->type != C_TOKEN_RIGHT_BRACE) {
		if (parseStatement(tokens, buffer)!=0) {
			// ERROR
			return -1
		}
	}
	CVarTable.current_scope--;
	mcc_clean_tab();
	eat(tokens);
	return 0;
}
