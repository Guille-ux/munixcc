#ifndef MUNIXCC_PARSER_H
#define MUNIXCC_PARSER_H

#include <stddef.h>
#include <stdint.h>
#include <stdbool.h>

#include "tokens.h"
#include "lexer.h"
#include "structs.h"
#include "utils.h"
#include "symbols.h"
#include "bufferi.h"

int mcc_parse_program(TokenC *tokens, BufferI *buffer);

#endif
