#include "jstools.h"
#include "JST_string.h"

#include <stdio.h>
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
