#include "../include/symbols.h"

MCC_g_Table CGlobalTable;
MCC_VarTable CVarTable;

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
	CVarTable.current_stack_offset += var.size;
	var.nscope = CVarTable.current_scope;
	var.offset = CVarTable.current_stack_offset;
	var.global = false;
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
