#include "jstools.h"
#include "JST_list.h"
#include "JST_string.h"
#include "JST_pairs_compare.h"

#include <ctype.h>
#include <errno.h>
#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <sys/stat.h>

#include "jstools.h"

bool JST_DEBUG_SHOW_TOKENS = false;

const JST_Array       JST_Array_Zero       = { .items = NULL, .count = 0U, .parent = NULL };
const JST_Object      JST_Object_Zero      = { .items = NULL, .count = 0U, .parent = NULL, .first = NULL };
const JST_Value       JST_Value_Zero       = { .object = JST_Object_Zero };
const JST_Element     JST_Element_Zero     = { .type = JST_NULL, .value = JST_Value_Zero, .parent_is_pair = false, .parent = NULL };
const JST_ArrayItem   JST_ArrayItem_Zero   = { .parent = NULL, .element = JST_Element_Zero };
const JST_Pair        JST_Pair_Zero        = { .parent = NULL, .element = JST_Element_Zero, .name = NULL, .next = NULL };
const JST_SyntaxError JST_SyntaxError_Zero = { .line = 0U, .pos = 0U };

typedef enum {
   TOKEN_NONE,
   TOKEN_OPEN_OBJECT,
   TOKEN_COLON,
   TOKEN_CLOSE_OBJECT,
   TOKEN_COMMA,
   TOKEN_OPEN_ARRAY,
   TOKEN_CLOSE_ARRAY,
   TOKEN_TRUE,
   TOKEN_FALSE,
   TOKEN_INTEGER,
   TOKEN_DOUBLE,
   TOKEN_STRING,
   TOKEN_NULL,
} TokenType;

static const TokenType TokenType_Zero = TOKEN_NONE;

typedef union {
   int64_t i;
   double  d;
   char *  s;
} TokenValue;

static const TokenValue TokenValue_Zero = { .s = NULL };

typedef struct {
   TokenType  type;
   TokenValue value;
} Token;

static const Token Token_Zero = { .type = TokenType_Zero, .value = TokenValue_Zero };

typedef struct {
   FILE *    stream;
   char      look_ahead;
   char      buffer[4096];
   size_t    limit;
   size_t    pos;
   size_t    line;
   JST_Error error;
} Context;

static const Context Context_Zero = {
   .stream     = NULL,
   .look_ahead = '\0',
   .buffer     = "",
   .limit      = 0U,
   .pos        = 0U,
   .line       = 0U,
   .error      = JST_ERR_NONE
};

static const char EndOfFile = 26;

static bool get_next_char( Context * context, char * c ) {
   if( context->look_ahead ) {
      *c = context->look_ahead;
      context->look_ahead = '\0';
      return true;
   }
   if( context->pos >= context->limit ) {
      context->limit = fread( context->buffer, 1, sizeof( context->buffer ), context->stream );
      if(( context->limit == 0 )&& feof( context->stream )) {
         *c = EndOfFile;
         return true;
      }
      context->pos = 0;
   }
   *c = context->buffer[context->pos++];
   return true;
}

static bool remaining_chars_are_only_whitespaces( Context * context ) {
   while( context->pos < context->limit ) {
      char c = 0;
      if( get_next_char( context, &c )&&( NULL == strchr( " \t\r\n", c ))) {
         context->error = JST_ERR_UNEXPECTED_TOKEN;
         return false;
      }
   }
   return true;
}

static bool check_separator( Context * context, JST_Error error ) {
   char c = 0;
   if( ! get_next_char( context, &c )) {
      context->error = error;
      return false;
   }
   if(  ( c == ' '  )||( c == '\t' )
      ||( c == '\r' )||( c == '\n' )
      ||( c == ','  )||( c == ':'  )
      ||( c == '}'  )||( c == ']'  )
      ||( c == '\0' )||( c == EndOfFile ))
   {
      context->look_ahead = c; // On remet ce qu'on a lu de trop
      return true;
   }
   context->error = error;
   return false;
}

static bool is_separator_after_number( char c ) {
   return( c == '\0' )
      || ( c == ' '  )||( c == '\t' )
      || ( c == '\r' )||( c == '\n' )
      || ( c == ','  )||( c == '}'  )||( c == ']' );
}

