#pragma once

#include "stdbool.h"
#include "stdint.h"
#include "stdio.h"

/**
 * A JSON array.
 */
typedef struct {
   struct JST_ArrayItem_ ** items;  //!< Array of JST_ArrayItem
   unsigned                 count;  //!< Cardinality of the previous array
   struct JST_Element_ *    parent; //!< of type JST_ArrayItem or JST_Pair, case selector is parent->type
} JST_Array;

/**
 * Constant defined to initialize safely a variable of type JST_Array.
 */
extern const JST_Array JST_Array_Zero;

/**
 * A JSON object, a sorted set of named-value pairs
 */
typedef struct {
   struct JST_Pair_ **   items;  //!< Array of JST_Pair, ordered by JST_Pair.name to ease search with bsearch()
   unsigned              count;  //!< Cardinality of the previous array
   struct JST_Element_ * parent; //!< of type JST_ArrayItem or JST_Pair, case selector is parent->type
   struct JST_Pair_ *    first;  //!< A linked list is mandatory to preserve the properties's order
} JST_Object;

/**
 * Constant defined to initialize safely a variable of type JST_Object.
 */
extern const JST_Object JST_Object_Zero;

/**
 * A JSON Value may be an object, an array, a boolean, a number or a string.
 * The number is splitted here in integer (64 bits) and double.
 */
typedef union {
   JST_Object object;  //!< An object value
   JST_Array  array;   //!< An array value
   bool       boolean; //!< A boolean value
   int64_t    integer; //!< A 64 bits integer value
   double     dbl;     //!< An IEEE 754 64 bits floating point value
   char *     string;  //!< A nul terminated string
} JST_Value;

/**
 * Constant defined to initialize safely a variable of type JST_Value.
 */
extern const JST_Value JST_Value_Zero;

/**
 * The JSON Value types
 */
typedef enum {
   JST_OBJECT,  //!< A list of named-value pairs
   JST_ARRAY,   //!< A list of any type
   JST_BOOLEAN, //!< A boolean, the literal `true` or `false`
   JST_INTEGER, //!< A 64 bits integer
   JST_DOUBLE,  //!< A 64 bits floating point IEEE 754
   JST_STRING,  //!< A nul terminated string
   JST_NULL,    //!< The literal `null`
} JST_ValueType;

/**
 * An element is a typed value.
 */
typedef struct JST_Element_ {
   JST_ValueType type;           //!< Element type
   JST_Value     value;          //!< Element value
   bool          parent_is_pair; //!< Used to cast parent field
   void *        parent;         //!< Parent of this value,  of type JST_Pair or JST_ArrayItem, may be null when it's the root.
} JST_Element;

/**
 * Constant defined to initialize safely a variable of type JST_Element.
 */
extern const JST_Element JST_Element_Zero;

/**
 * An array item has a parent and a typed value.
 */
typedef struct JST_ArrayItem_ {
   JST_Array * parent;  //!< The parent, an array
   JST_Element element; //!< The data, an element of any type
} JST_ArrayItem;

/**
 * Constant defined to initialize safely a variable of type JST_ArrayItem.
 */
extern const JST_ArrayItem JST_ArrayItem_Zero;

/**
 * A object attribute item has a parent and is a named-typed-value pair.
 */
typedef struct JST_Pair_ {
   JST_Object *       parent;  //!< This property's owner
   JST_Element        element; //!< The value associated with the name
   char *             name;    //!< This property's name
   struct JST_Pair_ * next;    //!< A linked list is mandatory to preserve the properties's order
} JST_Pair;

/**
 * Constant defined to initialize safely a variable of type JST_Pair.
 */
extern const JST_Pair JST_Pair_Zero;

/**
 * When things goes wrong, an error information is given.
 */
typedef enum {
   JST_ERR_NONE,                                //!< All correct
   JST_ERR_NULL_ARGUMENT,                       //!< Function misused
   JST_ERR_NULL_TYPE,                           //!< A type field has been set to JST_NONE
   JST_ERR_ERRNO,                               //!< strerror() or perror() can be used to show the operating system layer error
   JST_ERR_STRING_NOT_ENDED,                    //!< ending quote has not been found
   JST_ERR_STRING_BAD_ESCAPE_SEQUENCE,          //!< only ", \, /, b, f, n, r, t, u may be escaped
   JST_ERR_STRING_CONTROL_CHAR_MUST_BE_ESCAPED, /*!< a control character has been found, it must be escaped */
   JST_ERR_SEPARATOR_EXPECTED_AFTER_TRUE,       //!< syntax error on 'true'
   JST_ERR_SEPARATOR_EXPECTED_AFTER_FALSE,      //!< syntax error on 'false'
   JST_ERR_SEPARATOR_EXPECTED_AFTER_NULL,       //!< syntax error on 'null'
   JST_ERR_TRUE_EXPECTED,                       //!< 'true' token was expected
   JST_ERR_FALSE_EXPECTED,                      //!< 'false' token was expected
   JST_ERR_NULL_EXPECTED,                       //!< 'null' token was expected
   JST_ERR_NUMBER_EXPECTED,                     //!< token is not a number
   JST_ERR_COLON_EXPECTED,                      //!< token is not a colon
   JST_ERR_UNEXPECTED_TOKEN,                    //!< unexpected token
   JST_ERR_PATH_SYNTAX,                         //!< Bad JSON path syntax
   JST_ERR_NOT_FOUND,                           //!< JST_get() has no result
   JST_ERR_ARRAY_INDEX_OUT_OF_RANGE,            //!< index is out of range
   JST_ERR_INTERRUPTED,                         //!< JST_Visitor has returned false to interrupt the tree's iteration
} JST_Error;

