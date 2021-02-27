#include "JST_string.h"

#include <stdlib.h>

static bool serialize_quoted_string( JST_String * string, const char * s ) {
   return   JST_String_append_char  ( string, '"' )
         && JST_String_append_string( string, s   )
         && JST_String_append_char  ( string, '"' );
}

static JST_Error serialize_element( const JST_Element * elt, JST_String * string, unsigned left_margin, unsigned indent );

static JST_Error serialize_object( const JST_Object * object, JST_String * string, unsigned left_margin, unsigned indent ) {
   for( JST_Pair * iter = object->first; iter; iter = iter->next ) {
      if(  ( ! JST_String_spaces       ( string, left_margin ))
         ||( ! serialize_quoted_string ( string, iter->name  ))
         ||( ! JST_String_append_string( string, ": "        )))
      {
         return JST_ERR_ERRNO;
      }
      JST_Error err = serialize_element( &(iter->element), string, left_margin, indent );
      if( err != JST_ERR_NONE ) {
         return err;
      }
      if( ! JST_String_append_string( string, iter->next ? ",\n" : "\n" )) {
         return JST_ERR_ERRNO;
      }
   }
   return JST_ERR_NONE;
}

static JST_Error serialize_array( const JST_Array * array, JST_String * string, unsigned left_margin, unsigned indent ) {
   for( unsigned i = 0; i < array->count; ++i ) {
      if( ! JST_String_spaces( string, left_margin )) {
         return JST_ERR_ERRNO;
      }
      JST_Error err = serialize_element( &(array->items[i]->element), string, left_margin, indent );
      if( err != JST_ERR_NONE ) {
         return err;
      }
      if( ! JST_String_append_string( string, ( i < array->count - 1 ) ? ",\n" : "\n" )) {
         return JST_ERR_ERRNO;
      }
   }
   return JST_ERR_NONE;
}

static JST_Error serialize_element( const JST_Element * elt, JST_String * string, unsigned left_margin, unsigned indent ) {
   static char buffer[100];
   JST_Error err = JST_ERR_NONE;
   switch( elt->type ) {
   case JST_OBJECT:
      if( ! JST_String_append_string( string, "{\n" )) {
         return JST_ERR_ERRNO;
      }
      err = serialize_object( &(elt->value.object), string, left_margin+indent, indent );
      if( err != JST_ERR_NONE ) {
         return err;
      }
      if(  ( ! JST_String_spaces( string, left_margin ))
         ||( ! JST_String_append_char( string, '}'    )))
      {
         return JST_ERR_ERRNO;
      }
      break;
   case JST_ARRAY:
      if( ! JST_String_append_string( string, "[\n" )) {
         return JST_ERR_ERRNO;
      }
      err = serialize_array( &(elt->value.array), string, left_margin+indent, indent );
      if( err != JST_ERR_NONE ) {
         return err;
      }
      if(  ( ! JST_String_spaces( string, left_margin ))
         ||( ! JST_String_append_char( string, ']' )))
      {
         return JST_ERR_ERRNO;
      }
      break;
   case JST_BOOLEAN:
      if( ! JST_String_append_string( string, elt->value.boolean ? "true" : "false" )) {
         return JST_ERR_ERRNO;
      }
      break;
   case JST_INTEGER:
      sprintf( buffer, "%ld", elt->value.integer );
      if( ! JST_String_append_string( string, buffer )) {
         return JST_ERR_ERRNO;
      }
      break;
   case JST_DOUBLE:
      sprintf( buffer, "%G", elt->value.dbl );
      if( ! JST_String_append_string( string, buffer )) {
         return JST_ERR_ERRNO;
      }
      break;
   case JST_STRING:
      if( ! serialize_quoted_string( string, elt->value.string )) {
         return JST_ERR_ERRNO;
      }
      break;
   case JST_NULL:
      if( ! JST_String_append_string( string, "null" )) {
         return JST_ERR_ERRNO;
      }
      break;
   default: return JST_ERR_NULL_TYPE;
   }
   return JST_ERR_NONE;
}

