#include "../include/compat.h"
#include "../include/parser.h"

// TODO: añadir manejo de errores, acabar el resto de cosas

char *name_base;

size_t label = 0;
size_t tok_index = 0;
// usaremos etiquetas con hasta 4 dígitos por función!

// TODO: parsePrototype

// definiciones de funciones internas
static int parseStatement(TokenC *tokens, BufferI *buffer);
static int parsePrototype(TokenC *tokens, BufferI *buffer);
static int parseIf(TokenC *tokens, BufferI *buffer);
static int parseScope(TokenC *tokens, BufferI *buffer);
static int parseDeclaration(TokenC *tokens, BufferI *buffer);
static int parseIdentifier(TokenC *tokens, BufferI *buffer);
static int parseFunctionDeclaration(TokenC *tokens, BufferI *buffer);
static int parseReturn(TokenC *tokens, BufferI *buffer);
static int parseWhile(TokenC *tokens, BufferI *buffer);
static int parseGoto(TokenC *tokens, BufferI *buffer);

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
	// limpiamos flags globales
	tok_index = 0;
	label = 0;
	/*
	 * El siguiente fragmento es para limpiar las tablas
	 */
	CGlobalTable.symbol_count = 0;
	CVarTable.current_stack_offset = 0;
	CVarTable.current_scope = 0;
	CVarTable.symbol_count = 0;

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

