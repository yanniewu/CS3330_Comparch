#include <stdio.h>
#include <stdlib.h>
#include "defs.h"
#include <immintrin.h>

/* 
 * Please fill in the following team struct 
 */
who_t who = {
    "chicken",           /* Scoreboard name */

    "Siddharth Tickle",      /* First member full name */
    "st3ehn@virginia.edu",     /* First member email address */
};

/*** UTILITY FUNCTIONS ***/

/* You are free to use these utility functions, or write your own versions
 * of them. */

/* A struct used to compute averaged pixel value */
typedef struct {
    unsigned short red;
    unsigned short green;
    unsigned short blue;
    unsigned short alpha;
    unsigned short num;
} pixel_sum;

/* Compute min and max of two integers, respectively */
static int min(int a, int b) { return (a < b ? a : b); }
static int max(int a, int b) { return (a > b ? a : b); }

/* 
 * initialize_pixel_sum - Initializes all fields of sum to 0 
 */
static void initialize_pixel_sum(pixel_sum *sum) 
{
    sum->red = sum->green = sum->blue = sum->alpha = 0;
    sum->num = 0;
    return;
}

/* 
 * accumulate_sum - Accumulates field values of p in corresponding 
 * fields of sum 
 */
static void accumulate_sum(pixel_sum *sum, pixel p) 
{
    sum->red += (int) p.red;
    sum->green += (int) p.green;
    sum->blue += (int) p.blue;
    sum->alpha += (int) p.alpha;
    sum->num++;
    return;
}

/* 
 * assign_sum_to_pixel - Computes averaged pixel value in current_pixel 
 */
static void assign_sum_to_pixel(pixel *current_pixel, pixel_sum sum) 
{
    current_pixel->red = (unsigned short) (sum.red/sum.num);
    current_pixel->green = (unsigned short) (sum.green/sum.num);
    current_pixel->blue = (unsigned short) (sum.blue/sum.num);
    current_pixel->alpha = (unsigned short) (sum.alpha/sum.num);
    return;
}

/* 
 * avg - Returns averaged pixel value at (i,j) 
 */
static pixel avg(int dim, int i, int j, pixel *src) {
    pixel_sum sum;
    pixel current_pixel;

    initialize_pixel_sum(&sum);
    for(int jj=max(j-1, 0); jj <= min(j+1, dim-1); jj++) 
        for(int ii=max(i-1, 0); ii <= min(i+1, dim-1); ii++) 
            accumulate_sum(&sum, src[RIDX(ii,jj,dim)]);

    assign_sum_to_pixel(&current_pixel, sum);
 
    return current_pixel;
}

