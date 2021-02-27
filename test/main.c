#include "jstools.h"

#include <errno.h>
#include <stdio.h>
#include <string.h>
#include <stdlib.h>
#include <inttypes.h>
#include <math.h>

static const char * SYNTAX_ERROR_STRING_NOT_ENDED               = "Syntax error: string isn't terminated";
static const char * SYNTAX_ERROR_SEPARATOR_EXPECTED_AFTER_TRUE  = "Syntax error: separator expected after 'true' token";
static const char * SYNTAX_ERROR_SEPARATOR_EXPECTED_AFTER_FALSE = "Syntax error: separator expected after 'false' token";
static const char * SYNTAX_ERROR_SEPARATOR_EXPECTED_AFTER_NULL  = "Syntax error: separator expected after 'null' token";
static const char * SYNTAX_ERROR_TRUE_EXPECTED                  = "Syntax error: 'true' token expected";
static const char * SYNTAX_ERROR_FALSE_EXPECTED                 = "Syntax error: 'false' token expected";
static const char * SYNTAX_ERROR_NULL_EXPECTED                  = "Syntax error: 'null' token expected";
static const char * SYNTAX_ERROR_NUMBER_EXPECTED                = "Syntax error: <number> expected";
static const char * SYNTAX_ERROR_COLON_EXPECTED                 = "Syntax error: <:> expected";
static const char * SYNTAX_ERROR_UNEXPECTED_TOKEN               = "Syntax error: unexpected token";

static bool dump( const char * name, unsigned index, bool pre, const JST_Element * element, void * user_context ) {
   int * indent = (int *)user_context;
   if( pre ) {
      if( index != -1U ) {
         fprintf( stderr, "%*s%d ==> ", *indent, "", index );
      }
      else if( name ) {
         fprintf( stderr, "%*s'%s' : ", *indent, "", name );
      }
      else {
         fprintf( stderr, "ROOT ==> " );
      }
      *indent += 3;
      switch( element->type ) {
      case JST_ARRAY  : fprintf( stderr, "(array @%p) [\n", (const void *)&(element->value.array)); break;
      case JST_OBJECT : fprintf( stderr, "(object @%p) {\n", (const void *)&(element->value.object)); break;
      case JST_BOOLEAN: fprintf( stderr, "%s\n"  , element->value.boolean ? "true" : "false" ); break;
      case JST_INTEGER: fprintf( stderr, "%ld\n" , element->value.integer ); break;
      case JST_DOUBLE : fprintf( stderr, "%G\n"  , element->value.dbl ); break;
      case JST_STRING : fprintf( stderr, "'%s'\n", element->value.string ); break;
      case JST_NULL   : fprintf( stderr, "null\n" ); break;
      default: fprintf( stderr, "Trou dans la raquette : element->type = %d\n", element->type ); break;
      }
   }
   else {
      *indent -= 3;
      switch( element->type ) {
      case JST_ARRAY  : fprintf( stderr, "%*s]\n", *indent, "" ); break;
      case JST_OBJECT : fprintf( stderr, "%*s}\n", *indent, "" ); break;
      default: break;
      }
   }
   return true;
}

static bool test_get( JST_Element * root ) {
   JST_Element * elt = NULL;
   return( JST_get( "repository.examples[2].subs[3].value", root, &elt ) == JST_ERR_NONE )
      && ( JST_get( "repository.examples[2].subs[3]"      , root, &elt ) == JST_ERR_NONE )
      && ( elt->type == JST_OBJECT )
      && ( JST_get( "repository.examples[2].subs"         , root, &elt ) == JST_ERR_NONE )
      && ( elt->type == JST_ARRAY )
      && ( elt->value.array.count == 6 );
}

static bool check_sequence_of_pairs( JST_Element * elt, const char * names[], unsigned names_count ) {
   unsigned check = 0U;
   for( JST_Pair * iter = elt->value.object.first; iter; iter = iter->next ) {
      if( strcmp( iter->name, names[check++] )) {
         return false;
      }
      if( check > names_count ) {
         return false;
      }
   }
   return true;
}

static bool test_add_property( JST_Element * root ) {
   static const char * names[] = {
      "integer",
      "double",
      "added_on_the_fly",
      "boolean",
      "subs"
   };
   static const unsigned names_count = sizeof( names ) / sizeof( names[0]);
   JST_Element * elt = NULL;
   const JST_Element value = {.type = JST_ARRAY, { .array = { .items = NULL, .count = 0, .parent = NULL }}};
   JST_Element * new_array = NULL;
   return( JST_get( "repository.examples[2]", root, &elt ) == JST_ERR_NONE )
      && ( elt->type == JST_OBJECT )
      && ( JST_add_property( &(elt->value.object), 2, "added_on_the_fly", &value ) == JST_ERR_NONE )
      && ( JST_get( "repository.examples[2].added_on_the_fly", root, &new_array ) == JST_ERR_NONE )
      && check_sequence_of_pairs( elt, names, names_count )
      && ( new_array->type == JST_ARRAY )
      && ( new_array->value.array.count == 0 )
      && ( new_array->value.array.parent == new_array );
}

