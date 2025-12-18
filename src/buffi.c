#include "../include/buffi.h"

// Implementación para uno dinámico

void munixcc_initDynamicBuffer(BufferI *buffer) {
	buffer->buffer = (char*)malloc(MUNIXCC_BUFFER_START_CAP);
	buffer->size=0;
	buffer->cap=MUNIXCC_BUFFER_START_CAP;
	buffer->freeBuff=MCC_freeDynBuff;
	buffer->emitText=MCC_emitDynText;
}

void MCC_freeDynBuff(BufferI *buffer) {
	free(buffer->buffer);
	buffer->size=0;
	buffer->cap=0;
}

void MCC_emitDynText(BufferI *buffer, char *text) {
	size_t len = strlen(text);
	int correction = buffer->cap - (buffer->size + len);
	if (correction < 1) {
		char *newBuff = (char*)malloc(buffer->cap*MCC_BUFF_GROWTH);
		buffer->cap = buffer->cap * MCC_BUFF_GROWTH;
		memcpy(newBuff, buffer->buffer, buffer->size);
		free(buffer->buffer);
		buffer->buffer = newBuff;
	}
	memcpy((char*)(((size_t)buffer->buffer) + buffer->size), text, len);
	buffer->size += len;
}