static __m256i avg_middle(int dim, int i, int j, pixel *src) {
    // pixel_sum sum;
    // pixel current_pixel;

    __m256i the_pixel;

    __m256i partial_sums = _mm256_setzero_si256();
    __m256i partial_sum1 = _mm256_setzero_si256();
    __m256i partial_sum2 = _mm256_setzero_si256();
    __m256i partial_sum3 = _mm256_setzero_si256();

    // initialize_pixel_sum(&sum);
    // (i-1, j-1)
    // accumulate_sum(&sum, src[RIDX(i-1,j-1,dim)]);

    the_pixel = _mm256_cvtepu8_epi16(_mm_loadu_si128((__m128i*) &src[RIDX(i-1, j-1, dim)]));
    partial_sum1 = _mm256_add_epi16(partial_sum1, the_pixel);

    // (i-1, j)
    // accumulate_sum(&sum, src[RIDX(i-1,j,dim)]);

    the_pixel = _mm256_cvtepu8_epi16(_mm_loadu_si128((__m128i*) &src[RIDX(i-1, j, dim)]));
    partial_sum1 = _mm256_add_epi16(partial_sum1, the_pixel);

    // (i-1, j+1)
    // accumulate_sum(&sum, src[RIDX(i-1,j+1,dim)]);

    the_pixel = _mm256_cvtepu8_epi16(_mm_loadu_si128((__m128i*) &src[RIDX(i-1, j+1, dim)]));
    partial_sum1 = _mm256_add_epi16(partial_sum1, the_pixel);

    // (i, j-1)
    // accumulate_sum(&sum, src[RIDX(i,j-1,dim)]);

    the_pixel = _mm256_cvtepu8_epi16(_mm_loadu_si128((__m128i*) &src[RIDX(i, j-1, dim)]));
    partial_sum2 = _mm256_add_epi16(partial_sum2, the_pixel);

    // (i,j)
    // accumulate_sum(&sum, src[RIDX(i,j,dim)]);

    the_pixel = _mm256_cvtepu8_epi16(_mm_loadu_si128((__m128i*) &src[RIDX(i, j, dim)]));
    partial_sum2 = _mm256_add_epi16(partial_sum2, the_pixel);

    // (i, j+1)
    // accumulate_sum(&sum, src[RIDX(i,j+1,dim)]);

    the_pixel = _mm256_cvtepu8_epi16(_mm_loadu_si128((__m128i*) &src[RIDX(i, j+1, dim)]));
    partial_sum2 = _mm256_add_epi16(partial_sum2, the_pixel);

    // (i+1, j-1)
    // accumulate_sum(&sum, src[RIDX(i+1,j-1,dim)]);

    the_pixel = _mm256_cvtepu8_epi16(_mm_loadu_si128((__m128i*) &src[RIDX(i+1, j-1, dim)]));
    partial_sum3 = _mm256_add_epi16(partial_sum3, the_pixel);

    // (i+1, j)
    // accumulate_sum(&sum, src[RIDX(i+1,j,dim)]);

    the_pixel = _mm256_cvtepu8_epi16(_mm_loadu_si128((__m128i*) &src[RIDX(i+1, j, dim)]));
    partial_sum3 = _mm256_add_epi16(partial_sum3, the_pixel);

    // (i+1, j+1)
    // accumulate_sum(&sum, src[RIDX(i+1,j+1,dim)]);

    the_pixel = _mm256_cvtepu8_epi16(_mm_loadu_si128((__m128i*) &src[RIDX(i+1, j+1, dim)]));
    partial_sum3 = _mm256_add_epi16(partial_sum3, the_pixel);

    // current_pixel.red = (unsigned short) (sum.red/9);
    // current_pixel.green = (unsigned short) (sum.green/9);
    // current_pixel.blue = (unsigned short) (sum.blue/9);
    // current_pixel.alpha = (unsigned short) (sum.alpha/9);

    partial_sums = _mm256_add_epi16(partial_sum1, _mm256_add_epi16(partial_sum2, partial_sum3));

    __m256i mults = _mm256_set1_epi16(7282);

    partial_sums = _mm256_mulhi_epi16(partial_sums, mults);

    return partial_sums;

    // unsigned short pixel_elements[8];
    // // _mm256_storeu_si256((__m256i*) pixel_elements, (partial_sums));

    // _mm_storeu_si128((__m128i*) pixel_elements, _mm256_extracti128_si256(partial_sums, 0));


    // current_pixel.red = (unsigned short) (pixel_elements[0]);
    // current_pixel.green = (unsigned short) (pixel_elements[1]);
    // current_pixel.blue = (unsigned short) (pixel_elements[2]);
    // current_pixel.alpha = (unsigned short) (pixel_elements[3]);

    // return current_pixel;
}

static pixel avg_corner(int dim, int i, int j, pixel *src) {
    pixel_sum sum;
    pixel current_pixel;

    initialize_pixel_sum(&sum);

    // corner 0
    if (i == 0 && j == 0) {
        // (i,j) *
        accumulate_sum(&sum, src[RIDX(i,j,dim)]);
        // (i, j+1) *
        accumulate_sum(&sum, src[RIDX(i,j+1,dim)]);
        // (i+1, j) *
        accumulate_sum(&sum, src[RIDX(i+1,j,dim)]);
        // (i+1, j+1) *
        accumulate_sum(&sum, src[RIDX(i+1,j+1,dim)]);
    }
    // corner 1
    else if (i == 0 && j == dim-1) {
        // (i,j) *
        accumulate_sum(&sum, src[RIDX(i,j,dim)]);
        // (i, j-1) *
        accumulate_sum(&sum, src[RIDX(i,j-1,dim)]);
        // (i-1, j-1) *
        accumulate_sum(&sum, src[RIDX(i+1,j,dim)]);
        // (i-1, j) *
        accumulate_sum(&sum, src[RIDX(i+1,j-1,dim)]);
    }
    // corner 2
    else if (i == dim-1 && j == 0) {
        // (i,j) *
        accumulate_sum(&sum, src[RIDX(i,j,dim)]);
        // (i, j+1) *
        accumulate_sum(&sum, src[RIDX(i,j+1,dim)]);
        // (i-1, j) *
        accumulate_sum(&sum, src[RIDX(i-1,j,dim)]);
        // (i-1, j+1) *
        accumulate_sum(&sum, src[RIDX(i-1,j+1,dim)]);
    }
    // corner 3
    else if (i == dim-1 && j == dim-1) {
        // (i,j) *
        accumulate_sum(&sum, src[RIDX(i,j,dim)]);
        // (i, j-1) *
        accumulate_sum(&sum, src[RIDX(i,j-1,dim)]);
        // (i-1, j) *
        accumulate_sum(&sum, src[RIDX(i-1,j,dim)]);
        // (i-1, j-1) *
        accumulate_sum(&sum, src[RIDX(i-1,j-1,dim)]);
    }

    current_pixel.red = (unsigned short) (sum.red/4);
    current_pixel.green = (unsigned short) (sum.green/4);
    current_pixel.blue = (unsigned short) (sum.blue/4);
    current_pixel.alpha = (unsigned short) (sum.alpha/4);

    return current_pixel;
}