JST_Error JST_serialize( JST_Element * root, char ** dest, unsigned indent ) {
   if(( root == NULL )||( dest == NULL )) {
      return JST_ERR_NULL_ARGUMENT;
   }
   JST_String string = JST_String_Zero;
   if(( serialize_element( root, &string, 0, indent ) == JST_ERR_NONE )&& JST_String_append_char( &string, '\n' )) {
      string.buffer[string.length] = '\0';
      *dest = realloc( string.buffer, string.length + 1 ); // truncate, if necessary
      if( *dest == NULL ) {
         JST_String_delete( &string );
         return JST_ERR_ERRNO;
      }
      string.buffer = NULL;
   }
   JST_String_delete( &string );
   return JST_ERR_NONE;
}

static JST_Error serialize_element_compact( const JST_Element * elt, JST_String * string );

static JST_Error serialize_object_compact( const JST_Object * object, JST_String * string ) {
   for( JST_Pair * iter = object->first; iter; iter = iter->next ) {
      if(  ( ! serialize_quoted_string( string, iter->name ))
         ||( ! JST_String_append_char ( string, ':'        )))
      {
         return JST_ERR_ERRNO;
      }
      JST_Error err = serialize_element_compact( &(iter->element), string );
      if( err != JST_ERR_NONE ) {
         return err;
      }
      if( iter->next ) {
         if( ! JST_String_append_char( string, ',' )) {
            return JST_ERR_ERRNO;
         }
      }
   }
   return JST_ERR_NONE;
}

static JST_Error serialize_array_compact( const JST_Array * array, JST_String * string ) {
   for( unsigned i = 0; i < array->count; ++i ) {
      JST_Error err = serialize_element_compact( &(array->items[i]->element), string );
      if( err != JST_ERR_NONE ) {
         return err;
      }
      if( i < array->count - 1 ) {
         if( ! JST_String_append_char( string, ',' )) {
            return JST_ERR_ERRNO;
         }
      }
   }
   return JST_ERR_NONE;
}

static JST_Error serialize_element_compact( const JST_Element * elt, JST_String * string ) {
   static char buffer[100];
   JST_Error err = JST_ERR_NONE;
   switch( elt->type ) {
   case JST_OBJECT:
      if( ! JST_String_append_char( string, '{' )) {
         return JST_ERR_ERRNO;
      }
      err = serialize_object_compact( &(elt->value.object), string );
      if( err != JST_ERR_NONE ) {
         return err;
      }
      if( ! JST_String_append_char( string, '}' )) {
         return JST_ERR_ERRNO;
      }
      break;
   case JST_ARRAY:
      if( ! JST_String_append_char( string, '[' )) {
         return JST_ERR_ERRNO;
      }
      err = serialize_array_compact( &(elt->value.array), string );
      if( err != JST_ERR_NONE ) {
         return err;
      }
      if( ! JST_String_append_char( string, ']' )) {
         return JST_ERR_ERRNO;
      }
      break;
   case JST_BOOLEAN:
      if( ! JST_String_append_string( string, elt->value.boolean ? "true" : "false" )) {
         return JST_ERR_ERRNO;
      }
      break;
   case JST_INTEGER:
      sprintf( buffer, "%ld", elt->value.integer );
      if( ! JST_String_append_string( string, buffer )) {
         return JST_ERR_ERRNO;
      }
      break;
   case JST_DOUBLE:
      sprintf( buffer, "%G", elt->value.dbl );
      if( ! JST_String_append_string( string, buffer )) {
         return JST_ERR_ERRNO;
      }
      break;
   case JST_STRING:
      if( ! serialize_quoted_string( string, elt->value.string )) {
         return JST_ERR_ERRNO;
      }
      break;
   case JST_NULL:
      if( ! JST_String_append_string( string, "null" )) {
         return JST_ERR_ERRNO;
      }
      break;
   default: return JST_ERR_NULL_TYPE;
   }
   return JST_ERR_NONE;
}

JST_Error JST_serialize_compact( JST_Element * root, char ** dest ) {
   if(( root == NULL )||( dest == NULL )) {
      return JST_ERR_NULL_ARGUMENT;
   }
   JST_String string = JST_String_Zero;
   if(( serialize_element_compact( root, &string ) == JST_ERR_NONE )) {
      string.buffer[string.length] = '\0';
      *dest = realloc( string.buffer, string.length + 1 ); // truncate, if necessary
      if( *dest == NULL ) {
         JST_String_delete( &string );
         return JST_ERR_ERRNO;
      }
      string.buffer = NULL;
   }
   JST_String_delete( &string );
   return JST_ERR_NONE;
}
