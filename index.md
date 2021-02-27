# JSON parser in C #

## Features ##

* Tree oriented: each node is strongly typed
* Dynamically allocated: like a DOM XML parser, this parser reads all the data in memory,
  chunks allocated by `calloc` and `strdup`.
* Reading a file or a character buffer
* Getting a JST_Element by its path
* Creates, read, updates or deletes a property of an object or an item of an array.
* Documentation built with [Doxygen](https://www.doxygen.nl/index.html)

## How to build ##

* download [jsontools-1.0.0.tar.gz](https://github.com/AubinMahe/jsontools/jsontools-1.0.0.tar.gz)
* tar xzf jsontools-1.0.0.tar.gz
* cd jsontools-1.0.0
* ./configure
* make
* make install

## API ##

See [documentation](https://aubinmahe.github.io/jsontools/html/index.html)

## Sample code ##

This sample code:
* read the json file ["data/object.json"](https://github.com/AubinMahe/jsontools/data/object.json)
* search an element in the loaded tree via the path `"repository.examples[2].subs[3].value"`
* print the value found
* serialize as text the tree and print it
* free the allocated resources

```
#include "jstools.h"

#include <stdio.h>
#include <stdlib.h>

int main( void ) {
   const char *    json_filename = "data/object.json";
   JST_Element     root          = JST_Element_Zero;
   JST_SyntaxError syntax_error  = JST_SyntaxError_Zero;
   JST_Error       err           = JST_load_from_file( json_filename, &root, &syntax_error );
   if( err != JST_ERR_NONE ) {
      return err;
   }
   const char *  json_path = "repository.examples[2].subs[3].value";
   JST_Element * elt       = NULL;
   if(  ( JST_get( json_path, &root, &elt ) == JST_ERR_NONE )
      &&( elt != NULL )
      &&( elt->type == JST_INTEGER ))
   {
      printf( "%s = %ld\n", json_path, elt->value.integer );
   }
   char * json_text = NULL;
   if( JST_serialize( &root, &json_text, 2 ) == JST_ERR_NONE ) {
      printf( "%s\n", json_text );
      free( json_text );
   }
   JST_delete_element( &root );
   return 0;
}
```
