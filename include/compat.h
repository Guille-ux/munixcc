#ifndef MINIASM_COMPAT_H
#define MINIASM_COMPAT_H

/*
 * Esto debe incluir las declaraciones de:
 *
 * Malloc y Free
 * memcpy, memset, strcmp, strncmp y strlen
 *
 * (NOTA: No uso nada de FOPEN etc, por compatibilidad, a cambio
 *  el ensamblador devolvera los BLOBS binarios directamente)
 *  Lo saque del código de mi ensamblador como comprenderas
 *
 */

#include <stdlib.h> // para malloc y free
#include <string.h> // para memcpy, memset, strcmp, strncmp y strlen
#include <ctype.h> // necesario

#endif
