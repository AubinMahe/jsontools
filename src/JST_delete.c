#include "jstools.h"

#include <stdlib.h>

JST_Error JST_delete_pair( JST_Pair * pair ) {
   if( pair == NULL ) {
      return JST_ERR_NULL_ARGUMENT;
   }
   JST_Error error = JST_delete_element( &(pair->element));
   free( pair->name );
   free( pair );
   return error;
}

JST_Error JST_delete_object( JST_Object * object ) {
   if( object == NULL ) {
      return JST_ERR_NULL_ARGUMENT;
   }
   JST_Error error = JST_ERR_NONE;
   for( unsigned i = 0; ( error == JST_ERR_NONE )&&( i < object->count ); ++i ) {
      error = JST_delete_pair( object->items[i] );
   }
   free( object->items );
   object->parent = NULL;
   object->count  = 0;
   object->first  = NULL;
   object->items  = NULL;
   return error;
}

JST_Error JST_delete_array_item( JST_ArrayItem * item ) {
   if( item == NULL ) {
      return JST_ERR_NULL_ARGUMENT;
   }
   JST_Error error = JST_delete_element( &(item->element));
   free( item );
   return error;
}

JST_Error JST_delete_array( JST_Array * array ) {
   if( array == NULL ) {
      return JST_ERR_NULL_ARGUMENT;
   }
   JST_Error error = JST_ERR_NONE;
   for( unsigned i = 0; ( error == JST_ERR_NONE )&&( i < array->count ); ++i ) {
      error = JST_delete_array_item( array->items[i] );
   }
   free( array->items );
   array->parent = NULL;
   array->count  = 0;
   array->items  = NULL;
   return error;
}

JST_Error JST_delete_element( JST_Element * element ) {
   if( element == NULL ) {
      return JST_ERR_NULL_ARGUMENT;
   }
   switch( element->type ) {
   case JST_OBJECT: JST_delete_object( &(element->value.object)); break;
   case JST_ARRAY : JST_delete_array( &(element->value.array)); break;
   case JST_STRING: free( element->value.string ); break;
   default: break;
   }
   element->type         = JST_NULL;
   element->value.string = NULL;
   return JST_ERR_NONE;
}