/**
 * Visitor in depth first with 'pre' set to true then backtrack with 'pre' set to false.
 */
typedef bool (* JST_Visitor )( const char * name, unsigned index, bool pre, const JST_Element * element, void * user_context );

/**
 * Information returned by JST_load() in case of error.
 */
typedef struct {
   unsigned line;        //!< Last line's index (from 1)
   char     context[80]; //!< Characters around the error
   unsigned pos;         //!< Position of error in context
} JST_SyntaxError;

/**
 * Constant defined to initialize safely a variable of type JST_SyntaxError.
 */
extern const JST_SyntaxError JST_SyntaxError_Zero;

/**
 * Reads the input file, allocates the node of the corresponding tree, than frees the input buffer.
 * @param filepath the file path
 * @param root the root of the JSON tree
 * @param error this structure, in case of error, is filled with information of its nature, its context and its origin
 * @return JST_ERR_NONE on success, JST_ERR_NULL_ARGUMENT, JST_ERR_ERRNO or any syntax error otherwise.
 */
JST_Error JST_load_from_file( const char * filepath, JST_Element * root, JST_SyntaxError * error );

/**
 * Reads the input stream, allocates the node of the corresponding tree, than frees the input buffer.
 * @param stream the character stream to read
 * @param root the root of the JSON tree
 * @param error this structure, in case of error, is filled with information of its nature, its context and its origin
 * @return JST_ERR_NONE on success, JST_ERR_NULL_ARGUMENT, JST_ERR_ERRNO or any syntax error otherwise.
 */
JST_Error JST_load_from_stream( FILE * stream, JST_Element * root, JST_SyntaxError * error );

/**
 * Save the JSON tree to a stream, pretty-printed.
 * @param stream the target stream.
 * @param root the root of the tree to save.
 * @param indent the amout of space to pretty print the tree.
 * @return JST_ERR_NONE on success, JST_ERR_NULL_ARGUMENT, JST_ERR_ERRNO or JST_ERR_NULL_TYPE otherwise.
 */
JST_Error JST_save_to_stream( FILE * stream, const JST_Element * root, unsigned indent );

/**
 * Save the JSON tree to a file, pretty-printed.
 * @param filepath the path of the target file
 * @param root     the root of the tree to save
 * @param indent   the amout of space to pretty print the tree.
 * @return JST_ERR_NONE on success, JST_ERR_NULL_ARGUMENT, JST_ERR_ERRNO or JST_ERR_NULL_TYPE otherwise.
 */
JST_Error JST_save_to_file( const char * filepath, const JST_Element * root, unsigned indent );

/**
 * Save the JSON tree to an XML stream, pretty-printed.
 * @param stream the target stream.
 * @param root the root of the tree to save.
 * @param indent the amout of space to pretty print the tree.
 * @return JST_ERR_NONE on success, JST_ERR_NULL_ARGUMENT, JST_ERR_ERRNO or JST_ERR_NULL_TYPE otherwise.
 */
JST_Error JST_save_to_xml_stream( FILE * stream, const JST_Element * root, unsigned indent );

/**
 * Save the JSON tree to an XML file, pretty-printed.
 * @param filepath the path of the target file
 * @param root     the root of the tree to save
 * @param indent   the amout of space to pretty print the tree.
 * @return JST_ERR_NONE on success, JST_ERR_NULL_ARGUMENT, JST_ERR_ERRNO or JST_ERR_NULL_TYPE otherwise.
 */
JST_Error JST_save_to_xml_file( const char * filepath, const JST_Element * root, unsigned indent );

/**
 * Save the JSON tree to a stream, as a single line of text, without whitespaces.
 * @param stream the path of the target file
 * @param root   the root of the tree to save
 * @return JST_ERR_NONE on success, JST_ERR_NULL_ARGUMENT, JST_ERR_ERRNO or JST_ERR_NULL_TYPE otherwise.
 */
JST_Error JST_save_to_stream_compact( FILE * stream, const JST_Element * root );

/**
 * Save the JSON tree to a file, as a single line of text, without whitespaces.
 * @param filepath the path of the target file
 * @param root     the root of the tree to save
 * @return JST_ERR_NONE on success, JST_ERR_NULL_ARGUMENT, JST_ERR_ERRNO or JST_ERR_NULL_TYPE otherwise.
 */
JST_Error JST_save_to_file_compact( const char * filepath, const JST_Element * root );