static bool get_number( Context * context, char c, Token * token ) {
   JST_String s = JST_String_Zero;
   JST_String_append_char( &s, c );
   while( get_next_char( context, &c )
      &&( ! is_separator_after_number( c ))
      &&( c != EndOfFile )
      &&  JST_String_append_char( &s, c ));
   context->look_ahead = c; // On remet ce qu'on a lu de trop
   if(( s.length > 2 )&&( s.buffer[0] == '0' )&&( s.buffer[1] != '.' )) {
      context->error = JST_ERR_NUMBER_EXPECTED;
      JST_String_delete( &s );
      return false;
   }
   s.buffer[s.length] = '\0';
   char * err = NULL;
   token->value.i = strtoll( s.buffer, &err, 10 );
   if( is_separator_after_number( *err )) {
      token->type = TOKEN_INTEGER;
      if( JST_DEBUG_SHOW_TOKENS ) {
         fprintf( stderr, "TOKEN_INTEGER: %ld\n", token->value.i );
      }
      JST_String_delete( &s );
      return true;
   }
   token->value.d = strtod( s.buffer, &err );
   if( is_separator_after_number( *err )) {
      token->type = TOKEN_DOUBLE;
      if( JST_DEBUG_SHOW_TOKENS ) {
         fprintf( stderr, "TOKEN_DOUBLE: %G\n", token->value.d );
      }
      JST_String_delete( &s );
      return true;
   }
   context->error = JST_ERR_NUMBER_EXPECTED;
   JST_String_delete( &s );
   return false;
}

static bool get_string( Context * context, Token * token ) {
   token->type = TOKEN_STRING;
   JST_String s = JST_String_Zero;
   char c = '"', p;
   do {
      p = c;
      if(( ! get_next_char( context, &c ))||( ! JST_String_append_char( &s, c ))) {
         context->error = JST_ERR_ERRNO;
         JST_String_delete( &s );
         return false;
      }
      if( strchr( "\b\f\n\r\t", c )) {
         context->error = JST_ERR_STRING_CONTROL_CHAR_MUST_BE_ESCAPED;
         JST_String_delete( &s );
         return false;
      }
      if( p == '\\' ) {
         if( NULL == strchr( "\"\\/bfnrtu", c )) {
            context->error = JST_ERR_STRING_BAD_ESCAPE_SEQUENCE;
            JST_String_delete( &s );
            return false;
         }
         else if( c == 'u' ) {
            for( int i = 0; i < 4; ++i ) {
               if( ! get_next_char( context, &c )) {
                  JST_String_delete( &s );
                  return false;
               }
               if( ! isxdigit( c )) {
                  context->error = JST_ERR_STRING_BAD_ESCAPE_SEQUENCE;
                  JST_String_delete( &s );
                  return false;
               }
               if( ! JST_String_append_char( &s, c )) {
                  context->error = JST_ERR_ERRNO;
                  JST_String_delete( &s );
                  return false;
               }
            }
         }
         if(  ( ! get_next_char( context, &c )  )
            ||( ! JST_String_append_char( &s, c )))
         {
            context->error = JST_ERR_ERRNO;
            JST_String_delete( &s );
            return false;
         }
      }
   } while( c != '"' );
   if( c == '"' ) {
      if( s.buffer ) {
         s.buffer[s.length-1] = '\0';
         token->value.s = strdup( s.buffer );
         JST_String_delete( &s );
         if( JST_DEBUG_SHOW_TOKENS ) {
            fprintf( stderr, "TOKEN_STRING: '%s'\n", token->value.s );
         }
         return true;
      }
      context->error = JST_ERR_ERRNO;
      JST_String_delete( &s );
      return false;
   }
   context->error = JST_ERR_STRING_NOT_ENDED;
   JST_String_delete( &s );
   return false;
}

