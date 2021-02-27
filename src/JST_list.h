#pragma once
#include "jstools.h"

/**
 * A node in the list.
 */
typedef struct JST_Node_ {
   void *             item; //!< The data
   struct JST_Node_ * next; //!< The next node, or NULL for the last
} JST_List;

/**
 * Constant defined to initialize safely a variable of type JST_List.
 */
extern const JST_List JST_List_Zero;

/**
 * A callback signature used by JST_List_move_to to transfer ownership of node's data.
 */
typedef void ( * JST_Assign )( void * dest, unsigned index, void * src );

/**
 * Allocate a node and chain it after current, if it's not null otherwise update first.
 * @return JST_ERR_NONE or JST_ERR_NULL_ARGUMENT when first or current are null.
 */
JST_Error JST_List_push_back( JST_List ** first, JST_List ** current, void * data );

/**
 * For each item in list, call assign to transfert ownership of items and free the list, node by node.
 * @param from the list to clear
 * @param dest the destination of transfert, a data structure (opaque here)
 * @param assign the callback able to make the transfert from JST_List to another data structure (opaque here)
 * @return JST_ERR_NONE or JST_ERR_NULL_ARGUMENT when from, dest or assign are null.
 */
JST_Error JST_List_move_to( JST_List * from, void * dest, JST_Assign assign );