/**
 * Serialize the JSON tree to a string, pretty-printed.
 * @param root   the root of the tree to save
 * @param dest   a pointer to the char buffer allocated by this function, must be freed by the user
 * @param indent the amout of space to pretty print the tree.
 * @return JST_ERR_NONE on success, JST_ERR_NULL_ARGUMENT, JST_ERR_ERRNO or JST_ERR_NULL_TYPE otherwise.
 */
JST_Error JST_serialize( JST_Element * root, char ** dest, unsigned indent );

/**
 * Serialize the JSON tree to an XML string, pretty-printed.
 * @param root   the root of the tree to save
 * @param dest   a pointer to the char buffer allocated by this function, must be freed by the user
 * @param indent the amout of space to pretty print the tree.
 * @return JST_ERR_NONE on success, JST_ERR_NULL_ARGUMENT, JST_ERR_ERRNO or JST_ERR_NULL_TYPE otherwise.
 */
JST_Error JST_serialize_xml( JST_Element * root, char ** dest, unsigned indent );

/**
 * Serialize the JSON tree to a string as a single line of text, without whitespaces.
 * @param root the root of the tree to save
 * @param dest a pointer to the char buffer allocated by this function, must be freed by the user
 * @return JST_ERR_NONE on success, JST_ERR_NULL_ARGUMENT, JST_ERR_ERRNO or JST_ERR_NULL_TYPE otherwise.
 */
JST_Error JST_serialize_compact( JST_Element * root, char ** dest );

/**
 * Search an element by its path.
 * @param jsonpath syntax is (&lt;name>|[&lt;integer>])(.&lt;name>|[&lt;integer>])*
 * @param root     the root of the search
 * @param dest     an address of a pointer to the element found
 * @return an error status, JST_ERR_NOT_FOUND when jsonpath doesn't match.
 */
JST_Error JST_get( const char * jsonpath, JST_Element * root, JST_Element ** dest );

/**
 * Add a named-value pair to an object
 * @param object the target object to modify
 * @param index  the property's rank, -1U to add at the end
 * @param name   the property's name
 * @param value  the property's value
 * @return JST_ERR_NONE on success, JST_ERR_NULL_TYPE or JST_ERR_ERRNO on failure.
 */
JST_Error JST_add_property( JST_Object * object, unsigned index, const char * name, const JST_Element * value );

/**
 * Remove a named-value pair from an object
 * @param object the target object to modify
 * @param name   the property's name
 * @return JST_ERR_NONE on success, JST_ERR_NOT_FOUND, JST_ERR_NULL_ARGUMENT on failure.
 */
JST_Error JST_remove_property( JST_Object * object, const char * name );

/**
 * Add or insert an item to an array.
 * @param array the array to update
 * @param value the new value to add
 * @param index the index of the value in the array, -1U means "last"
 * @return JST_ERR_NONE on success, JST_ERR_NOT_FOUND, JST_ERR_NULL_ARGUMENT on failure.
 */
JST_Error JST_add_item( JST_Array * array, unsigned index, const JST_Element * value );

/**
 * Remove an item from an array.
 * @param array the array to update
 * @param index the index of the item to remove
 * @return JST_ERR_NONE on success, JST_ERR_NOT_FOUND, JST_ERR_NULL_ARGUMENT on failure.
 */
JST_Error JST_remove_item( JST_Array * array, unsigned index );

/**
 * Remove an item from an array.
 * @param array the array to update
 * @param index the index of the item to replace
 * @param value the new value to set in place
 * @return JST_ERR_NONE on success, JST_ERR_NOT_FOUND, JST_ERR_NULL_ARGUMENT on failure.
 */
JST_Error JST_replace_item( JST_Array * array, unsigned index, const JST_Element * value );

/**
 * Traverse the tree, in depth first.
 * @param root    the tree to traverse.
 * @param visitor a user defined function called twice for each node
 * @param context a used defined context.
 */
JST_Error JST_walk( const JST_Element * root, JST_Visitor visitor, void * context );

/**
 * Utility function used to deeply delete sub-trees.
 * @param pair the root to free.
 * @return JST_ERR_NONE on success, JST_ERR_NULL_ARGUMENT on failure.
 */
JST_Error JST_delete_pair( JST_Pair * pair );

/**
 * Utility function used to deeply delete sub-trees.
 * @param object the root to free.
 * @return JST_ERR_NONE on success, JST_ERR_NULL_ARGUMENT on failure.
 */
JST_Error JST_delete_object( JST_Object * object );

/**
 * Utility function used to deeply delete sub-trees.
 * @param item the root to free.
 * @return JST_ERR_NONE on success, JST_ERR_NULL_ARGUMENT on failure.
 */
JST_Error JST_delete_array_item( JST_ArrayItem * item );

/**
 * Utility function used to deeply delete sub-trees.
 * @param array the root to free.
 * @return JST_ERR_NONE on success, JST_ERR_NULL_ARGUMENT on failure.
 */
JST_Error JST_delete_array( JST_Array * array );

/**
 * Utility function used to deeply delete sub-trees.
 * @param element the root to free.
 * @return JST_ERR_NONE on success, JST_ERR_NULL_ARGUMENT on failure.
 */
JST_Error JST_delete_element( JST_Element * element );
