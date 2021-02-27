#include "jstools.h"

#include <stdio.h>

static JST_Error save_element( const JST_Element * elt, FILE * stream, unsigned left_margin, unsigned indent );

static JST_Error save_object( const JST_Object * object, FILE * stream, unsigned left_margin, unsigned indent ) {
   for( JST_Pair * iter = object->first; iter; iter = iter->next ) {
      fprintf( stream, "%*s\"%s\": ", left_margin, "", iter->name );
      save_element( &(iter->element), stream, left_margin, indent );
      fprintf( stream, "%s", ( iter->next == NULL ) ? "\n" : ",\n" );
   }
   return JST_ERR_NONE;
}

static JST_Error save_array( const JST_Array * array, FILE * stream, unsigned left_margin, unsigned indent ) {
   for( unsigned i = 0; i < array->count; ++i ) {
      fprintf( stream, "%*s", left_margin, "" );
      save_element( &(array->items[i]->element), stream, left_margin, indent );
      fprintf( stream, "%s", ( i < array->count - 1 ) ? ",\n" : "\n" );
   }
   return JST_ERR_NONE;
}

static JST_Error save_element( const JST_Element * elt, FILE * stream, unsigned left_margin, unsigned indent ) {
   switch( elt->type ) {
   case JST_OBJECT:
      fprintf( stream, "{\n" );
      save_object( &(elt->value.object), stream, left_margin+indent, indent );
      fprintf( stream, "%*s}", left_margin, "" );
      break;
   case JST_ARRAY:
      fprintf( stream, "[\n" );
      save_array ( &(elt->value.array ), stream, left_margin+indent, indent );
      fprintf( stream, "%*s]", left_margin, "" );
      break;
   case JST_BOOLEAN: fprintf( stream, "%s"    , elt->value.boolean ? "true" : "false" ); break;
   case JST_INTEGER: fprintf( stream, "%ld"   , elt->value.integer );                    break;
   case JST_DOUBLE : fprintf( stream, "%G"    , elt->value.dbl );                        break;
   case JST_STRING : fprintf( stream, "\"%s\"", elt->value.string );                     break;
   case JST_NULL   : fprintf( stream, "null" );                                          break;
   default: return JST_ERR_NULL_TYPE;
   }
   return JST_ERR_NONE;
}

JST_Error JST_save_to_stream( FILE * stream, const JST_Element * root, unsigned indent ) {
   if(( stream == NULL )||( root == NULL )) {
      return JST_ERR_NULL_ARGUMENT;
   }
   save_element( root, stream, 0, indent );
   fprintf( stream, "\n" );
   return fclose( stream ) ? JST_ERR_ERRNO : JST_ERR_NONE;
}

JST_Error JST_save_to_file( const char * path, const JST_Element * root, unsigned indent ) {
   if(( path == NULL )||( root == NULL )) {
      return JST_ERR_NULL_ARGUMENT;
   }
   FILE * stream = fopen( path, "wt" );
   if( stream == NULL ) {
      return JST_ERR_ERRNO;
   }
   return JST_save_to_stream( stream, root, indent );
}

static JST_Error save_element_compact( const JST_Element * elt, FILE * stream );

static JST_Error save_object_compact( const JST_Object * object, FILE * stream ) {
   for( JST_Pair * iter = object->first; iter; iter = iter->next ) {
      fprintf( stream, "\"%s\":", iter->name );
      save_element_compact( &(iter->element), stream );
      if( iter->next ) {
         fputc( ',', stream );
      }
   }
   return JST_ERR_NONE;
}

static JST_Error save_array_compact( const JST_Array * array, FILE * stream ) {
   for( unsigned i = 0; i < array->count; ++i ) {
      save_element_compact( &(array->items[i]->element), stream );
      if( i < array->count - 1 ) {
         fputc( ',', stream );
      }
   }
   return JST_ERR_NONE;
}

static JST_Error save_element_compact( const JST_Element * elt, FILE * stream ) {
   switch( elt->type ) {
   case JST_OBJECT:
      fprintf( stream, "{" );
      save_object_compact( &(elt->value.object), stream );
      fprintf( stream, "}" );
      break;
   case JST_ARRAY:
      fprintf( stream, "[" );
      save_array_compact( &(elt->value.array), stream );
      fprintf( stream, "]" );
      break;
   case JST_BOOLEAN: fprintf( stream, "%s"    , elt->value.boolean ? "true" : "false" ); break;
   case JST_INTEGER: fprintf( stream, "%ld"   , elt->value.integer );                    break;
   case JST_DOUBLE : fprintf( stream, "%G"    , elt->value.dbl );                        break;
   case JST_STRING : fprintf( stream, "\"%s\"", elt->value.string );                     break;
   case JST_NULL   : fprintf( stream, "null" );                                          break;
   default: return JST_ERR_NULL_TYPE;
   }
   return JST_ERR_NONE;
}

JST_Error JST_save_to_stream_compact( FILE * stream, const JST_Element * root ) {
   if(( stream == NULL )||( root == NULL )) {
      return JST_ERR_NULL_ARGUMENT;
   }
   save_element_compact( root, stream );
   return fclose( stream ) ? JST_ERR_ERRNO : JST_ERR_NONE;
}

JST_Error JST_save_to_file_compact( const char * path, const JST_Element * root ) {
   if(( path == NULL )||( root == NULL )) {
      return JST_ERR_NULL_ARGUMENT;
   }
   FILE * stream = fopen( path, "wt" );
   if( stream == NULL ) {
      return JST_ERR_ERRNO;
   }
   return JST_save_to_stream_compact( stream, root );
}