static bool get_true( Context * context, Token * token ) {
   char c =                                   't';
   if(   get_next_char( context, &c )&&( c == 'r' )
      && get_next_char( context, &c )&&( c == 'u' )
      && get_next_char( context, &c )&&( c == 'e' ))
   {
      token->type = TOKEN_TRUE;
      if( JST_DEBUG_SHOW_TOKENS ) {
         fprintf( stderr, "TOKEN_TRUE\n" );
      }
      return check_separator( context, JST_ERR_SEPARATOR_EXPECTED_AFTER_TRUE );
   }
   context->error = JST_ERR_TRUE_EXPECTED;
   return false;
}

static bool get_false( Context * context, Token * token ) {
   char c =                                   'f';
   if(   get_next_char( context, &c )&&( c == 'a' )
      && get_next_char( context, &c )&&( c == 'l' )
      && get_next_char( context, &c )&&( c == 's' )
      && get_next_char( context, &c )&&( c == 'e' ))
   {
      token->type = TOKEN_FALSE;
      if( JST_DEBUG_SHOW_TOKENS ) {
         fprintf( stderr, "TOKEN_FALSE\n" );
      }
      return check_separator( context, JST_ERR_SEPARATOR_EXPECTED_AFTER_FALSE );
   }
   context->error = JST_ERR_FALSE_EXPECTED;
   return false;
}

static bool get_null( Context * context, Token * token ) {
   char c =                                   'n';
   if(   get_next_char( context, &c )&&( c == 'u' )
      && get_next_char( context, &c )&&( c == 'l' )
      && get_next_char( context, &c )&&( c == 'l' ))
   {
      token->type = TOKEN_NULL;
      if( JST_DEBUG_SHOW_TOKENS ) {
         fprintf( stderr, "TOKEN_NULL\n" );
      }
      return check_separator( context, JST_ERR_SEPARATOR_EXPECTED_AFTER_NULL );
   }
   context->error = JST_ERR_NULL_EXPECTED;
   return false;
}

static bool get_next_token( Context * context, Token * token ) {
   token->type = TOKEN_NONE;
   char c = ' ';
   while(( c == ' ' )||( c == '\t' )||( c == '\r' )||( c == '\n' )) {
      if(( c == '\r' )||( c == '\n' )) {
         context->line++;
      }
      if( ! get_next_char( context, &c )) {
         return false;
      }
   }
   switch( c ) {
   case '{':
      if( JST_DEBUG_SHOW_TOKENS ) {
         fprintf( stderr, "TOKEN_OPEN_OBJECT\n" );
      }
      token->type  = TOKEN_OPEN_OBJECT;
      break;
   case ':':
      if( JST_DEBUG_SHOW_TOKENS ) {
         fprintf( stderr, "TOKEN_COLON\n" );
      }
      token->type  = TOKEN_COLON;
      break;
   case ',':
      if( JST_DEBUG_SHOW_TOKENS ) {
         fprintf( stderr, "TOKEN_COMMA\n" );
      }
      token->type  = TOKEN_COMMA;
      break;
   case '}':
      if( JST_DEBUG_SHOW_TOKENS ) {
         fprintf( stderr, "TOKEN_CLOSE_OBJECT\n" );
      }
      token->type  = TOKEN_CLOSE_OBJECT;
      break;
   case '[':
      if( JST_DEBUG_SHOW_TOKENS ) {
         fprintf( stderr, "TOKEN_OPEN_ARRAY\n" );
      }
      token->type  = TOKEN_OPEN_ARRAY;
      break;
   case ']':
      if( JST_DEBUG_SHOW_TOKENS ) {
         fprintf( stderr, "TOKEN_CLOSE_ARRAY\n" );
      }
      token->type  = TOKEN_CLOSE_ARRAY;
      break;
   case '"': return get_string( context, token );
   case 't': return get_true  ( context, token );
   case 'f': return get_false ( context, token );
   case 'n': return get_null  ( context, token );
   case '-': case '0':case '1':case '2':case '3':
   case '4': case '5':case '6':case '7':case '8':
   case '9': return get_number( context, c, token );
   default:
      context->error = JST_ERR_UNEXPECTED_TOKEN;
      return false;
   }
   return true;
}

static bool add_object( Context * context, JST_Object * object, JST_Element * parent );
static bool add_array( Context * context, JST_Array * array, JST_Element * parent );