static pixel avg_edge(int dim, int i, int j, pixel *src) {
    pixel_sum sum;
    pixel current_pixel;

    initialize_pixel_sum(&sum);

    // top edge
    if(i == 0) {   
        accumulate_sum(&sum, src[RIDX(i,j,dim)]);
        accumulate_sum(&sum, src[RIDX(i,j+1,dim)]);
        accumulate_sum(&sum, src[RIDX(i,j-1,dim)]);
        accumulate_sum(&sum, src[RIDX(i+1,j-1,dim)]);
        accumulate_sum(&sum, src[RIDX(i+1,j,dim)]);
        accumulate_sum(&sum, src[RIDX(i+1,j+1,dim)]);
    }
    // bottom edge
    else if(i == dim-1) {   
        accumulate_sum(&sum, src[RIDX(i,j,dim)]);
        accumulate_sum(&sum, src[RIDX(i,j+1,dim)]);
        accumulate_sum(&sum, src[RIDX(i,j-1,dim)]);
        accumulate_sum(&sum, src[RIDX(i-1,j-1,dim)]);
        accumulate_sum(&sum, src[RIDX(i-1,j,dim)]);
        accumulate_sum(&sum, src[RIDX(i-1,j+1,dim)]);
    }
    // left edge
    else if(j == 0) {   
        accumulate_sum(&sum, src[RIDX(i,j,dim)]);
        accumulate_sum(&sum, src[RIDX(i-1,j,dim)]);
        accumulate_sum(&sum, src[RIDX(i+1,j,dim)]);
        accumulate_sum(&sum, src[RIDX(i-1,j+1,dim)]);
        accumulate_sum(&sum, src[RIDX(i,j+1,dim)]);
        accumulate_sum(&sum, src[RIDX(i+1,j+1,dim)]);
    }

    // right edge
    else if(j == dim-1) {   
        accumulate_sum(&sum, src[RIDX(i,j,dim)]);
        accumulate_sum(&sum, src[RIDX(i-1,j,dim)]);
        accumulate_sum(&sum, src[RIDX(i+1,j,dim)]);
        accumulate_sum(&sum, src[RIDX(i-1,j-1,dim)]);
        accumulate_sum(&sum, src[RIDX(i,j-1,dim)]);
        accumulate_sum(&sum, src[RIDX(i+1,j-1,dim)]);
    }

    current_pixel.red = (unsigned short) (sum.red/6);
    current_pixel.green = (unsigned short) (sum.green/6);
    current_pixel.blue = (unsigned short) (sum.blue/6);
    current_pixel.alpha = (unsigned short) (sum.alpha/6);

    return current_pixel;
}



/******************************************************
 * Your different versions of the smooth go here
 ******************************************************/

/* 
 * naive_smooth - The naive baseline version of smooth
 */
char naive_smooth_descr[] = "naive_smooth: Naive baseline implementation";
void naive_smooth(int dim, pixel *src, pixel *dst) 
{
    for (int i = 0; i < dim; i++)
	for (int j = 0; j < dim; j++)
            dst[RIDX(i,j, dim)] = avg(dim, i, j, src);
}
/* 
 * smooth - Your current working version of smooth
 *          Our supplied version simply calls naive_smooth
 */
