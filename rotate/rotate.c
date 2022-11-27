#include <stdio.h>
#include <stdlib.h>
#include "defs.h"

/*
 * Please fill in the following struct with your name and the name you'd like to appear on the scoreboard
 */
who_t who = {
    "anw5ph", /* Scoreboard name */

    "Alexander Williams",  /* Full name */
    "anw5ph@virginia.edu", /* Email address */
};

/***************
 * ROTATE KERNEL
 ***************/

/******************************************************
 * Your different versions of the rotate kernel go here
 ******************************************************/
char rotate_outer_loop_unrolled_16_times_descr[] = "rotate_outer_loop_unrolled_16_times: My version of rotate using loop unrolling";
void rotate_outer_loop_unrolled_16_times(int dim, pixel *src, pixel *dst)
{
    for (int i = 0; i < dim; i += 16)
    {
        for (int j = 0; j < dim; j++)
        {
            dst[RIDX(dim - 1 - j, i, dim)] = src[RIDX(i, j, dim)];
            dst[RIDX(dim - 1 - j, i + 1, dim)] = src[RIDX(i + 1, j, dim)];
            dst[RIDX(dim - 1 - j, i + 2, dim)] = src[RIDX(i + 2, j, dim)];
            dst[RIDX(dim - 1 - j, i + 3, dim)] = src[RIDX(i + 3, j, dim)];
            dst[RIDX(dim - 1 - j, i + 4, dim)] = src[RIDX(i + 4, j, dim)];
            dst[RIDX(dim - 1 - j, i + 5, dim)] = src[RIDX(i + 5, j, dim)];
            dst[RIDX(dim - 1 - j, i + 6, dim)] = src[RIDX(i + 6, j, dim)];
            dst[RIDX(dim - 1 - j, i + 7, dim)] = src[RIDX(i + 7, j, dim)];
            dst[RIDX(dim - 1 - j, i + 8, dim)] = src[RIDX(i + 8, j, dim)];
            dst[RIDX(dim - 1 - j, i + 9, dim)] = src[RIDX(i + 9, j, dim)];
            dst[RIDX(dim - 1 - j, i + 10, dim)] = src[RIDX(i + 10, j, dim)];
            dst[RIDX(dim - 1 - j, i + 11, dim)] = src[RIDX(i + 11, j, dim)];
            dst[RIDX(dim - 1 - j, i + 12, dim)] = src[RIDX(i + 12, j, dim)];
            dst[RIDX(dim - 1 - j, i + 13, dim)] = src[RIDX(i + 13, j, dim)];
            dst[RIDX(dim - 1 - j, i + 14, dim)] = src[RIDX(i + 14, j, dim)];
            dst[RIDX(dim - 1 - j, i + 15, dim)] = src[RIDX(i + 15, j, dim)];
        }
    }
}

/*
 * naive_rotate - The naive baseline version of rotate
 */
char naive_rotate_descr[] = "naive_rotate: Naive baseline implementation";
void naive_rotate(int dim, pixel *src, pixel *dst)
{
    for (int i = 0; i < dim; i++)
        for (int j = 0; j < dim; j++)
            dst[RIDX(dim - 1 - j, i, dim)] = src[RIDX(i, j, dim)];
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

/*********************************************************************
 * register_rotate_functions - Register all of your different versions
 *     of the rotate function by calling the add_rotate_function() for
 *     each test function. When you run the benchmark program, it will
 *     test and report the performance of each registered test
 *     function.
 *********************************************************************/

void register_rotate_functions()
{
    add_rotate_function(&naive_rotate, naive_rotate_descr);
    add_rotate_function(&another_rotate, another_rotate_descr);
    add_rotate_function(&rotate_outer_loop_unrolled_16_times, rotate_outer_loop_unrolled_16_times_descr);
}
