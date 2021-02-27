#pragma once

/**
 * Compares two JST_Pair's name according to qsort()/bsearch() signature.
 * @param left the first JST_Pair to compare
 * @param right the second JST_Pair to compare
 * @return a value less than, equals to or greater than zero when
 * left->name < right->name, left->name = right->name and left->name > right->name
 * respectively.
 */
int JST_pairs_compare( const void * left, const void * right );
