#include <stdio.h>
#include <stdlib.h>
#include "defs.h"
#include <immintrin.h>

/*
 * Please fill in the following team struct
 */
who_t who = {
    "anw5ph", /* Scoreboard name */

    "Alexander Williams",  /* First member full name */
    "anw5ph@virginia.edu", /* First member email address */
};

/*** UTILITY FUNCTIONS ***/

/* You are free to use these utility functions, or write your own versions
 * of them. */

/* A struct used to compute averaged pixel value */
typedef struct
{
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
    sum->red += (int)p.red;
    sum->green += (int)p.green;
    sum->blue += (int)p.blue;
    sum->alpha += (int)p.alpha;
    sum->num++;
    return;
}

/*
 * assign_sum_to_pixel - Computes averaged pixel value in current_pixel
 */
static void assign_sum_to_pixel(pixel *current_pixel, pixel_sum sum)
{
    current_pixel->red = (unsigned short)(sum.red / sum.num);
    current_pixel->green = (unsigned short)(sum.green / sum.num);
    current_pixel->blue = (unsigned short)(sum.blue / sum.num);
    current_pixel->alpha = (unsigned short)(sum.alpha / sum.num);
    return;
}

static void assign_sum_to_pixel_edge(pixel *current_pixel, pixel_sum sum)
{
    current_pixel->red = (unsigned short)(sum.red / 6);
    current_pixel->green = (unsigned short)(sum.green / 6);
    current_pixel->blue = (unsigned short)(sum.blue / 6);
    current_pixel->alpha = (unsigned short)(sum.alpha / 6);
    return;
}

static void assign_sum_to_pixel_corner(pixel *current_pixel, pixel_sum sum)
{
    current_pixel->red = (unsigned short)(sum.red / 4);
    current_pixel->green = (unsigned short)(sum.green / 4);
    current_pixel->blue = (unsigned short)(sum.blue / 4);
    current_pixel->alpha = (unsigned short)(sum.alpha / 4);
    return;
}

// static void assign_sum_to_pixel_norm(pixel *current_pixel, unsigned short pixel_elements[])
// {
//     current_pixel->red = (unsigned short)(pixel_elements[0] / 9);
//     current_pixel->green = (unsigned short)(pixel_elements[1] / 9);
//     current_pixel->blue = (unsigned short)(pixel_elements[2] / 9);
//     current_pixel->alpha = (unsigned short)(pixel_elements[3] / 9);
//     return;
// }

/*
 * avg - Returns averaged pixel value at (i,j)
 */
static pixel avg(int dim, int i, int j, pixel *src)
{
    pixel_sum sum;
    pixel current_pixel;

    initialize_pixel_sum(&sum);
    for (int jj = max(j - 1, 0); jj <= min(j + 1, dim - 1); jj++)
        for (int ii = max(i - 1, 0); ii <= min(i + 1, dim - 1); ii++)
            accumulate_sum(&sum, src[RIDX(ii, jj, dim)]);

    assign_sum_to_pixel(&current_pixel, sum);

    return current_pixel;
}

// static pixel avgTop(int dim, int i, pixel *src)
// {

//     pixel_sum sum;
//     pixel current_pixel;

//     initialize_pixel_sum(&sum);

//     accumulate_sum(&sum, src[RIDX(i, 0, dim)]);
//     accumulate_sum(&sum, src[RIDX(i + 1, 0, dim)]);
//     accumulate_sum(&sum, src[RIDX(i - 1, 0, dim)]);
//     accumulate_sum(&sum, src[RIDX(i, 1, dim)]);
//     accumulate_sum(&sum, src[RIDX(i + 1, 1, dim)]);
//     accumulate_sum(&sum, src[RIDX(i - 1, 1, dim)]);

//     assign_sum_to_pixel_edge(&current_pixel, sum);

//     return current_pixel;
// }

// static pixel avgBottom(int dim, int i, pixel *src)
// {

//     pixel_sum sum;
//     pixel current_pixel;

//     initialize_pixel_sum(&sum);

