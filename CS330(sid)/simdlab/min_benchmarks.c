#include <stdlib.h>
#include <limits.h>  /* for USHRT_MAX */

#include <immintrin.h>

#include "min.h"
/* reference implementation in C */
short min_C(long size, short * a) {
    short result = SHRT_MAX;
    for (int i = 0; i < size; ++i) {
        if (a[i] < result)
            result = a[i];
    }
    return result;
}

short min_AVX(long size, short * a) {

    __m256i partial_mins = _mm256_loadu_si256((__m256i*) &a[0]);

    for (int i = 16; i < size; i+=16) {

        // load the same value into a vector
        // __m256i cur_values = _mm256_set1_epi16(a[i]);
        
        __m256i a_values = _mm256_loadu_si256((__m256i*) &a[i]);

        // compare to current mins
        partial_mins = _mm256_min_epi16(a_values, partial_mins);

    }

    unsigned short extracted_partial_mins[16];
    _mm256_storeu_si256((__m256i*) &extracted_partial_mins, partial_mins);

    int min = SHRT_MAX;
    for(int i = 0; i < 16; i++) {
        if(extracted_partial_mins[i] < min)
            min = extracted_partial_mins[i];
    }

    return min;
}



/* This is the list of functions to test */
function_info functions[] = {
    {min_C, "C (local)"},
    // add entries here!
    {min_AVX, "min with AVX"},
    {NULL, NULL},
};