static bool dbl_equals( double d1, double d2 ) {
   return fabs( d1 - d2 ) < 0.00001;
}

static bool test_add_item( JST_Element * root ) {
   const JST_Element dblVal5pi = { .type = JST_DOUBLE, .value = { .dbl = 5.0 * 3.141592653 }};
   const JST_Element dblVal6pi = { .type = JST_DOUBLE, .value = { .dbl = 6.0 * 3.141592653 }};
   JST_Element * array = NULL;
   return( JST_get( "repository.examples[2].added_on_the_fly", root, &array ) == JST_ERR_NONE )
      && ( array->type == JST_ARRAY )
      && ( JST_add_item( &(array->value.array), -1U, &dblVal6pi ) == JST_ERR_NONE )
      && ( JST_add_item( &(array->value.array),  0U, &dblVal5pi ) == JST_ERR_NONE )
      && ( JST_DOUBLE == array->value.array.items[0]->element.type )
      && ( dbl_equals(   array->value.array.items[0]->element.value.dbl, dblVal5pi.value.dbl ))
      && ( JST_DOUBLE == array->value.array.items[1]->element.type )
      && ( dbl_equals(   array->value.array.items[1]->element.value.dbl, dblVal6pi.value.dbl ));
}

static bool test_replace_item( JST_Element * root ) {
   const JST_Element dblVal6pi = { .type = JST_DOUBLE, .value = { .dbl = 6.0 * 3.141592653 }};
   const JST_Element dblVal1pi = { .type = JST_DOUBLE, .value = { .dbl = 1.0 * 3.141592653 }};
   JST_Element * elt   = NULL;
   JST_Element * value = NULL;
   return( JST_get( "repository.examples[2].added_on_the_fly", root, &elt ) == JST_ERR_NONE )
      && ( elt->type == JST_ARRAY )
      && ( JST_replace_item( &(elt->value.array), 0, &dblVal1pi ) == JST_ERR_NONE )
      && ( JST_get( "repository.examples[2].added_on_the_fly[0]", root, &value ) == JST_ERR_NONE )
      && ( value->type == JST_DOUBLE )
      && dbl_equals( value->value.dbl, dblVal1pi.value.dbl )
      && ( JST_get( "repository.examples[2].added_on_the_fly[1]", root, &value ) == JST_ERR_NONE )
      && ( value->type == JST_DOUBLE )
      && dbl_equals( value->value.dbl, dblVal6pi.value.dbl );
}

static bool test_remove_item( JST_Element * root ) {
   JST_Element * elt = NULL;
   return( JST_get( "repository.examples[2].added_on_the_fly", root, &elt ) == JST_ERR_NONE )
      && ( elt->type == JST_ARRAY )
      && ( JST_remove_item( &(elt->value.array), 0 ) == JST_ERR_NONE );
}

static bool test_remove_property( JST_Element * root ) {
   static const char * names[] = {
      "integer",
      "double",
      "boolean",
      "subs"
   };
   static const unsigned names_count = sizeof( names ) / sizeof( names[0]);
   JST_Element * elt = NULL;
   return( JST_get( "repository.examples[2]", root, &elt ) == JST_ERR_NONE )
      && ( elt->type == JST_OBJECT )
      && ( JST_remove_property( &(elt->value.object), "added_on_the_fly" ) == JST_ERR_NONE )
      && ( elt->value.object.count == 4 )
      && check_sequence_of_pairs( elt, names, names_count );
}