static bool set_value( Context * context, JST_Element * node, JST_Pair * pparent, JST_ArrayItem * aparent, const Token * token ) {
   if( pparent ) {
      node->parent_is_pair = true;
      node->parent = pparent;
   }
   else {
      node->parent_is_pair = false;
      node->parent = aparent;
   }
   switch( token->type ) {
   case TOKEN_OPEN_OBJECT:
      node->type          = JST_OBJECT;
      return add_object( context, &(node->value.object), node );
   case TOKEN_OPEN_ARRAY:
      node->type          = JST_ARRAY;
      return add_array( context, &(node->value.array), node );
   case TOKEN_TRUE:
      node->type          = JST_BOOLEAN;
      node->value.boolean = true;
      break;
   case TOKEN_FALSE:
      node->type          = JST_BOOLEAN;
      node->value.boolean = false;
      break;
   case TOKEN_INTEGER:
      node->type          = JST_INTEGER;
      node->value.integer = token->value.i;
      break;
   case TOKEN_DOUBLE:
      node->type          = JST_DOUBLE;
      node->value.dbl     = token->value.d;
      break;
   case TOKEN_STRING:
      node->type          = JST_STRING;
      node->value.string  = token->value.s;
      break;
   case TOKEN_NULL:
      node->type          = JST_NULL;
      break;
   default:
      context->error = JST_ERR_UNEXPECTED_TOKEN;
      return false;
   }
   return true;
}

static JST_Pair * add_property( Context * context, char * property_name, JST_Object * parent ) {
   JST_Pair * item = calloc( 1, sizeof( JST_Pair ));
   if( ! item ) {
      context->error = JST_ERR_ERRNO;
      return NULL;
   }
   item->name   = property_name;
   item->parent = parent;
   Token token;
   if( get_next_token( context, &token )) {
      if( token.type == TOKEN_COLON ) {
         if(   get_next_token( context, &token )
            && set_value( context, &(item->element), item, NULL, &token ))
         {
            return item;
         }
      }
      else {
         context->error = JST_ERR_COLON_EXPECTED;
      }
   }
   free( property_name );
   free( item );
   return NULL;
}

static JST_ArrayItem * add_item( Context * context, const Token * token, JST_Array * parent ) {
   JST_ArrayItem * item = calloc( 1, sizeof( JST_ArrayItem ));
   if( ! item ) {
      context->error = JST_ERR_ERRNO;
      return NULL;
   }
   item->parent = parent;
   if( set_value( context, &(item->element), NULL, item, token )) {
      return item;
   }
   free( item );
   return NULL;
}

static void set_pair( void * dest, unsigned index, void * src ) {
   JST_Object * object = dest;
   if( index == 0 ) {
      object->first = src;
   }
   else {
      JST_Pair * prev = object->items[index-1];
      prev->next = src;
   }
   object->items[index] = src;
}

static void set_item( void * dest, unsigned index, void * src ) {
   ((JST_Array *)dest)->items[index] = src;
}

static bool add_object( Context * context, JST_Object * object, JST_Element * parent ) {
   object->parent = parent;
   Token token = { .type = TOKEN_NONE, .value = { .s = NULL }};
   JST_List * first   = NULL;
   JST_List * current = NULL;
   bool item_expected = true;
   while(( context->error == JST_ERR_NONE )&&( token.type != TOKEN_CLOSE_OBJECT )) {
      if( get_next_token( context, &token )) {
         if( item_expected ) {
            if(( token.type == TOKEN_CLOSE_OBJECT )&&( object->count == 0 )) {
               break;
            }
            if( token.type == TOKEN_STRING ) {
               JST_Pair * pair = add_property( context, token.value.s, object );
               if( pair ) {
                  JST_List_push_back( &first, &current, pair );
                  ++(object->count);
                  if( get_next_token( context, &token )) {
                     if(( token.type != TOKEN_COMMA )&&( token.type != TOKEN_CLOSE_OBJECT )) {
                        context->error = JST_ERR_UNEXPECTED_TOKEN;
                     }
                     else {
                        item_expected = token.type == TOKEN_COMMA;
                     }
                  }
                  else {
                     context->error = JST_ERR_UNEXPECTED_TOKEN;
                  }
               }
            }
            else {
               context->error = JST_ERR_UNEXPECTED_TOKEN;
            }
         }
         else {
            context->error = JST_ERR_UNEXPECTED_TOKEN;
         }
      }
   }
   object->items = calloc( object->count, sizeof( JST_Pair *));
   JST_List_move_to( first, object, set_pair );
   qsort( object->items, object->count, sizeof( JST_Pair *), JST_pairs_compare );
   if( context->error == JST_ERR_NONE ) {
      return true;
   }
   return false;
}

