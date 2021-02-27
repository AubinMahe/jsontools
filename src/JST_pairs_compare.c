#include "JST_pairs_compare.h"
#include "jstools.h"

#include <string.h>

int JST_pairs_compare( const void * left, const void * right ) {
   const JST_Pair * const * lpp = left;
   const JST_Pair * const * rpp = right;
   const JST_Pair *         lp  = *lpp;
   const JST_Pair *         rp  = *rpp;
   const char *             ln  = lp->name;
   const char *             rn  = rp->name;
   return strcmp( ln, rn );
}
