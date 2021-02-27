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