static bool test_parents( JST_Element * root ) {
   JST_Element * elt = NULL;
   if( JST_get( "repository.examples[2].subs[0].value", root, &elt ) != JST_ERR_NONE ) {
      return false;
   }
   if( elt->type != JST_INTEGER ) {
      return false;
   }
   if( elt->parent == NULL ) {
      return false;
   }
   if( ! elt->parent_is_pair ) {
      return false;
   }
   JST_Pair * pair = (JST_Pair *)(elt->parent);
   if( pair == NULL ) {
      return false;
   }
   if( strcmp( pair->name, "value" )) {
      return false;
   }
   JST_Object * object = pair->parent;
   if( object == NULL ) {
      return false;
   }
   elt = object->parent;
   if( elt == NULL ) {
      return false;
   }
   if( elt->parent_is_pair ) {
      return false;
   }
   const JST_ArrayItem * item = (const JST_ArrayItem *)elt->parent;
   if( item == NULL ) {
      return false;
   }
   const JST_Array * array = item->parent;
   if( array->items[0] != item ) {
      return false;
   }
   elt = array->parent;
   if( elt == NULL ) {
      return false;
   }
   if( ! elt->parent_is_pair ) {
      return false;
   }
   pair = (JST_Pair *)(elt->parent);
   if( pair == NULL ) {
      return false;
   }
   if( strcmp( pair->name, "subs" )) {
      return false;
   }
   object = pair->parent;
   if( object == NULL ) {
      return false;
   }
   elt = object->parent;
   if( elt == NULL ) {
      return false;
   }
   if( elt->parent_is_pair ) {
      return false;
   }
   item = (const JST_ArrayItem *)elt->parent;
   if( item == NULL ) {
      return false;
   }
   array = item->parent;
   if( array->items[2] != item ) {
      return false;
   }
   elt = array->parent;
   if( elt == NULL ) {
      return false;
   }
   if( ! elt->parent_is_pair ) {
      return false;
   }
   pair = (JST_Pair *)(elt->parent);
   if( pair == NULL ) {
      return false;
   }
   if( strcmp( pair->name, "examples" )) {
      return false;
   }
   object = pair->parent;
   if( object == NULL ) {
      return false;
   }
   elt = object->parent;
   if( elt == NULL ) {
      return false;
   }
   if( ! elt->parent_is_pair ) {
      return false;
   }
   pair = (JST_Pair *)(elt->parent);
   if( pair == NULL ) {
      return false;
   }
   if( strcmp( pair->name, "repository" )) {
      return false;
   }
   object = pair->parent;
   if( object == NULL ) {
      return false;
   }
   elt = object->parent;
   return elt == root;
}

static bool test_save( JST_Element * root, const char * ref ) {
   char cmp_command[240];
   snprintf( cmp_command, sizeof( cmp_command ), "cmp %s /tmp/pass4-save.json", ref );
   return( JST_save_to_file( "/tmp/pass4-save.json", root, 2 ) == JST_ERR_NONE )
      && ( system( cmp_command ) == EXIT_SUCCESS );
}

static bool test_serialize( JST_Element * root, const char * ref ) {
   char cmp_command[240];
   snprintf( cmp_command, sizeof( cmp_command ), "cmp %s /tmp/pass4-serialize.json", ref );
   char * buffer = NULL;
   if( JST_serialize( root, &buffer, 2 ) != JST_ERR_NONE ) {
      return false;
   }
   FILE * tmp = fopen( "/tmp/pass4-serialize.json", "wt" );
   fputs( buffer, tmp );
   fclose ( tmp );
   free( buffer );
   return system( cmp_command ) == EXIT_SUCCESS;
}

static bool test_save_compact( JST_Element * root, const char * ref ) {
   char cmp_command[240];
   snprintf( cmp_command, sizeof( cmp_command ), "cmp %s /tmp/pass5-save-compact.json", ref );
   return( JST_save_to_file_compact( "/tmp/pass5-save-compact.json", root ) == JST_ERR_NONE )
      && ( system( cmp_command ) == EXIT_SUCCESS );
}

static bool test_serialize_compact( JST_Element * root, const char * ref ) {
   char cmp_command[240];
   snprintf( cmp_command, sizeof( cmp_command ), "cmp %s /tmp/pass5-serialize-compact.json", ref );
   char * buffer = NULL;
   if( JST_serialize_compact( root, &buffer ) != JST_ERR_NONE ) {
      return false;
   }
   FILE * tmp = fopen( "/tmp/pass5-serialize-compact.json", "wt" );
   fputs( buffer, tmp );
   fclose ( tmp );
   free( buffer );
   return system( cmp_command ) == EXIT_SUCCESS;
}

static bool test_save_xml( JST_Element * root, const char * json_filename ) {
   char * xml_filename = strdup( json_filename );
   if( strtok( xml_filename, "." ) == NULL ) {
      free( xml_filename );
      return false;
   }
   strcat( xml_filename, ".xml" );
   char cmp_command[240];
   snprintf( cmp_command, sizeof( cmp_command ), "cmp %s /tmp/object.xml", xml_filename );
   bool ok = ( JST_save_to_xml_file( "/tmp/object.xml", root, 3 ) == JST_ERR_NONE )
      &&     ( system( cmp_command ) == EXIT_SUCCESS );
   free( xml_filename );
   return ok;
}

