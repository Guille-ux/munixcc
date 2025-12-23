#include "../include/compat.h"
#include "../include/structs.h"

MCCStructTable CStructTable = {.current_struct=0};

void mcc_CreateNewStruct(const char *name, ...) {
	if (CStructTable.current_struct >= MCC_STRUCTS_TABLE_SIZE) return;
	va_list args;
	va_start(args, name);

	MCCStructDef *newStruct=&CStructTable.structs[CStructTable.current_struct++];
	
	size_t len = strlen(name);
	len = (len < MCC_STRUCT_MAX_NAME) ? len : MCC_STRUCT_MAX_NAME;
	memcpy(newStruct->name, name, len);
	MCCStructMember *cmember;
	while (1) {
		cmember = va_arg(args, MCCStructMember*);
		if (cmember == NULL ||
		    newStruct->num_members > MCC_STRUCT_MAX_MEMBERS)
		{ // llego a su final
			va_end(args);
			return;
		}
		cmember->offset = newStruct->size;	
		memcpy(&newStruct->members[newStruct->num_members], cmember, sizeof(MCCStructMember));
		newStruct->size += cmember->size;
		newStruct->num_members++;
	}
}

MCCStructDef *mcc_FindStruct(const char *name) {
	size_t len = MCC_STRUCT_MAX_NAME;
	MCCStructDef *def=NULL;
	MCCStructDef *defs=CStructTable.structs;
	for (size_t i = 0;i<CStructTable.current_struct;i++) {
		if (strncmp(name, defs[i].name, len)==0) {
			def=&defs[i];
			break;
		}
	}
	return def;
}

MCCStructMember *mcc_FindMember(const char *name, MCCStructDef *truct) {
	size_t len = MCC_STRUCT_MAX_NAME;
	MCCStructMember *member;
	for (size_t i=0;i<MCC_STRUCT_MAX_MEMBERS;i++) {
		if (strncmp(name, truct->members[i].name, len)==0) {
			member = &truct->members[i];
			break;
		}
	}
	return member;
}
