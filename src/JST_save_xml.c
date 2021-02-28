#include "jstools.h"
#include "JST_string.h"

#include <stdio.h>
#include <stdlib.h>
#include <string.h>

static const char * SCHEMA =
   " xmlns:xsi=\"http://www.w3.org/2001/XMLSchema-instance\""
   " xsi:noNamespaceSchemaLocation=\"../json-xml.xsd\"";

static JST_Error save_xml_element( const JST_Element * elt, FILE * stream, unsigned left_margin, unsigned indent );

static JST_Error save_xml_object( const JST_Object * object, FILE * stream, unsigned left_margin, unsigned indent ) {
   for( JST_Pair * iter = object->first; iter; iter = iter->next ) {
      fprintf( stream, "%*s<property name=\"%s\">\n", left_margin, "", iter->name );
      fprintf( stream, "%*s", left_margin+indent, "" );
      save_xml_element( &(iter->element), stream, left_margin+indent, indent );
      fprintf( stream, "%*s</property>\n", left_margin, "" );
   }
   return JST_ERR_NONE;
}

static JST_Error save_xml_array( const JST_Array * array, FILE * stream, unsigned left_margin, unsigned indent ) {
   for( unsigned i = 0; i < array->count; ++i ) {
      fprintf( stream, "%*s", left_margin, "" );
      save_xml_element( &(array->items[i]->element), stream, left_margin, indent );
   }
   return JST_ERR_NONE;
}

static JST_String xml = { .limit = 0, .length = 0, .buffer = NULL };

static const char * xml_encode( const char * text ) {
   if( xml.limit > 0 ) {
      xml.length    = 0;
      xml.buffer[0] = '\0';
   }
   char c = text[0];
   for( unsigned i = 0; c; c = text[++i] ) {
      switch( c ) {
      case '<' : JST_String_append_string( &xml, "&lt;"   ); break;
      case '&' : JST_String_append_string( &xml, "&amp;"  ); break;
      case '>' : JST_String_append_string( &xml, "&gt;"   ); break;
      case '"' : JST_String_append_string( &xml, "&quot;" ); break;
      case '\'': JST_String_append_string( &xml, "&apos;" ); break;
      case '\\':
         switch( text[i+1] ) {
         case 'b': JST_String_append_string( &xml, "\b" ); ++i; break;
         case 'n': JST_String_append_string( &xml, "\n" ); ++i; break;
         case 'r': JST_String_append_string( &xml, "\r" ); ++i; break;
         case 't': JST_String_append_string( &xml, "\t" ); ++i; break;
         default : JST_String_append_string( &xml, "\\" ); break;
         }
         break;
      default  : JST_String_append_char( &xml, c );          break;
      }
   }
   xml.buffer[xml.length] = '\0';
   return xml.buffer;
}

static JST_Error save_xml_element( const JST_Element * elt, FILE * stream, unsigned left_margin, unsigned indent ) {
   switch( elt->type ) {
   case JST_OBJECT:
      fprintf( stream, "<object%s>\n", elt->parent ? "" : SCHEMA );
      save_xml_object( &(elt->value.object), stream, left_margin+indent, indent );
      fprintf( stream, "%*s</object>", left_margin, "" );
      break;
   case JST_ARRAY:
      fprintf( stream, "<array%s>\n", elt->parent ? "" : SCHEMA );
      save_xml_array ( &(elt->value.array ), stream, left_margin+indent, indent );
      fprintf( stream, "%*s</array>", left_margin, "" );
      break;
   case JST_BOOLEAN:
      fprintf( stream, "<boolean%s value=\"%s\" />" , elt->parent ? "" : SCHEMA, elt->value.boolean ? "true" : "false" );
      break;
   case JST_INTEGER:
      fprintf( stream, "<integer%s value=\"%ld\" />", elt->parent ? "" : SCHEMA, elt->value.integer );
      break;
   case JST_DOUBLE:
      fprintf( stream, "<double%s value=\"%G\" />"  , elt->parent ? "" : SCHEMA, elt->value.dbl );
      break;
   case JST_STRING:
      fprintf( stream, "<string%s>%s</string>", elt->parent ? "" : SCHEMA, xml_encode( elt->value.string ));
      break;
   case JST_NULL:
      fprintf( stream, "<null%s />", elt->parent ? "" : SCHEMA );
      break;
   default: return JST_ERR_NULL_TYPE;
   }
   fprintf( stream, "\n" );
   return JST_ERR_NONE;
}

JST_Error JST_save_to_xml_stream( FILE * stream, const JST_Element * root, unsigned indent ) {
   if(( stream == NULL )||( root == NULL )) {
      return JST_ERR_NULL_ARGUMENT;
   }
   fprintf( stream, "<?xml version=\"1.0\" encoding=\"UTF-8\"?>\n" );
   save_xml_element( root, stream, 0, indent );
   JST_String_delete( &xml );
   return fclose( stream ) ? JST_ERR_ERRNO : JST_ERR_NONE;
}

JST_Error JST_save_to_xml_file( const char * path, const JST_Element * root, unsigned indent ) {
   if(( path == NULL )||( root == NULL )) {
      return JST_ERR_NULL_ARGUMENT;
   }
   FILE * stream = fopen( path, "wt" );
   if( stream == NULL ) {
      return JST_ERR_ERRNO;
   }
   return JST_save_to_xml_stream( stream, root, indent );
}