static bool test_json_to_xml_text( JST_Element * root, const char * json_filename ) {
   char * xml = NULL;
   if( JST_serialize_xml( root, &xml, 3 ) != JST_ERR_NONE ) {
      return false;
   }
   FILE * tmp = fopen( "/tmp/test_json_to_xml_text.xml", "wt" );
   fputs( xml, tmp );
   fclose ( tmp );
   free( xml );
   char * xml_filename = strdup( json_filename );
   if( strtok( xml_filename, "." ) == NULL ) {
      free( xml_filename );
      return false;
   }
   strcat( xml_filename, ".xml" );
   const char * fmt = "cmp %s /tmp/test_json_to_xml_text.xml";
   size_t len = strlen( fmt ) + strlen( xml_filename );
   char * cmp_command = malloc( len + 1 );
   snprintf( cmp_command, len, fmt, xml_filename );
   free( xml_filename );
   int retVal = system( cmp_command );
   free( cmp_command );
   return retVal == EXIT_SUCCESS;
}

static const char * get_error_string( JST_Error err ) {
   switch( err ) {
   case JST_ERR_ERRNO                         : return strerror( errno );
   case JST_ERR_STRING_NOT_ENDED              : return SYNTAX_ERROR_STRING_NOT_ENDED;
   case JST_ERR_SEPARATOR_EXPECTED_AFTER_TRUE : return SYNTAX_ERROR_SEPARATOR_EXPECTED_AFTER_TRUE;
   case JST_ERR_SEPARATOR_EXPECTED_AFTER_FALSE: return SYNTAX_ERROR_SEPARATOR_EXPECTED_AFTER_FALSE;
   case JST_ERR_SEPARATOR_EXPECTED_AFTER_NULL : return SYNTAX_ERROR_SEPARATOR_EXPECTED_AFTER_NULL;
   case JST_ERR_TRUE_EXPECTED                 : return SYNTAX_ERROR_TRUE_EXPECTED;
   case JST_ERR_FALSE_EXPECTED                : return SYNTAX_ERROR_FALSE_EXPECTED;
   case JST_ERR_NULL_EXPECTED                 : return SYNTAX_ERROR_NULL_EXPECTED;
   case JST_ERR_NUMBER_EXPECTED               : return SYNTAX_ERROR_NUMBER_EXPECTED;
   case JST_ERR_COLON_EXPECTED                : return SYNTAX_ERROR_COLON_EXPECTED;
   case JST_ERR_UNEXPECTED_TOKEN              : return SYNTAX_ERROR_UNEXPECTED_TOKEN;
   default                                    : return "-- unknown error --";
   }
}

int main( int argc, char * argv[] ) {
   int retVal = EXIT_FAILURE;
   if( argc < 2 ) {
      fprintf( stderr, "usage: %s <json path>\n", argv[0] );
   }
   else {
      JST_Element root = JST_Element_Zero;
      for( int i = 1; i < argc; ++i ) {
         const char * json_filename = argv[i];
         fprintf( stderr, "Parsing '%s'\n", json_filename );
         JST_SyntaxError syntax_error = JST_SyntaxError_Zero;
         JST_Error err = JST_load_from_file( json_filename, &root, &syntax_error );
         if( err != JST_ERR_NONE ) {
            const char * error = get_error_string( err );
            fprintf( stderr, "%s, at line %d, when reading:\n%s\n%*s^\n", error,
               syntax_error.line, syntax_error.context, syntax_error.pos, "" );
         }
         else {
            if( strstr( json_filename, "data/object.json" )) {
               if(   test_get             ( &root )
                  && test_add_property    ( &root )
                  && test_add_item        ( &root )
                  && test_replace_item    ( &root )
                  && test_remove_item     ( &root )
                  && test_remove_property ( &root )
                  && test_parents         ( &root )
                  && test_save_xml        ( &root, json_filename )
                  && test_json_to_xml_text( &root, json_filename ))
               {
                  retVal = EXIT_SUCCESS;
               }
            }
            else if( strstr( json_filename, "data/pass4.json" )) {
               bool ok = test_save             ( &root, json_filename )
                  &&     test_serialize        ( &root, json_filename );
               retVal = ok ? EXIT_SUCCESS : EXIT_FAILURE;
            }
            else if( strstr( json_filename, "data/pass5.json" )) {
               bool ok = test_save_compact     ( &root, json_filename )
                  &&     test_serialize_compact( &root, json_filename );
               retVal = ok ? EXIT_SUCCESS : EXIT_FAILURE;
            }
            else {
               int indent = 0;
               retVal = ( JST_walk( &root, dump, &indent ) == JST_ERR_NONE ) ? EXIT_SUCCESS : EXIT_FAILURE;
            }
         }
         JST_delete_element( &root );
      }
   }
   return retVal;
}
