#include "jstools.h"

#include <stdio.h>
#include <stddef.h>

static bool visit_element( const char * name, unsigned index, const JST_Element * element, JST_Visitor visitor, void * user_context );

static bool visit_object( const JST_Object * object, JST_Visitor visitor, void * user_context ) {
   for( unsigned i = 0; i < object->count; ++i ) {
      if( ! visit_element( object->items[i]->name, -1U, &(object->items[i]->element), visitor, user_context )) {
         return false;
      }
   }
   return true;
}

static bool visit_array( const JST_Array * array, JST_Visitor visitor, void * user_context ) {
   for( unsigned i = 0; i < array->count; ++i ) {
      if( ! visit_element( NULL, i, &(array->items[i]->element), visitor, user_context )) {
         return false;
      }
   }
   return true;
}

static bool visit_element( const char * name, unsigned index, const JST_Element * element, JST_Visitor visitor, void * context ) {
   (*visitor)( name, index, true, element, context );
   switch( element->type ) {
   case JST_OBJECT:
      if( ! visit_object( &(element->value.object), visitor, context )) {
         return false;
      }
      break;
   case JST_ARRAY:
      if( ! visit_array( &(element->value.array), visitor, context )) {
         return false;
      }
      break;
   default: break;
   }
   (*visitor)( name, index, false, element, context );
   return true;
}

JST_Error JST_walk( const JST_Element * root, JST_Visitor visitor, void * context ) {
   if(( root == NULL )||( visitor == NULL )) {
      return JST_ERR_NULL_ARGUMENT;
   }
   return visit_element( NULL, -1U, root, visitor, context ) ?  JST_ERR_NONE : JST_ERR_INTERRUPTED;
}
