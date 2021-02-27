#include "JST_string.h"

#include <stdlib.h>
#include <string.h>

const JST_String JST_String_Zero = { .length = 0, .limit = 0, .buffer = NULL };

bool JST_String_append_char( JST_String * string, char c ) {
   if( string == NULL ) {
      return false;
   }
   if( string->length >= string->limit ) {
      if( string->limit == 0 ) {
         string->limit = 40;
      }
      string->limit *= 2;
      string->buffer = realloc( string->buffer, string->limit );
      if( string->buffer == NULL ) {
         return false;
      }
   }
   string->buffer[string->length++] = c;
   return true;
}

bool JST_String_append_string( JST_String * string, const char * s ) {
   if(( string == NULL )||( s == NULL )) {
      return false;
   }
   unsigned lb = string->length;
   size_t count = strlen( s );
   for( unsigned i = 0; ( i < count )&& JST_String_append_char( string, s[i] ); ++i );
   return string->length == lb + count;
}

bool JST_String_spaces( JST_String * string, unsigned count ) {
   if( string == NULL ) {
      return false;
   }
   unsigned lb = string->length;
   for( unsigned i = 0; ( i < count )&& JST_String_append_char( string, ' ' ); ++i );
   return string->length == lb + count;
}

JST_Error JST_String_delete( JST_String * string ) {
   if( string == NULL ) {
      return JST_ERR_NULL_ARGUMENT;
   }
   if( string->buffer ) {
      free( string->buffer );
   }
   string->buffer = NULL;
   string->length = 0;
   string->limit  = 0;
   return JST_ERR_NONE;
}
