#include <stdio.h>
#include <stdlib.h>
#include "defs.h"

/* 
 * Please fill in the following struct with your name and the name you'd like to appear on the scoreboard
 */
who_t who = {
    "chicken",           /* Scoreboard name */

    "Siddharth Tickle",   /* Full name */
    "st3ehn@virginia.edu",  /* Email address */
};

/***************
 * ROTATE KERNEL
 ***************/

/******************************************************
 * Your different versions of the rotate kernel go here
 ******************************************************/

/* 
 * naive_rotate - The naive baseline version of rotate 
 */
char naive_rotate_descr[] = "naive_rotate: Naive baseline implementation";
void naive_rotate(int dim, pixel *src, pixel *dst) 
{
    for (int i = 0; i < dim; i++)
        for (int j = 0; j < dim; j++)
            dst[RIDX(dim-1-j, i, dim)] = src[RIDX(i, j, dim)];
}
/* 
 * rotate - Your current working version of rotate
 *          Our supplied version simply calls naive_rotate
 */
char another_rotate_descr[] = "another_rotate: Another version of rotate";
void another_rotate(int dim, pixel *src, pixel *dst) 
{
    naive_rotate(dim, src, dst);
}

char inner_4_unrolled_rotate_descr[] = "rotate_inner_unrolled: Unroll inner loop 4 times";
void rotate_inner_4_unrolled(int dim, pixel *src, pixel *dst) 
{
    for (int i = 0; i < dim; i++)
	    for (int j = 0; j+3 < dim; j+=4) {
	        dst[RIDX(dim-1-j, i, dim)] = src[RIDX(i, j, dim)];
	        dst[RIDX(dim-1-(j+1), i, dim)] = src[RIDX(i, j+1, dim)];
	        dst[RIDX(dim-1-(j+2), i, dim)] = src[RIDX(i, j+2, dim)];
	        dst[RIDX(dim-1-(j+3), i, dim)] = src[RIDX(i, j+3, dim)];
        }
}

char blocking_rotate_descr[] = "blocking_rotate_descr: cache blocking";
void rotate_blocking(int dim, pixel *src, pixel *dst) 
{
    int blk = 8;
    for(int ii = 0; ii < dim; ii+=blk)
    for(int jj = 0; jj < dim; jj+=blk)
    for (int i = ii; i < ii+blk; i++)
	    for (int j = jj; j+3 < jj+blk; j+=4) {
	        dst[RIDX(dim-1-j, i, dim)] = src[RIDX(i, j, dim)];
	        dst[RIDX(dim-1-(j+1), i, dim)] = src[RIDX(i, j+1, dim)];
	        dst[RIDX(dim-1-(j+2), i, dim)] = src[RIDX(i, j+2, dim)];
	        dst[RIDX(dim-1-(j+3), i, dim)] = src[RIDX(i, j+3, dim)];
        }
}

char blocking_rotate_nu_descr[] = "blocking_rotate_nu_descr: cache blocking w/ no unrolling";
void rotate_nu_blocking(int dim, pixel *src, pixel *dst) 
{
    int blk = 16;
    for(int ii = 0; ii < dim; ii+=blk)
    for(int jj = 0; jj < dim; jj+=blk)
    for (int i = ii; i < ii+blk; i++)
	    for (int j = jj; j < jj+blk; j++)
	        dst[RIDX(dim-1-j, i, dim)] = src[RIDX(i, j, dim)];
}

char loopinterchange_rotate_descr[] = "loopinterchange_rotate_descr: cache blocking w/ loop interchange & no unrolling";
void rotate_loopinterchange(int dim, pixel *src, pixel *dst) 
{
    int blk = 32;
    for(int jj = 0; jj < dim; jj+=blk)
    for(int ii = 0; ii < dim; ii+=blk)
    for (int j = jj; j < jj+blk; j++)
        for (int i = ii; i < ii+blk; i++)
	        dst[RIDX(dim-1-j, i, dim)] = src[RIDX(i, j, dim)];
}

char loopinterchange_ur_rotate_descr[] = "loopinterchange_ur_rotate_descr: cache blocking w/ loop interchange & unrolling 4 times";
void rotate_ur_loopinterchange(int dim, pixel *src, pixel *dst) 
{
    int blk = 32;
    for(int jj = 0; jj < dim; jj+=blk)
    for(int ii = 0; ii < dim; ii+=blk)
    for (int j = jj; j < jj+blk; j++)
        for (int i = ii; i+3 < ii+blk; i+=4) {
	        dst[RIDX(dim-1-j, i, dim)] = src[RIDX(i, j, dim)];
	        dst[RIDX(dim-1-j, i+1, dim)] = src[RIDX(i+1, j, dim)];
	        dst[RIDX(dim-1-j, i+2, dim)] = src[RIDX(i+2, j, dim)];
	        dst[RIDX(dim-1-j, i+3, dim)] = src[RIDX(i+3, j, dim)];	        

        }
}

char loopinterchange_ur2_rotate_descr[] = "loopinterchange_ur_rotate_descr: cache blocking w/ loop interchange & unrolling 8 times";
void rotate_ur2_loopinterchange(int dim, pixel *src, pixel *dst) 
{
    int blk = 32;
    for(int jj = 0; jj < dim; jj+=blk)
    for(int ii = 0; ii < dim; ii+=blk)
    for (int j = jj; j < jj+blk; j++)
        for (int i = ii; i+7 < ii+blk; i+=8) {
	        dst[RIDX(dim-1-j, i, dim)] = src[RIDX(i, j, dim)];
	        dst[RIDX(dim-1-j, i+1, dim)] = src[RIDX(i+1, j, dim)];
	        dst[RIDX(dim-1-j, i+2, dim)] = src[RIDX(i+2, j, dim)];
	        dst[RIDX(dim-1-j, i+3, dim)] = src[RIDX(i+3, j, dim)];	        
            dst[RIDX(dim-1-j, i+4, dim)] = src[RIDX(i+4, j, dim)];
	        dst[RIDX(dim-1-j, i+5, dim)] = src[RIDX(i+5, j, dim)];
	        dst[RIDX(dim-1-j, i+6, dim)] = src[RIDX(i+6, j, dim)];
	        dst[RIDX(dim-1-j, i+7, dim)] = src[RIDX(i+7, j, dim)];
        }
}
/*********************************************************************
 * register_rotate_functions - Register all of your different versions
 *     of the rotate function by calling the add_rotate_function() for
 *     each test function. When you run the benchmark program, it will
 *     test and report the performance of each registered test
 *     function.  
 *********************************************************************/

void register_rotate_functions() {
    add_rotate_function(&naive_rotate, naive_rotate_descr);
    add_rotate_function(&another_rotate, another_rotate_descr);
    add_rotate_function(&rotate_inner_4_unrolled, inner_4_unrolled_rotate_descr);
    add_rotate_function(&rotate_blocking, blocking_rotate_descr);
    add_rotate_function(&rotate_nu_blocking, blocking_rotate_nu_descr);
    add_rotate_function(&rotate_loopinterchange, loopinterchange_rotate_descr);
    add_rotate_function(&rotate_ur_loopinterchange, loopinterchange_ur_rotate_descr);
    add_rotate_function(&rotate_ur2_loopinterchange, loopinterchange_ur2_rotate_descr);
}
