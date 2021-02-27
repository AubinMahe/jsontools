#include "jstools.h"
#include "JST_pairs_compare.h"

#include <string.h>
#include <stdlib.h>

static JST_Error element_copy( JST_Element * dest, const JST_Element * src );

static JST_Error array_item_copy( JST_Array * parent, JST_ArrayItem * src, JST_ArrayItem ** pdest ) {
   *pdest = calloc( 1, sizeof( JST_ArrayItem ));
   JST_ArrayItem * dest = *pdest;
   if( dest == NULL ) {
      return JST_ERR_ERRNO;
   }
   dest->parent = parent;
   return element_copy( &(dest->element), &(src->element));
}

static JST_Error pair_copy( JST_Object * parent, JST_Pair * src, JST_Pair ** pdest ) {
   *pdest = calloc( 1, sizeof( JST_Pair ));
   JST_Pair * dest = *pdest;
   if( dest == NULL ) {
      return JST_ERR_ERRNO;
   }
   dest->parent = parent;
   dest->name   = strdup( src->name );
   return element_copy( &(dest->element), &(src->element));
}

static JST_Error element_copy( JST_Element * dest, const JST_Element * src ) {
   dest->type = src->type;
   switch( src->type ) {
   case JST_OBJECT:
      dest->value.object.parent = dest;
      dest->value.object.count  = src->value.object.count;
      dest->value.object.items  = calloc( dest->value.object.count, sizeof( JST_Pair * ));
      for( unsigned i = 0; i < src->value.object.count; ++i ) {
         pair_copy( &(dest->value.object), src->value.object.items[i], &(dest->value.object.items[i]));
      }
      break;
   case JST_ARRAY:
      dest->value.array.parent = dest;
      dest->value.array.count  = src->value.array.count;
      dest->value.array.items  = calloc( dest->value.array.count, sizeof( JST_ArrayItem * ));
      for( unsigned i = 0; i < src->value.array.count; ++i ) {
         array_item_copy( &(dest->value.array), src->value.array.items[i], &(dest->value.array.items[i]));
      }
      break;
   case JST_BOOLEAN: dest->value.boolean = src->value.boolean;          break;
   case JST_INTEGER: dest->value.integer = src->value.integer;          break;
   case JST_DOUBLE : dest->value.dbl     = src->value.dbl;              break;
   case JST_STRING : dest->value.string  = strdup( src->value.string ); break;
   case JST_NULL   : dest->value.string  = NULL;                        break;
   default:
      return JST_ERR_NULL_TYPE;
   }
   return JST_ERR_NONE;
}

JST_Error JST_add_property( JST_Object * object, unsigned index, const char * name, const JST_Element * value ) {
   JST_Pair * item = calloc( 1, sizeof( JST_Pair ));
   if( ! item ) {
      return JST_ERR_ERRNO;
   }
   item->name   = strdup( name );
   item->parent = object;
   JST_Error error = element_copy( &(item->element), value );
   if( error == JST_ERR_NONE ) {
      ++(object->count);
      object->items = realloc( object->items, object->count * sizeof( JST_Pair * ));
      if( object->items == NULL ) {
         object->count = 0;
         error = JST_ERR_ERRNO;
      }
      else {
         object->items[object->count-1] = item;
         qsort( object->items, object->count, sizeof( JST_Pair *), JST_pairs_compare );
      }
   }
   if( error == JST_ERR_NONE ) {
      if( index != -1U ) {
         if( index == 0 ) {
            item->next = object->first;
            object->first = item;
         }
         else {
            JST_Pair * prev = object->first;
            for( unsigned i = 1; prev->next &&( i < index ); ++i, prev = prev->next );
            item->next = prev->next;
            prev->next = item;
         }
      }
   }
   else {
      JST_delete_element( &(item->element));
      free( item->name );
      free( item );
   }
   return error;
}

JST_Error JST_add_item( JST_Array * array, unsigned index, const JST_Element * value ) {
   JST_ArrayItem * item = calloc( 1, sizeof( JST_ArrayItem ));
   if( ! item ) {
      return JST_ERR_ERRNO;
   }
   item->parent = array;
   JST_Error error = element_copy( &(item->element), value );
   if( error == JST_ERR_NONE ) {
      ++(array->count);
      array->items = realloc( array->items, array->count * sizeof( JST_ArrayItem * ));
      if( array->items == NULL ) {
         array->count = 0;
         error = JST_ERR_ERRNO;
      }
      else {
         if( index == -1U ) {
            array->items[array->count-1] = item;
         }
         else {
            memmove( array->items + index + 1, array->items + index, ( array->count - 1 - index ) * sizeof( JST_ArrayItem * ));
            array->items[index] = item;
         }
      }
   }
   if( error != JST_ERR_NONE ) {
      JST_delete_element( &(item->element));
      free( item );
   }
   return error;
}

JST_Error JST_replace_item( JST_Array * array, unsigned index, const JST_Element * src ) {
   if(( array == NULL )||( src == NULL )) {
      return JST_ERR_NULL_ARGUMENT;
   }
   if( index > array->count ) {
      return JST_ERR_ARRAY_INDEX_OUT_OF_RANGE;
   }
   return element_copy( &(array->items[index]->element), src );
}

JST_Error JST_remove_property( JST_Object * object, const char * name ) {
   if(( object == NULL )||( name == NULL )) {
      return JST_ERR_NULL_ARGUMENT;
   }
   for( unsigned i = 0; i < object->count; ++i ) {
      JST_Pair * pair = object->items[i];
      if( 0 == strcmp( pair->name, name )) {
         memmove( object->items+i, object->items+i+1, sizeof( JST_Pair * )*( object->count - i - 1 ));
         --(object->count);
         if( object->first == pair ) {
            object->first = pair->next;
         }
         else {
            for( JST_Pair * iter = object->first; iter; iter = iter->next ) {
               if( iter->next == pair ) {
                  iter->next = pair->next;
                  pair->next = NULL;
                  break;
               }
            }

         }
         JST_delete_pair( pair );
         return JST_ERR_NONE;
      }
   }
   return JST_ERR_NOT_FOUND;
}

JST_Error JST_remove_item( JST_Array * array, unsigned index ) {
   if( array == NULL ) {
      return JST_ERR_NULL_ARGUMENT;
   }
   if( index >= array->count ) {
      return JST_ERR_ARRAY_INDEX_OUT_OF_RANGE;
   }
   JST_ArrayItem * item = array->items[index];
   memmove( array->items+index, array->items+index+1, sizeof( JST_ArrayItem * )*( array->count - index - 1 ));
   --(array->count);
   JST_delete_array_item( item );
   return JST_ERR_NONE;
}
