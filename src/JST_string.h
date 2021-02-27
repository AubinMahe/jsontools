#pragma once

#include "jstools.h"

typedef struct {
   unsigned length;
   unsigned limit;
   char *   buffer;
} JST_String;

extern const JST_String JST_String_Zero;

bool JST_String_append_char( JST_String * str, char c );

bool JST_String_append_string( JST_String * string, const char * s );

bool JST_String_spaces( JST_String * str, unsigned count );

JST_Error JST_String_delete( JST_String * string );