//     accumulate_sum(&sum, src[RIDX(i, dim - 1, dim)]);
//     accumulate_sum(&sum, src[RIDX(i + 1, dim - 1, dim)]);
//     accumulate_sum(&sum, src[RIDX(i - 1, dim - 1, dim)]);
//     accumulate_sum(&sum, src[RIDX(i, dim - 2, dim)]);
//     accumulate_sum(&sum, src[RIDX(i + 1, dim - 2, dim)]);
//     accumulate_sum(&sum, src[RIDX(i - 1, dim - 2, dim)]);

//     assign_sum_to_pixel_edge(&current_pixel, sum);

//     return current_pixel;
// }

// static pixel avgLeft(int dim, int i, pixel *src)
// {

//     pixel_sum sum;
//     pixel current_pixel;

//     initialize_pixel_sum(&sum);

//     accumulate_sum(&sum, src[RIDX(0, i, dim)]);
//     accumulate_sum(&sum, src[RIDX(0, i + 1, dim)]);
//     accumulate_sum(&sum, src[RIDX(0, i - 1, dim)]);
//     accumulate_sum(&sum, src[RIDX(1, i, dim)]);
//     accumulate_sum(&sum, src[RIDX(1, i + 1, dim)]);
//     accumulate_sum(&sum, src[RIDX(1, i - 1, dim)]);

//     assign_sum_to_pixel_edge(&current_pixel, sum);

//     return current_pixel;
// }

// static pixel avgRight(int dim, int i, pixel *src)
// {

//     pixel_sum sum;
//     pixel current_pixel;

//     initialize_pixel_sum(&sum);

//     accumulate_sum(&sum, src[RIDX(dim - 1, i, dim)]);
//     accumulate_sum(&sum, src[RIDX(dim - 1, i + 1, dim)]);
//     accumulate_sum(&sum, src[RIDX(dim - 1, i - 1, dim)]);
//     accumulate_sum(&sum, src[RIDX(dim - 2, i, dim)]);
//     accumulate_sum(&sum, src[RIDX(dim - 2, i + 1, dim)]);
//     accumulate_sum(&sum, src[RIDX(dim - 2, i - 1, dim)]);

//     assign_sum_to_pixel_edge(&current_pixel, sum);

//     return current_pixel;
// }

// static pixel avgTopLeft(int dim, pixel *src)
// {

//     pixel_sum sum;
//     pixel current_pixel;

//     initialize_pixel_sum(&sum);

//     accumulate_sum(&sum, src[RIDX(0, 0, dim)]);
//     accumulate_sum(&sum, src[RIDX(0, 1, dim)]);
//     accumulate_sum(&sum, src[RIDX(1, 0, dim)]);
//     accumulate_sum(&sum, src[RIDX(1, 1, dim)]);

//     assign_sum_to_pixel_corner(&current_pixel, sum);

//     return current_pixel;
// }

// static pixel avgTopRight(int dim, pixel *src)
// {

//     pixel_sum sum;
//     pixel current_pixel;

//     initialize_pixel_sum(&sum);

//     accumulate_sum(&sum, src[RIDX(dim - 1, 0, dim)]);
//     accumulate_sum(&sum, src[RIDX(dim - 2, 0, dim)]);
//     accumulate_sum(&sum, src[RIDX(dim - 2, 1, dim)]);
//     accumulate_sum(&sum, src[RIDX(dim - 1, 1, dim)]);

//     assign_sum_to_pixel_corner(&current_pixel, sum);

//     return current_pixel;
// }

// static pixel avgBottomLeft(int dim, pixel *src)
// {

//     pixel_sum sum;
//     pixel current_pixel;

//     initialize_pixel_sum(&sum);

//     accumulate_sum(&sum, src[RIDX(0, dim - 1, dim)]);
//     accumulate_sum(&sum, src[RIDX(0, dim - 2, dim)]);
//     accumulate_sum(&sum, src[RIDX(1, dim - 2, dim)]);
//     accumulate_sum(&sum, src[RIDX(1, dim - 1, dim)]);

//     assign_sum_to_pixel_corner(&current_pixel, sum);

//     return current_pixel;
// }

// static pixel avgBottomRight(int dim, pixel *src)
// {

//     pixel_sum sum;
//     pixel current_pixel;

//     initialize_pixel_sum(&sum);