static JST_Error serialize_element_xml( const JST_Element * elt, JST_String * string, unsigned left_margin, unsigned indent );

static JST_Error serialize_object_xml( const JST_Object * object, JST_String * string, unsigned left_margin, unsigned indent ) {
   for( JST_Pair * iter = object->first; iter; iter = iter->next ) {
      if(  ( ! JST_String_spaces( string, left_margin ))
         ||( ! JST_String_append_string( string, "<property name=\"" ))
         ||( ! JST_String_append_string( string, iter->name ))
         ||( ! JST_String_append_string( string, "\">\n" )))
      {
         return JST_ERR_ERRNO;
      }
      JST_Error err = serialize_element_xml( &(iter->element), string, left_margin+indent, indent );
      if( err != JST_ERR_NONE ) {
         return err;
      }
      if(  ( ! JST_String_spaces( string, left_margin ))
         ||( ! JST_String_append_string( string, "</property>\n" )))
      {
         return JST_ERR_ERRNO;
      }
   }
   return JST_ERR_NONE;
}

static JST_Error serialize_array_xml( const JST_Array * array, JST_String * string, unsigned left_margin, unsigned indent ) {
   for( unsigned i = 0; i < array->count; ++i ) {
      JST_Error err = serialize_element_xml( &(array->items[i]->element), string, left_margin, indent );
      if( err != JST_ERR_NONE ) {
         return err;
      }
   }
   return JST_ERR_NONE;
}

static JST_Error serialize_element_xml( const JST_Element * elt, JST_String * string, unsigned left_margin, unsigned indent ) {
   JST_String_spaces( string, left_margin );
   char buffer[80];
   switch( elt->type ) {
   case JST_OBJECT:
      JST_String_append_string( string, "<object" );
      if( elt->parent == NULL ) {
         JST_String_append_string( string, SCHEMA );
      }
      JST_String_append_string( string, ">\n" );
      serialize_object_xml( &(elt->value.object), string, left_margin+indent, indent );
      JST_String_spaces( string, left_margin );
      JST_String_append_string( string, "</object>" );
      break;
   case JST_ARRAY:
      JST_String_append_string( string, "<array" );
      if( elt->parent == NULL ) {
         JST_String_append_string( string, SCHEMA );
      }
      JST_String_append_string( string, ">\n" );
      serialize_array_xml( &(elt->value.array ), string, left_margin+indent, indent );
      JST_String_spaces( string, left_margin );
      JST_String_append_string( string, "</array>" );
      break;
   case JST_BOOLEAN:
      JST_String_append_string( string, "<boolean" );
      if( elt->parent == NULL ) {
         JST_String_append_string( string, SCHEMA );
      }
      JST_String_append_string( string, " value=\"" );
      JST_String_append_string( string, elt->value.boolean ? "true" : "false" );
      JST_String_append_string( string, "\" />" );
      break;
   case JST_INTEGER:
      JST_String_append_string( string, "<integer" );
      if( elt->parent == NULL ) {
         JST_String_append_string( string, SCHEMA );
      }
      sprintf( buffer, " value=\"%ld\" />", elt->value.integer );
      JST_String_append_string( string, buffer );
      break;
   case JST_DOUBLE:
      JST_String_append_string( string, "<double" );
      if( elt->parent == NULL ) {
         JST_String_append_string( string, SCHEMA );
      }
      sprintf( buffer, " value=\"%G\" />", elt->value.dbl );
      JST_String_append_string( string, buffer );
      break;
   case JST_STRING:
      JST_String_append_string( string, "<string" );
      if( elt->parent == NULL ) {
         JST_String_append_string( string, SCHEMA );
      }
      JST_String_append_char( string, '>' );
      JST_String_append_string( string, xml_encode( elt->value.string ));
      JST_String_append_string( string, "</string>" );
      break;
   case JST_NULL:
      JST_String_append_string( string, "<null" );
      if( elt->parent == NULL ) {
         JST_String_append_string( string, SCHEMA );
      }
      JST_String_append_string( string, " />" );
      break;
   default: return JST_ERR_NULL_TYPE;
   }
   JST_String_append_char( string, '\n' );
   return JST_ERR_NONE;
}

JST_Error JST_serialize_xml( JST_Element * root, char ** dest, unsigned indent ) {
   if(( root == NULL )||( dest == NULL )) {
      return JST_ERR_NULL_ARGUMENT;
   }
   JST_String string = JST_String_Zero;
   JST_String_append_string( &string, "<?xml version=\"1.0\" encoding=\"UTF-8\"?>\n" );
   if(( serialize_element_xml( root, &string, 0, indent ) == JST_ERR_NONE )) {
      *dest = realloc( string.buffer, string.length + 1 ); // truncate, if necessary
      (*dest)[string.length] = '\0';
      if( *dest == NULL ) {
         JST_String_delete( &string );
         JST_String_delete( &xml );
         return JST_ERR_ERRNO;
      }
      string.buffer = NULL;
   }
   JST_String_delete( &string );
   JST_String_delete( &xml );
   return JST_ERR_NONE;
}