static bool add_array( Context * context, JST_Array * array, JST_Element * parent ) {
   array->parent = parent;
   Token      token   = { .type = TOKEN_NONE, .value = { .s = NULL }};
   JST_List * first   = NULL;
   JST_List * current = NULL;
   bool item_expected = true;
   while(( context->error == JST_ERR_NONE )&&( token.type != TOKEN_CLOSE_ARRAY )) {
      if( get_next_token( context, &token )) {
         if( item_expected ) {
            if(( token.type == TOKEN_CLOSE_ARRAY )&&( array->count == 0 )) {
               break;
            }
            JST_ArrayItem * item = add_item( context, &token, array );
            if( item ) {
               JST_List_push_back( &first, &current, item );
               ++(array->count);
               if( get_next_token( context, &token )) {
                  if(( token.type != TOKEN_COMMA )&&( token.type != TOKEN_CLOSE_ARRAY )) {
                     context->error = JST_ERR_UNEXPECTED_TOKEN;
                  }
                  else {
                     item_expected = token.type == TOKEN_COMMA;
                  }
               }
               else {
                  context->error = JST_ERR_UNEXPECTED_TOKEN;
               }
            }
         }
         else {
            context->error = JST_ERR_UNEXPECTED_TOKEN;
         }
      }
   }
   array->items = calloc( array->count, sizeof( JST_ArrayItem *));
   JST_List_move_to( first, array, set_item );
   return context->error == JST_ERR_NONE;
}

JST_Error JST_load_from_stream( FILE * stream, JST_Element * root, JST_SyntaxError * syntax_error ) {
   if( syntax_error ) {
      *syntax_error = JST_SyntaxError_Zero;
   }
   if(( stream == NULL )||( root == NULL )) {
      return JST_ERR_NULL_ARGUMENT;
   }
   *root = JST_Element_Zero;
   Context context = Context_Zero;
   Token   token   = Token_Zero;
   context.stream = stream;
   bool    ok      = true
      && get_next_token( &context, &token )
      && set_value( &context, root, NULL, NULL, &token )
      && remaining_chars_are_only_whitespaces( &context );
   if( ! ok ) {
      if( syntax_error ) {
         syntax_error->line = (unsigned)( 1 + context.line );
         size_t pos = (( context.pos < context.limit )||(context.limit == 0 )) ? context.pos : ( context.limit - 1 );
         while(( pos > 0 )
            && (( context.pos - pos ) < ( sizeof( syntax_error->context ) / 2 ))
            && ( context.buffer[pos] != '\r' )
            && ( context.buffer[pos] != '\n' ))
         {
            --pos;
         }
         if(( context.buffer[pos] == '\r' )||( context.buffer[pos] == '\n' )) {
            ++pos;
         }
         strncpy( syntax_error->context, context.buffer + pos, sizeof( syntax_error->context ) - 1 );
         syntax_error->context[sizeof( syntax_error->context )-1] = '\0';
         syntax_error->pos  = (unsigned)( context.pos - pos );
         if( syntax_error->pos > 0 ) {
            --(syntax_error->pos);
         }
      }
   }
   return context.error;
}

JST_Error JST_load_from_file( const char * path, JST_Element * root, JST_SyntaxError * syntax_error ) {
   if(( path == NULL )) {
      return JST_ERR_NULL_ARGUMENT;
   }
   FILE * stream = fopen( path, "rt" );
   if( stream == NULL ) {
      return JST_ERR_ERRNO;
   }
   JST_Error error = JST_load_from_stream( stream, root, syntax_error );
   int myerrno = errno;
   fclose( stream );
   errno = myerrno;
   return error;
}