//     accumulate_sum(&sum, src[RIDX(dim - 1, dim - 1, dim)]);
//     accumulate_sum(&sum, src[RIDX(dim - 2, dim - 1, dim)]);
//     accumulate_sum(&sum, src[RIDX(dim - 1, dim - 2, dim)]);
//     accumulate_sum(&sum, src[RIDX(dim - 2, dim - 2, dim)]);

//     assign_sum_to_pixel_corner(&current_pixel, sum);

//     return current_pixel;
// }

// static pixel avgNorm(int dim, int i, int j, pixel *src)
// {
//     pixel_sum sum;
//     pixel current_pixel;

//     initialize_pixel_sum(&sum);

//     accumulate_sum(&sum, src[RIDX(i, j, dim)]);
//     accumulate_sum(&sum, src[RIDX(i - 1, j, dim)]);
//     accumulate_sum(&sum, src[RIDX(i + 1, j, dim)]);
//     accumulate_sum(&sum, src[RIDX(i, j - 1, dim)]);
//     accumulate_sum(&sum, src[RIDX(i, j + 1, dim)]);
//     accumulate_sum(&sum, src[RIDX(i + 1, j + 1, dim)]);
//     accumulate_sum(&sum, src[RIDX(i + 1, j - 1, dim)]);
//     accumulate_sum(&sum, src[RIDX(i - 1, j + 1, dim)]);
//     accumulate_sum(&sum, src[RIDX(i - 1, j - 1, dim)]);

//     assign_sum_to_pixel_norm(&current_pixel, sum);

//     return current_pixel;
// }
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
            dst[RIDX(i, j, dim)] = avg(dim, i, j, src);
}
/*
 * smooth - Your current working version of smooth
 *          Our supplied version simply calls naive_smooth
 */
