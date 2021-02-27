#include "JST_list.h"

#include <stdlib.h>

const JST_List JST_List_Zero = { .item = NULL, .next = NULL };

JST_Error JST_List_push_back( JST_List ** first, JST_List ** current, void * data ) {
   if(( first == NULL )||( current == NULL )) {
      return JST_ERR_NULL_ARGUMENT;
   }
   if( *first == NULL ) {
      *first = *current = calloc( 1, sizeof( JST_List ));
   }
   else {
      if( *current == NULL ) {
         return JST_ERR_NULL_ARGUMENT;
      }
      (*current)->next = calloc( 1, sizeof( JST_List ));
      *current = (*current)->next;
   }
   (*current)->item = data;
   return JST_ERR_NONE;
}

JST_Error JST_List_move_to( JST_List * from, void * dest, JST_Assign assign ) {
   if(( from == NULL )||( dest == NULL )||( assign == NULL )) {
      return JST_ERR_NULL_ARGUMENT;
   }
   JST_List * iter = from;
   for( unsigned i = 0; iter; ++i ) {
      assign( dest, i, iter->item );
      JST_List * next = iter->next;
      free( iter );
      iter = next;
   }
   return JST_ERR_NONE;
}