char another_smooth_descr[] = "another_smooth: Another version of smooth";
void another_smooth(int dim, pixel *src, pixel *dst) 
{
    // first for inner pixels
    for (int i = 1; i < dim-1; i++) {
	    for (int j = 1; j < dim-1; j+=2) {
            unsigned short pixel_elements[8];
            pixel current_pixel;
            _mm_storeu_si128((__m128i*) pixel_elements, _mm256_extracti128_si256(avg_middle(dim, i, j, src), 0));
            
            current_pixel.red = (unsigned short) (pixel_elements[0]);
            current_pixel.green = (unsigned short) (pixel_elements[1]);
            current_pixel.blue = (unsigned short) (pixel_elements[2]);
            current_pixel.alpha = (unsigned short) (pixel_elements[3]);

            dst[RIDX(i,j, dim)] = current_pixel;

            current_pixel.red = (unsigned short) (pixel_elements[4]);
            current_pixel.green = (unsigned short) (pixel_elements[5]);
            current_pixel.blue = (unsigned short) (pixel_elements[6]);
            current_pixel.alpha = (unsigned short) (pixel_elements[7]);

            dst[RIDX(i,j+1, dim)] = current_pixel;
        }
    }

    // corners
    dst[RIDX(0,0, dim)] = avg_corner(dim, 0, 0, src);
    dst[RIDX(0,dim-1, dim)] = avg_corner(dim, 0, dim-1, src);
    dst[RIDX(dim-1,0, dim)] = avg_corner(dim, dim-1, 0, src);
    dst[RIDX(dim-1,dim-1, dim)] = avg_corner(dim, dim-1, dim-1, src);

    for (int k = 1; k < dim-1; k++) {
        // top edge
        dst[RIDX(0,k, dim)] = avg_edge(dim, 0, k, src);
        // bottom edge
        dst[RIDX(dim-1,k, dim)] = avg_edge(dim, dim-1, k, src);
        // left edge
        dst[RIDX(k,0, dim)] = avg_edge(dim, k, 0, src);
        // right edge
        dst[RIDX(k,dim-1, dim)] = avg_edge(dim, k, dim-1, src);
    }   
}

// char sep_smooth_descr[] = "sep_smooth: separated by corner, edge, middle";
// void sep_smooth(int dim, pixel *src, pixel *dst) 
// {
//     // first for inner pixels
//     for (int i = 1; i < dim-1; i++)
// 	for (int j = 1; j < dim-1; j++)
//             dst[RIDX(i,j, dim)] = avg_middle(dim, i, j, src);

//     // corners
//     dst[RIDX(0,0, dim)] = avg_corner(dim, 0, 0, src);
//     dst[RIDX(0,dim-1, dim)] = avg_corner(dim, 0, dim-1, src);
//     dst[RIDX(dim-1,0, dim)] = avg_corner(dim, dim-1, 0, src);
//     dst[RIDX(dim-1,dim-1, dim)] = avg_corner(dim, dim-1, dim-1, src);
        
//     // top edge
//     for (int k = 1; k < dim-1; k++) {
//         dst[RIDX(0,k, dim)] = avg(dim, 0, k, src);
//     }    
    
//     // bottom edge
//     for (int k = 1; k < dim-1; k++) {
//         dst[RIDX(dim-1,k, dim)] = avg(dim, dim-1, k, src);
//     }

//     // left edge
//     for (int k = 1; k < dim-1; k++) {
//         dst[RIDX(k,0, dim)] = avg(dim, k, 0, src);
//     }

//     // right edge
//     for (int k = 1; k < dim-1; k++) {
//         dst[RIDX(k, dim-1, dim)] = avg(dim, k, dim-1, src);
//     }

// }


/*********************************************************************
 * register_smooth_functions - Register all of your different versions
 *     of the smooth function by calling the add_smooth_function() for
 *     each test function. When you run the benchmark program, it will
 *     test and report the performance of each registered test
 *     function.  
 *********************************************************************/

void register_smooth_functions() {
    add_smooth_function(&naive_smooth, naive_smooth_descr);
    add_smooth_function(&another_smooth, another_smooth_descr);
    // add_smooth_function(&sep_smooth, sep_smooth_descr);
}
