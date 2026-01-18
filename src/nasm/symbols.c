#include "../include/symbols.h"

MCC_g_Table CGlobalTable;
MCC_VarTable CVarTable;

bool isf=false;

void mcc_add_g(MCC_Var var) {
	if (CGlobalTable.symbol_count >= MCC_MAX_SYMBOLS) {
		// ERROR
		return;
	}
	var.global = true;
	memcpy(&CGlobalTable.table[CGlobalTable.symbol_count++], &var, sizeof(MCC_Var));
}

void mcc_push_var(MCC_Var var) {
	if (CVarTable.symbol_count >= MCC_MAX_SYMBOLS) {
		// ERROR
		return;
	}	
	var.nscope = CVarTable.current_scope;
	var.offset = CVarTable.current_stack_offset;
	var.global = false;
	CVarTable.current_stack_offset += var.size;
	memcpy(&CVarTable.table[CVarTable.symbol_count++], &var, sizeof(MCC_Var));
}

void mcc_clean_tab(void) {
	size_t removed = 0;
	size_t restore_offset = 0;
	MCC_Var *var;
	for (size_t i=CVarTable.symbol_count-1;i>0;i--) {
		var = &CVarTable.table[i];
		if (var->nscope > CVarTable.current_scope) {
			restore_offset += var->size;
			removed++;
		}
	}
	CVarTable.symbol_count -= removed;
	CVarTable.current_stack_offset -= restore_offset;
}

MCC_Var *mcc_find_var(char *name) {
	MCC_VarTable *vtable = &CVarTable;
	MCC_g_Table *gtable = &CGlobalTable;
	size_t len = MCC_MAX_SYMBOL_NAME;
	for (size_t i=vtable->symbol_count-1;i>0;i--) {
		if (strncmp(name, vtable->table[i].name, len)==0) {
			return &vtable->table[i];
		}
	}
	for (size_t i=gtable->symbol_count-1;i>0;i--) {
		if (strncmp(name, gtable->table[i].name, len)==0) {
			return &gtable->table[i];
		}
	}
	return (MCC_Var*)NULL;
}

int handle_identifier(TokenC *tokens, BufferI *buffer) {
	handle_address(tokens, buffer);
	buffer->emitText(buffer, "mov ecx, [eax]\n");
	TokenC *tok = eat(tokens);
	if (tok->type == C_TOKEN_POSTFIX_ADD) {
		buffer->emitText(buffer, "mov ebx, ecx\n");
		buffer->emitText(buffer, "add ebx, 1\n");
		buffer->emitText(buffer, "mov [eax], ebx\n");
	} else if (tok->type == C_TOKEN_POSTFIX_SUB) {
		buffer->emitText(buffer, "mov ebx, ecx\n");
		buffer->emitText(buffer, "sub ebx, 1\n");
		buffer->emitText(buffer, "mov [eax], ebx\n");
	} else if (tok->type == C_TOKEN_LEFT_PAREN && isf) { // isf dice si es función global
		tok_index--;
		return 0;
	} else {
		tok_index--;
	}
	buffer->emitText(buffer, "mov eax, ecx\n");
	return 0;
}


// lo siento operadores prefix
size_t handle_address(TokenC *tokens, BufferI *buffer) {
	MCC_Var *var;
	TokenC *tok = eat(tokens);
	
	// formateando el token
	char *tmp = (char*)malloc(tok->len+1);
	memcpy(tmp, tok->start, tok->len);
	tmp[tok->len] = '\0';

	var = mcc_find_var(tmp);
	

	if (var->is_ptr) {
		isf = true;
	} else {
		isf = false;
	}

	size_t size = var->size;	

	if (var->global) {
		buffer->emitText(buffer, "mov eax, ");
		char *tmp = (char*)malloc(MCC_MAX_SYMBOL_NAME+2);
		tmp[MCC_MAX_SYMBOL_NAME] = '\n';
		tmp[MCC_MAX_SYMBOL_NAME+1] = '\0';
		buffer->emitText(buffer, tmp);
	} else {
		char *arr = (char*)malloc(64);
		buffer->emitText(buffer, "mov ecx, [ebp - ");
		buffer->emitText(buffer, int2char(arr, 64, var->offset));
		buffer->emitText(buffer, "]\n");
		free(arr);
	}
	free(tmp);

	return size;
}
