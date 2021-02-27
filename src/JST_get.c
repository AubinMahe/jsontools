#include "jstools.h"
#include "JST_pairs_compare.h"

#include <string.h>
#include <stdlib.h>
#include <ctype.h>

static const char * STRTOK_PATH = ".[]";

JST_Error JST_get( const char * jsonpath, JST_Element * root, JST_Element ** dest ) {
   if(( root == NULL )||( jsonpath == NULL )||( dest == NULL )) {
      return JST_ERR_NULL_ARGUMENT;
   }
   char * tokenized = strdup( jsonpath );
   if( tokenized == NULL ) {
      return JST_ERR_ERRNO;
   }
   char * token = strtok( tokenized, STRTOK_PATH );
   if( token == NULL ) {
      free( tokenized );
      return JST_ERR_PATH_SYNTAX;
   }
   JST_Element * iter = root;
   while( iter && token ) {
      if( isdigit( *token )) {
         char * err;
         long unsigned index = strtoul( token, &err, 10 );
         if( err && *err ) {
            free( tokenized );
            return JST_ERR_PATH_SYNTAX;
         }
         if( iter->type !=  JST_ARRAY ) {
            free( tokenized );
            return JST_ERR_PATH_SYNTAX;
         }
         if( index >= iter->value.array.count ) {
            free( tokenized );
            return JST_ERR_NOT_FOUND;
         }
         JST_ArrayItem * item = iter->value.array.items[index];
         iter = &( item->element );
      }
      else {
         if( iter->type !=  JST_OBJECT ) {
            free( tokenized );
            return JST_ERR_PATH_SYNTAX;
         }
         JST_Pair key, *key_addr = &key;
         key.name = token;
         JST_Pair ** pair =
            bsearch( &key_addr, iter->value.object.items, iter->value.object.count, sizeof( JST_Pair * ), JST_pairs_compare );
         if( pair == NULL ) {
            free( tokenized );
            return JST_ERR_NOT_FOUND;
         }
         iter = &((*pair)->element);
      }
      token = strtok( NULL, STRTOK_PATH );
   }
   free( tokenized );
   *dest = iter;
   return JST_ERR_NONE;
}
