#include "../include/compat.h"
#include "../include/parser.h"

size_t label = 0;
size_t tok_index = 0;
// usaremos etiquetas con hasta 4 dígitos por función!

// definiciones de funciones internas
static int parseStatement(TokenC *tokens, BufferI *buffer);
static int parsePrototype(TokenC *tokens, BufferI *buffer);

// definiciones para parseo de expresiones
static int parseExpression(TokenC *tokens, BufferI *buffer);
static int parsePrimary(TokenC *tokens, BufferI *buffer);
static int parseUnary(TokenC *tokens, BufferI *buffer);
static int parseLogical(TokenC *tokens, BufferI *buffer);
static int parseAdditive(TokenC *tokens, BufferI *buffer);
static int parseMultiplicative(TokenC *tokens, BufferI *buffer);

// cosas de utilidad
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

// CONVENCIÓN DE OPERANDOS
// EL IZQUIERDO VA EN ECX
// Y EL DERECHO EN EAX
// USAMOS EL STACK (push, pop etc)
// EL RESULTADO SE GUARDA EN EAX
// en expresiones
// empujamos el resultado de eax a la pila, calculamos el otro resultado y
// bajamos el valor de la pila a ecx


// TODO: AÑADIR CÓDIGO DE parsePrototype y acabar parseStatement

static int parseStatement(TokenC *tokens, BufferI buffer) {
	switch (eat(tokens)->type) {
		case C_TOKEN_EXTERN: return parsePrototype(tokens, buffer);
		default: break; // BRUH, NO SE NADA
	}
	return -1; // no deberia haber llegado hasta aqui
}

// las 3 funciones para expresiones aritméticas

static int parseExpression(TokenC *tokens, BufferI *buffer) {
	return parseLogical(TokenC *tokens, BufferI *buffer);
}
static int parsePrimary(TokenC *tokens, BufferI *buffer) {

}
static int parseUnary(TokenC *tokens, BufferI *buffer) {

}

// TODO: COMPROBAR QUE OR Y AND FUNCIONAN ASÍ
static int parseLogical(TokenC *tokens, BufferI *buffer) {
	parseAdditive(tokens, buffer);
	while  (peek(tokens)->type == C_TOKEN_AND ||
		peek(tokens)->type == C_TOKEN_OR ||
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
				// comparamos con and
				buffer->emitText(buffer, "and ecx, eax\n");
				// movemos 1 a eax en caso de que ambos true
				buffer->emitText(buffer, "cmovz eax, 0d1\n");
				break;
			case C_TOKEN_OR:
				// comparamos con or
				buffer->emitText(buffer, "or ecx, eax\n");
				// movemos 1 a eax si alguno era true
				buffer->emitText(buffer, "cmovz eax, 0d1\n");
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
				// si alguno era movemos 1 a eax
				buffer->emitText(buffer, "cmovz eax, 0d1\n");
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
				// si alguno era movemos 1 a eax
				buffer->emitText(buffer, "cmovz eax, 0d1\n");
				break;
		}

		
	}
	return 0;
}
static int parseAdditive(TokenC *tokens, BufferI *buffer) {

}
static int parseMultiplicative(TokenC *tokens, BufferI *buffer) {

}