static int parseStatement(TokenC *tokens, BufferI *buffer) {
	switch (peek(tokens)->type) {
		case C_TOKEN_EXTERN: return parsePrototype(tokens, buffer);
		case C_TOKEN_IF: return parseIf(tokens, buffer);
		case C_TOKEN_LEFT_BRACE: return parseScope(tokens, buffer);
		case C_TOKEN_RIGHT_BRACE: return 0; // IMPORTANTE
		case C_TOKEN_SEMICOLON: tok_index++; return 0;
		case C_TOKEN_INT:
		case C_TOKEN_VOID:
			// cosas para declaraciones, digo, es diferente no?
			return parseDeclaration(tokens, buffer);
		case C_TOKEN_IDENTIFIER:
			// gestiona asignaciones y llamadas a funciones que
			// van fuera de una expresión
			return parseIdentifier(tokens, buffer);
		case C_TOKEN_RETURN: return parseReturn(tokens, buffer);
		case C_TOKEN_WHILE: return parseWhile(tokens, buffer);
		case C_TOKEN_GOTO: return parseGoto(tokens, buffer);
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

/*
 *
 *
 *
 *
 * BLOQUE PARA RECORDAR QUE DELANTE ESTA PARSE_PROTOTYPE
 *
 *
 *
 *
 *
 */

static int parsePrototype(TokenC *tokens, BufferI *buffer) {
	// IDK, solo hacer eso
	return 0;
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
	buffer->emitText(buffer, "push eax\n");

	if (eat(tokens)->type!=C_TOKEN_RIGHT_PAREN) {
		// ERROR, FALTA EL PARENTESIS
		return -1;
	}

	// antes de mover el statement, hacemos una comprobación
	char *arr = (char*)malloc(MCC_MAX_SYMBOL_NAME+1);
	memset(arr, '_', MCC_MAX_SYMBOL_NAME+1);
	arr[MCC_MAX_SYMBOL_NAME] = '\0';
	arr[0] = 'L';
	int2char(arr, 9, label++); // el 9 esta ahi porque permito hasta 6
	memcpy(&arr[10], name_base, 15);
	memcpy(&arr[25], "IF\n", 4);	// cifras, + 2 de L_ y + 1 de '\0'

	buffer->emitText(buffer, "loax ");
	buffer->emitText(buffer, arr);
	buffer->emitText(buffer, "mov ecx, eax\n"); // guardamos el valor
	memcpy(&arr[25], "ELSE\n", 6);
	buffer->emitText(buffer, "loax ");
	buffer->emitText(buffer, arr);

	buffer->emitText(buffer, "pop ebx\n"); // preparar la comparación
	buffer->emitText(buffer, "mov edx, 0d1\n");
	buffer->emitText(buffer, "cmp edx, ebx\n"); // comparamo
	
	buffer->emitText(buffer, "cmovz eax, ecx\n"); // movemos si true
	
	buffer->emitText(buffer, "jmp eax\n");

	if (parseStatement(tokens, buffer)!=0) {
		// ERROR
		return -1;
	}


	// ahora, si se cumple, hacemos salto a fin, bueno, no, hacemos salto
	// si no se cumpliese hubiesemos ido a ELSE

	

	// AQUI ANTES DE EMITIR CÓDIGO IF, hacemos un movnz


	memcpy(&arr[25], "END\n", 5);

	// AHORA HACEMOS SALTO AL FIN, de hecho, creo que haremos primero
	buffer->emitText(buffer, "loax ");
	buffer->emitText(buffer, arr);
	buffer->emitText(buffer, "jmp eax\n"); // hacemos salto a eax

	// AHORA USAMOS ELSE
	memcpy(&arr[25], "ELSE\n", 6);
	buffer->emitText(buffer, ".label "); // creamos etiqueta
	buffer->emitText(buffer, arr);

	// TRABAJAMOS CON LA ETIQUETA ELSE, si no hay else, sigue siendo
	// necesario, asi es más fácil
	if (peek(tokens)->type == C_TOKEN_ELSE) {
		eat(tokens);
		if (parseStatement(tokens, buffer)!=0) {
			// ERROR
			return -1;
		}
	}


	// YA ACABAMOS CON ELSE, AHORA TOCA END
	memcpy(&arr[25], "END\n", 5);
	buffer->emitText(buffer, ".label "); // creamos la etiqueta
	buffer->emitText(buffer, arr);
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

static int parseDeclaration(TokenC *tokens, BufferI *buffer) {
	// la parte fácil es gestionar la declaración de variables
	// lo difícil son las funciones, omg
	TokenC *type = eat(tokens);
	TokenC *dat = eat(tokens);
	MCC_Var var;
	var.size = 4;

	memcpy(&var.name, dat->start, dat->len);
	memcpy(&var._type_, type->start, type->len);
	var.name[MCC_MAX_SYMBOL_NAME-1] = '\0';
	var._type_[MCC_MAX_SYMBOL_NAME-1] = '\0';


	if (CVarTable.current_scope < 1) {
		// en este caso es algo global, va por la tabla de globales
		// CGlobalTable, aqui podria ser una función
		if (peek(tokens)->type == C_TOKEN_LEFT_PAREN) {
			// es una función, hacemos nuestras cosas....
			// solo tenemos que devolver lo que haga el parser 
			buffer->emitText(buffer, ".label ");
			buffer->emitText(buffer, var.name);
			buffer->emitText(buffer, "\n");
			
			return parseFunctionDeclaration(tokens, buffer);
		} else {
			// variable normal
			buffer->emitText(buffer, ".label ");
			buffer->emitText(buffer, var.name);
			buffer->emitText(buffer, "\n");
			if (peek(tokens)->type == C_TOKEN_LEFT_BRACKET) {
				tok_index++;
				char *arr = (char*)malloc(64);
				memset(arr, '\0', 64);
				TokenC *tok = eat(tokens);
				if (eat(tokens)->type != C_TOKEN_RIGHT_BRACKET) {
					// ERROR!
					return -1;
				}
				if (tok->type != C_TOKEN_NUMBER) {
					// ERROR!
					return -1;
				}
				memcpy(arr, tok->start, tok->len);
				buffer->emitText(buffer, "%times ");
				buffer->emitText(buffer, arr);
				buffer->emitText(buffer, " 0d0\n");
				free(arr);
			} else {
				buffer->emitText(buffer, "dd 0x0\n")
			}
			mcc_add_g(var);
		}
	} else {
		// entonces no es global, estamos dentro de algun scope
		// aqui usamos la tabla normal, CVarTable, aqui solo es un var
		mcc_push_var(var);
	}
	tok_index--; // restauramos el indice
		     // por si las moscas
	return parseStatement(tokens, buffer); // esto permite manejar
					       // fácilmente cosas como
					       // int x = 5;
}

static int parseIdentifier(TokenC *tokens, BufferI *buffer) {
	TokenC *identifier = eat(tokens);
	
	if (peek(tokens)->type == C_TOKEN_ASSIGN) {
		MCC_Var *var;
		char *arr = (char*)malloc(identifier->len+1);
		memcpy(arr, identifier->start, identifier->len);
		arr[identifier->len] = '\0';

		var = mcc_find_var(arr); // ya tenemos la variable
		free(arr);
	
		arr = (char*)malloc(64);	

	
		// hacer una asignación, es decir, vamos a calcular la expresión
		// luego pegarla en la variable
		eat(tokens);
		if (0!=parseExpression(tokens, buffer)) {
			// oh, no, hubo un error
			// a tomar por saco
			return -1;
		}
		buffer->emitText(buffer, "push eax\n"); // empujar para no perder
		buffer->emitText(buffer, "mov ecx, ");
		buffer->emitText(buffer, int2char(arr, 64, var->offset));
		buffer->emitText(buffer, "mov eax, ebp\n");
		buffer->emitText(buffer, "sub eax, ecx\n");
		buffer->emitText(buffer, "pop ecx\n");
		buffer->emitText(buffer, "mov Meax, ecx\n");

		free(arr);
	} else if (peek(tokens)->type == C_TOKEN_COLON) { // definición label
		eat(tokens);
		char *arr = (char*)malloc(identifier->len+2);
		memcpy(arr, identifer->start, identifier->len);
		arr[identifier->len]='\n';
		arr[identifier->len+1]='\0';

		//emitimos la etiqueta
		buffer->emitText(buffer, ".label ");
		buffer->emitText(buffer, arr);

		free(arr);
	} else {
		// esta opción existe para permitir otras cosas, como llamadas
		// o simplemente postfix, o si cambio en el futuro algunos
		// operadores.
		tok_index--;
		return parseExpression(tokens, buffer);
	} 

	/*
	 * El soporte de diferentes asignaciones con multiplicación suma etc
	 * lo dejaremos para más adelante
	 *
	switch (peek(tokens)->type) {
		case C_TOKEN_ASSIGN:
		case C_TOKEN_SUB_ASSIGN:
		case C_TOKEN_ADD_ASSIGN:
		case C_TOKEN_MUL_ASSIGN:
		case C_TOKEN_DIV_ASSIGN:
		case C_TOKEN_MOD_ASSIGN:
		default: // ERROR
			return -1;
	}
	*/

	return 0;
}

static int parseFunctionDeclaration(TokenC *tokens, BufferI *buffer) {
	// aqui parsearemos funciones
	// se supone que el primer caracter es '('
	eat(tokens); 
	// ahora vienen argumentos
	// primero preparamos algo...
	CVarTable.current_scope = 1;
	CVarTable.stack_offset = 8;// es así porque - 0 esta ocupado
				   // por el anterior ebp
				   // y el - 4 esta ocupado por la dirección
				   // de return
	// así que me toca implementar algo para los argumentos
	// usaremos un while
	while (1) {
		if (peek(tokens)->type == C_TOKEN_RIGHT_PAREN) break;
		if (peek(tokens)->type == C_TOKEN_COMMA) eat(tokens);
		parseDeclaration(tokens, buffer);
		// ahora, modificamos sus valores
		MCC_Var *current = &CVarTable.table[CVarTable.symbol_count];
		// jijii, entonces, si el offset inicial era 0...
		current->offset = (size_t)(-current->offset);
	}

	// despues de eso reseteamos algunas cosas...
	CVarTable.current_scope = 0;
	CVarTable.stack_offset = 0; // importante
	if (0!=parseStatement(tokens, buffer)) {// aunque debería poner scope
					       // de todas formas
					       // lo dejo así
					       // porque esto puede facilitar
					       // crear funciones en asm
		// ERROR
		return -1;
	}
	buffer->emitText(buffer, "mov esp, ebp\n");
	buffer->emitText(buffer, "pop ebp\n");
	buffer->emitText(buffer, "ret\n");
	return 0;
}

static int parseReturn(TokenC *tokens, BufferI *buffer) {
	eat(tokens);
	if (parseExpression(tokens, buffer)!=0) {
		// ERROR!
		return -1;
	}
	buffer->emitText(buffer, "mov esp, ebp\n");
	buffer->emitText(buffer, "pop ebp\n");
	buffer->emitText(buffer, "ret\n");
	return 0;
}

static int parseWhile(TokenC *tokens, BufferI *buffer) {
	//
	char *arr = (char*)malloc(MCC_MAX_SYMBOL_NAME+1);
	memset(arr, '_', MCC_MAX_SYMBOL_NAME+1);
	arr[MCC_MAX_SYMBOL_NAME] = '\0';
	arr[0] = 'L';
	int2char(arr, 9, label++); // el 9 esta ahi porque permito hasta 6
	memcpy(&arr[10], name_base, 15);
	memcpy(&arr[25], "WHILE\n", 7);

	// creamos la etiqueta
	buffer->emitText(buffer, ".label ");
	buffer->emitText(buffer, arr);

	// parseamos la condición
	if (parseExpression(tokens, buffer)!=0) {
		// ERROR!
		return -1;
	}
	// empujamos eax para no perder su valor
	buffer->emitText("push eax\n");
	// preparamos etiqueta
	memcpy(&arr[25], "BODY\n", 6);
	
	buffer->emitText(buffer, "loax ");
	buffer->emitText(buffer, arr);
	buffer->emitText(buffer, "mov ebx, eax\n");
	// cargamos etiqueta de inicio
	// preparamos una etiqueta que usaremos ahora
	memcpy(&arr[25], "END\n", 5);
	buffer->emitText(buffer, "loax ");
	buffer->emitText(buffer, arr);
	// comparamos...
	// preparación.....
	buffer->emitText(buffer, "mov edx, 0d1\n");
	buffer->emitText(buffer, "pop ecx\n");
	// ahora si se compara
	buffer->emitText(buffer, "cmp edx, ecx\n");

	// ahora usamos mov condicional, que tortura
	buffer->emitText(buffer, "cmovz eax, ebx\n");
	// ahora saltamos
	buffer->emitText(buffer, "jmp eax\n");

	memcpy(&arr[25], "BODY\n", 6);
	buffer->emitText(buffer, ".label ");
	buffer->emitText(buffer, arr);

	// ahora ponemos el cuerpo del bucle
	if (parseStatement(tokens, buffer)!=0) {
		// ERROR!
		return -1;
	}
	// ahora toca lógica para volver al inicio, que es sencillo, es
	// un loax y un salto, pero nada más, tambien toca crear la etiqueta
	// del final
	memcpy(&arr[25], "WHILE\n", 7);
	buffer->emitText(buffer, "loax ");
	buffer->emitText(buffer, arr);

	// ahora, toca saltar
	buffer->emitText(buffer, "jmp eax\n");
	// ahora creamos etiqueta del final
	memcpy(&arr[25], "END\n", 5);
	buffer->emitText(buffer, ".label ");
	buffer->emitText(buffer, arr);
	
	free(arr);
	return 0;
}
// los goto's !!!
static int parseGoto(TokenC *tokens, BufferI *buffer) {
	// es bastante sencillo, primero hay que pillar el identifier
	// que es la etiqueta
	eat(tokens);
	TokenC *tok = eat(tokens); // ahora pillamos la etiqueta
	
	char *arr = (char*)malloc(tok->len+2);
	memcpy(arr, tok->start, tok->len);
	arr[tok->len] = '\n';
	arr[tok->len+1] = '\0';

	// ahora emitimos la carga de la etiqueta a eax
	buffer->emitText(buffer, "loax ");
	buffer->emitText(buffer, arr);

	// ahora emitimos el salto a eax
	buffer->emitText(buffer, "jmp eax\n");
	
	return 0;
}