char another_smooth_descr[] = "another_smooth: Another version of smooth";
void another_smooth(int dim, pixel *src, pixel *dst)
{
    // Edge cases
    for (int i = 1; i < dim - 1; i++)
    {
        pixel_sum sum;
        pixel current_pixel;

        initialize_pixel_sum(&sum);

        accumulate_sum(&sum, src[RIDX(i, 0, dim)]);
        accumulate_sum(&sum, src[RIDX(i + 1, 0, dim)]);
        accumulate_sum(&sum, src[RIDX(i - 1, 0, dim)]);
        accumulate_sum(&sum, src[RIDX(i, 1, dim)]);
        accumulate_sum(&sum, src[RIDX(i + 1, 1, dim)]);
        accumulate_sum(&sum, src[RIDX(i - 1, 1, dim)]);

        assign_sum_to_pixel_edge(&current_pixel, sum);

        dst[RIDX(i, 0, dim)] = current_pixel; // Top edge

        pixel_sum sum2;
        pixel current_pixel2;

        initialize_pixel_sum(&sum2);

        accumulate_sum(&sum2, src[RIDX(i, dim - 1, dim)]);
        accumulate_sum(&sum2, src[RIDX(i + 1, dim - 1, dim)]);
        accumulate_sum(&sum2, src[RIDX(i - 1, dim - 1, dim)]);
        accumulate_sum(&sum2, src[RIDX(i, dim - 2, dim)]);
        accumulate_sum(&sum2, src[RIDX(i + 1, dim - 2, dim)]);
        accumulate_sum(&sum2, src[RIDX(i - 1, dim - 2, dim)]);

        assign_sum_to_pixel_edge(&current_pixel2, sum2);

        dst[RIDX(i, dim - 1, dim)] = current_pixel2; // Bottom edge

        pixel_sum sum3;
        pixel current_pixel3;

        initialize_pixel_sum(&sum3);

        accumulate_sum(&sum3, src[RIDX(0, i, dim)]);
        accumulate_sum(&sum3, src[RIDX(0, i + 1, dim)]);
        accumulate_sum(&sum3, src[RIDX(0, i - 1, dim)]);
        accumulate_sum(&sum3, src[RIDX(1, i, dim)]);
        accumulate_sum(&sum3, src[RIDX(1, i + 1, dim)]);
        accumulate_sum(&sum3, src[RIDX(1, i - 1, dim)]);

        assign_sum_to_pixel_edge(&current_pixel3, sum3);

        dst[RIDX(0, i, dim)] = current_pixel3; // Left edge

        pixel_sum sum4;
        pixel current_pixel4;

        initialize_pixel_sum(&sum4);

        accumulate_sum(&sum4, src[RIDX(dim - 1, i, dim)]);
        accumulate_sum(&sum4, src[RIDX(dim - 1, i + 1, dim)]);
        accumulate_sum(&sum4, src[RIDX(dim - 1, i - 1, dim)]);
        accumulate_sum(&sum4, src[RIDX(dim - 2, i, dim)]);
        accumulate_sum(&sum4, src[RIDX(dim - 2, i + 1, dim)]);
        accumulate_sum(&sum4, src[RIDX(dim - 2, i - 1, dim)]);

        assign_sum_to_pixel_edge(&current_pixel4, sum4);

        dst[RIDX(dim - 1, i, dim)] = current_pixel4; // Right edge
    }

    // Corner cases
    pixel_sum sum5;
    pixel current_pixel5;

    initialize_pixel_sum(&sum5);

    accumulate_sum(&sum5, src[RIDX(0, 0, dim)]);
    accumulate_sum(&sum5, src[RIDX(0, 1, dim)]);
    accumulate_sum(&sum5, src[RIDX(1, 0, dim)]);
    accumulate_sum(&sum5, src[RIDX(1, 1, dim)]);

    assign_sum_to_pixel_corner(&current_pixel5, sum5);

    dst[RIDX(0, 0, dim)] = current_pixel5; // Top left

    pixel_sum sum6;
    pixel current_pixel6;

    initialize_pixel_sum(&sum6);

    accumulate_sum(&sum6, src[RIDX(dim - 1, 0, dim)]);
    accumulate_sum(&sum6, src[RIDX(dim - 2, 0, dim)]);
    accumulate_sum(&sum6, src[RIDX(dim - 2, 1, dim)]);
    accumulate_sum(&sum6, src[RIDX(dim - 1, 1, dim)]);

    assign_sum_to_pixel_corner(&current_pixel6, sum6);

    dst[RIDX(dim - 1, 0, dim)] = current_pixel6; // Top right

    pixel_sum sum7;
    pixel current_pixel7;

    initialize_pixel_sum(&sum7);

    accumulate_sum(&sum7, src[RIDX(0, dim - 1, dim)]);
    accumulate_sum(&sum7, src[RIDX(0, dim - 2, dim)]);
    accumulate_sum(&sum7, src[RIDX(1, dim - 2, dim)]);
    accumulate_sum(&sum7, src[RIDX(1, dim - 1, dim)]);

    assign_sum_to_pixel_corner(&current_pixel7, sum7);

    dst[RIDX(0, dim - 1, dim)] = current_pixel7; // Bottom left

    pixel_sum sum8;
    pixel current_pixel8;

    initialize_pixel_sum(&sum8);

    accumulate_sum(&sum8, src[RIDX(dim - 1, dim - 1, dim)]);
    accumulate_sum(&sum8, src[RIDX(dim - 2, dim - 1, dim)]);
    accumulate_sum(&sum8, src[RIDX(dim - 1, dim - 2, dim)]);
    accumulate_sum(&sum8, src[RIDX(dim - 2, dim - 2, dim)]);

    assign_sum_to_pixel_corner(&current_pixel8, sum8);

    dst[RIDX(dim - 1, dim - 1, dim)] = current_pixel8; // Bottom right

    // Normal cases
    for (int i = 1; i < dim - 1; i++)
    {
        for (int j = 1; j < dim - 1; j++)
        {
            pixel_sum sum9;
            pixel current_pixel9;

            initialize_pixel_sum(&sum9);

            // accumulate_sum(&sum9, src[RIDX(i, j, dim)]);
            //  load 128 bits (4 pixels)
            __m128i the_pixel1 = _mm_loadu_si128((__m128i *)&src[RIDX(i, j, dim)]);
            __m256i the_pixel = _mm256_cvtepu8_epi16(the_pixel1);

            // accumulate_sum(&sum9, src[RIDX(i - 1, j, dim)]);
            __m128i left_pixel1 = _mm_loadu_si128((__m128i *)&src[RIDX(i - 1, j, dim)]);
            __m256i left_pixel = _mm256_cvtepu8_epi16(left_pixel1);

            // accumulate_sum(&sum9, src[RIDX(i + 1, j, dim)]);
            __m128i right_pixel1 = _mm_loadu_si128((__m128i *)&src[RIDX(i + 1, j, dim)]);
            __m256i right_pixel = _mm256_cvtepu8_epi16(right_pixel1);

            // accumulate_sum(&sum9, src[RIDX(i, j - 1, dim)]);
            __m128i top_pixel1 = _mm_loadu_si128((__m128i *)&src[RIDX(i, j - 1, dim)]);
            __m256i top_pixel = _mm256_cvtepu8_epi16(top_pixel1);

            // accumulate_sum(&sum9, src[RIDX(i, j + 1, dim)]);
            __m128i bot_pixel1 = _mm_loadu_si128((__m128i *)&src[RIDX(i, j + 1, dim)]);
            __m256i bot_pixel = _mm256_cvtepu8_epi16(bot_pixel1);

            // accumulate_sum(&sum9, src[RIDX(i + 1, j + 1, dim)]);
            __m128i botright_pixel1 = _mm_loadu_si128((__m128i *)&src[RIDX(i + 1, j + 1, dim)]);
            __m256i botright_pixel = _mm256_cvtepu8_epi16(botright_pixel1);

            // accumulate_sum(&sum9, src[RIDX(i + 1, j - 1, dim)]);
            __m128i topright_pixel1 = _mm_loadu_si128((__m128i *)&src[RIDX(i + 1, j - 1, dim)]);
            __m256i topright_pixel = _mm256_cvtepu8_epi16(topright_pixel1);

            // accumulate_sum(&sum9, src[RIDX(i - 1, j + 1, dim)]);
            __m128i botleft_pixel1 = _mm_loadu_si128((__m128i *)&src[RIDX(i - 1, j + 1, dim)]);
            __m256i botleft_pixel = _mm256_cvtepu8_epi16(botleft_pixel1);

            // accumulate_sum(&sum9, src[RIDX(i - 1, j - 1, dim)]);
            __m128i topleft_pixel1 = _mm_loadu_si128((__m128i *)&src[RIDX(i - 1, j - 1, dim)]);
            __m256i topleft_pixel = _mm256_cvtepu8_epi16(topleft_pixel1);

            __m256i theleft_sum = _mm256_add_epi32(the_pixel, left_pixel);
            __m256i righttop_sum = _mm256_add_epi32(right_pixel, top_pixel);
            __m256i botbotright_sum = _mm256_add_epi32(bot_pixel, botright_pixel);
            __m256i toprightbotleft_sum = _mm256_add_epi32(topright_pixel, botleft_pixel);

            __m256i theleft_righttop_sum = _mm256_add_epi32(theleft_sum, righttop_sum);
            __m256i botbotright_toprightbotleft_sum = _mm256_add_epi32(botbotright_sum, toprightbotleft_sum);

            __m256i theleft_righttop__botbotright_toprightbotleft_sum = _mm256_add_epi32(theleft_righttop_sum, botbotright_toprightbotleft_sum);

            __m256i sum_of_pixels = _mm256_add_epi32(theleft_righttop__botbotright_toprightbotleft_sum, topleft_pixel);

            unsigned short pixel_elements[16];
            _mm256_storeu_si256((__m256i *)pixel_elements, sum_of_pixels);

            // assign_sum_to_pixel_norm(&current_pixel9, pixel_elements);
            current_pixel9.red = (unsigned short)(pixel_elements[0] / 9);
            current_pixel9.green = (unsigned short)(pixel_elements[1] / 9);
            current_pixel9.blue = (unsigned short)(pixel_elements[2] / 9);
            current_pixel9.alpha = (unsigned short)(pixel_elements[3] / 9);

            dst[RIDX(i, j, dim)] = current_pixel9; // Normal case
        }
    }
}

/*********************************************************************
 * register_smooth_functions - Register all of your different versions
 *     of the smooth function by calling the add_smooth_function() for
 *     each test function. When you run the benchmark program, it will
 *     test and report the performance of each registered test
 *     function.
 *********************************************************************/

void register_smooth_functions()
{
    add_smooth_function(&naive_smooth, naive_smooth_descr);
    add_smooth_function(&another_smooth, another_smooth_descr);
}