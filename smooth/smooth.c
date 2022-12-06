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
        ////////////////// Top Edge //////////////////
        pixel current_pixel;

        // accumulate_sum(&sum, src[RIDX(i, 0, dim)]);
        //   load 128 bits (4 pixels)
        __m128i the_pixel1 = _mm_loadu_si128((__m128i *)&src[RIDX(i, 0, dim)]);
        __m256i the_pixel = _mm256_cvtepu8_epi16(the_pixel1);

        // accumulate_sum(&sum, src[RIDX(i + 1, 0, dim)]);
        __m128i right_pixel1 = _mm_loadu_si128((__m128i *)&src[RIDX(i + 1, 0, dim)]);
        __m256i right_pixel = _mm256_cvtepu8_epi16(right_pixel1);

        // accumulate_sum(&sum, src[RIDX(i - 1, 0, dim)]);
        __m128i left_pixel1 = _mm_loadu_si128((__m128i *)&src[RIDX(i - 1, 0, dim)]);
        __m256i left_pixel = _mm256_cvtepu8_epi16(left_pixel1);

        // accumulate_sum(&sum, src[RIDX(i, 1, dim)]);
        __m128i bot_pixel1 = _mm_loadu_si128((__m128i *)&src[RIDX(i, 1, dim)]);
        __m256i bot_pixel = _mm256_cvtepu8_epi16(bot_pixel1);

        // accumulate_sum(&sum, src[RIDX(i + 1, 1, dim)]);
        __m128i botright_pixel1 = _mm_loadu_si128((__m128i *)&src[RIDX(i + 1, 1, dim)]);
        __m256i botright_pixel = _mm256_cvtepu8_epi16(botright_pixel1);

        // accumulate_sum(&sum, src[RIDX(i - 1, 1, dim)]);
        __m128i botleft_pixel1 = _mm_loadu_si128((__m128i *)&src[RIDX(i - 1, 1, dim)]);
        __m256i botleft_pixel = _mm256_cvtepu8_epi16(botleft_pixel1);

        __m256i theright_sum = _mm256_add_epi32(the_pixel, right_pixel);
        __m256i leftbot_sum = _mm256_add_epi32(left_pixel, bot_pixel);
        __m256i botrightbotleft_sum = _mm256_add_epi32(botright_pixel, botleft_pixel);

        __m256i theright_leftbot_sum = _mm256_add_epi32(theright_sum, leftbot_sum);

        __m256i sum_of_pixels = _mm256_add_epi32(theright_leftbot_sum, botrightbotleft_sum);

        unsigned short pixel_elements[16];
        _mm256_storeu_si256((__m256i *)pixel_elements, sum_of_pixels);

        // assign_sum_to_pixel_norm(&current_pixel9, pixel_elements);
        current_pixel.red = (unsigned short)(pixel_elements[0] / 6);
        current_pixel.green = (unsigned short)(pixel_elements[1] / 6);
        current_pixel.blue = (unsigned short)(pixel_elements[2] / 6);
        current_pixel.alpha = (unsigned short)(pixel_elements[3] / 6);

        // assign_sum_to_pixel_edge(&current_pixel, sum);

        dst[RIDX(i, 0, dim)] = current_pixel; // Top edge

        //////////////////////////////////////////////////

        ////////////////// Bottom Edge //////////////////

        pixel current_pixel2;

        // accumulate_sum(&sum2, src[RIDX(i, dim - 1, dim)]);
        //    load 128 bits (4 pixels)
        __m128i the_pixel2 = _mm_loadu_si128((__m128i *)&src[RIDX(i, dim - 1, dim)]);
        __m256i the_pixel3 = _mm256_cvtepu8_epi16(the_pixel2);

        // accumulate_sum(&sum2, src[RIDX(i + 1, dim - 1, dim)]);
        __m128i botright_pixel2 = _mm_loadu_si128((__m128i *)&src[RIDX(i + 1, dim - 1, dim)]);
        __m256i botright_pixel3 = _mm256_cvtepu8_epi16(botright_pixel2);

        // accumulate_sum(&sum2, src[RIDX(i - 1, dim - 1, dim)]);
        __m128i botleft_pixel2 = _mm_loadu_si128((__m128i *)&src[RIDX(i - 1, dim - 1, dim)]);
        __m256i botleft_pixel3 = _mm256_cvtepu8_epi16(botleft_pixel2);

        // accumulate_sum(&sum2, src[RIDX(i, dim - 2, dim)]);
        __m128i top_pixel2 = _mm_loadu_si128((__m128i *)&src[RIDX(i, dim - 2, dim)]);
        __m256i top_pixel3 = _mm256_cvtepu8_epi16(top_pixel2);

        // accumulate_sum(&sum2, src[RIDX(i + 1, dim - 2, dim)]);
        __m128i topright_pixel2 = _mm_loadu_si128((__m128i *)&src[RIDX(i + 1, dim - 2, dim)]);
        __m256i topright_pixel3 = _mm256_cvtepu8_epi16(topright_pixel2);

        // accumulate_sum(&sum2, src[RIDX(i - 1, dim - 2, dim)]);
        __m128i topleft_pixel2 = _mm_loadu_si128((__m128i *)&src[RIDX(i - 1, dim - 2, dim)]);
        __m256i topleft_pixel3 = _mm256_cvtepu8_epi16(topleft_pixel2);

        __m256i thebotright_sum2 = _mm256_add_epi32(the_pixel3, botright_pixel3);
        __m256i botlefttop_sum2 = _mm256_add_epi32(botleft_pixel3, top_pixel3);
        __m256i toprighttopleft_sum2 = _mm256_add_epi32(topright_pixel3, topleft_pixel3);

        __m256i thebotright_botlefttop_sum2 = _mm256_add_epi32(thebotright_sum2, botlefttop_sum2);

        __m256i sum_of_pixels2 = _mm256_add_epi32(thebotright_botlefttop_sum2, toprighttopleft_sum2);

        unsigned short pixel_elements2[16];
        _mm256_storeu_si256((__m256i *)pixel_elements2, sum_of_pixels2);

        current_pixel2.red = (unsigned short)(pixel_elements2[0] / 6);
        current_pixel2.green = (unsigned short)(pixel_elements2[1] / 6);
        current_pixel2.blue = (unsigned short)(pixel_elements2[2] / 6);
        current_pixel2.alpha = (unsigned short)(pixel_elements2[3] / 6);

        dst[RIDX(i, dim - 1, dim)] = current_pixel2; // Bottom edge

        //////////////////////////////////////////////////

        ////////////////// Left Edge //////////////////
        pixel current_pixel3;

        // accumulate_sum(&sum3, src[RIDX(0, i, dim)]);
        __m128i the_pixel4 = _mm_loadu_si128((__m128i *)&src[RIDX(0, i, dim)]);
        __m256i the_pixel5 = _mm256_cvtepu8_epi16(the_pixel4);

        // accumulate_sum(&sum3, src[RIDX(0, i + 1, dim)]);
        __m128i bot_pixel4 = _mm_loadu_si128((__m128i *)&src[RIDX(0, i + 1, dim)]);
        __m256i bot_pixel5 = _mm256_cvtepu8_epi16(bot_pixel4);

        // accumulate_sum(&sum3, src[RIDX(0, i - 1, dim)]);
        __m128i top_pixel4 = _mm_loadu_si128((__m128i *)&src[RIDX(0, i - 1, dim)]);
        __m256i top_pixel5 = _mm256_cvtepu8_epi16(top_pixel4);

        // accumulate_sum(&sum3, src[RIDX(1, i, dim)]);
        __m128i right_pixel4 = _mm_loadu_si128((__m128i *)&src[RIDX(1, i, dim)]);
        __m256i right_pixel5 = _mm256_cvtepu8_epi16(right_pixel4);

        // accumulate_sum(&sum3, src[RIDX(1, i + 1, dim)]);
        __m128i botright_pixel4 = _mm_loadu_si128((__m128i *)&src[RIDX(1, i + 1, dim)]);
        __m256i botright_pixel5 = _mm256_cvtepu8_epi16(botright_pixel4);

        // accumulate_sum(&sum3, src[RIDX(1, i - 1, dim)]);
        __m128i topright_pixel4 = _mm_loadu_si128((__m128i *)&src[RIDX(1, i - 1, dim)]);
        __m256i topright_pixel5 = _mm256_cvtepu8_epi16(topright_pixel4);

        __m256i thebot_sum3 = _mm256_add_epi32(the_pixel5, bot_pixel5);
        __m256i topright_sum3 = _mm256_add_epi32(top_pixel5, right_pixel5);
        __m256i botrighttopright_sum3 = _mm256_add_epi32(botright_pixel5, topright_pixel5);

        __m256i thebot_topright_sum3 = _mm256_add_epi32(thebot_sum3, topright_sum3);

        __m256i sum_of_pixels3 = _mm256_add_epi32(thebot_topright_sum3, botrighttopright_sum3);

        unsigned short pixel_elements3[16];
        _mm256_storeu_si256((__m256i *)pixel_elements3, sum_of_pixels3);

        current_pixel3.red = (unsigned short)(pixel_elements3[0] / 6);
        current_pixel3.green = (unsigned short)(pixel_elements3[1] / 6);
        current_pixel3.blue = (unsigned short)(pixel_elements3[2] / 6);
        current_pixel3.alpha = (unsigned short)(pixel_elements3[3] / 6);

        dst[RIDX(0, i, dim)] = current_pixel3; // Left edge

        //////////////////////////////////////////////////

        ////////////////// Right Edge //////////////////

        pixel current_pixel4;

        // accumulate_sum(&sum4, src[RIDX(dim - 1, i, dim)]);
        __m128i the_pixel6 = _mm_loadu_si128((__m128i *)&src[RIDX(dim - 1, i, dim)]);
        __m256i the_pixel7 = _mm256_cvtepu8_epi16(the_pixel6);

        // accumulate_sum(&sum4, src[RIDX(dim - 1, i + 1, dim)]);
        __m128i bot_pixel6 = _mm_loadu_si128((__m128i *)&src[RIDX(dim - 1, i + 1, dim)]);
        __m256i bot_pixel7 = _mm256_cvtepu8_epi16(bot_pixel6);

        // accumulate_sum(&sum4, src[RIDX(dim - 1, i - 1, dim)]);
        __m128i top_pixel6 = _mm_loadu_si128((__m128i *)&src[RIDX(dim - 1, i - 1, dim)]);
        __m256i top_pixel7 = _mm256_cvtepu8_epi16(top_pixel6);

        // accumulate_sum(&sum4, src[RIDX(dim - 2, i, dim)]);
        __m128i left_pixel6 = _mm_loadu_si128((__m128i *)&src[RIDX(dim - 2, i, dim)]);
        __m256i left_pixel7 = _mm256_cvtepu8_epi16(left_pixel6);

        // accumulate_sum(&sum4, src[RIDX(dim - 2, i + 1, dim)]);
        __m128i botleft_pixel6 = _mm_loadu_si128((__m128i *)&src[RIDX(dim - 2, i + 1, dim)]);
        __m256i botleft_pixel7 = _mm256_cvtepu8_epi16(botleft_pixel6);

        // accumulate_sum(&sum4, src[RIDX(dim - 2, i - 1, dim)]);
        __m128i topleft_pixel6 = _mm_loadu_si128((__m128i *)&src[RIDX(dim - 2, i - 1, dim)]);
        __m256i topleft_pixel7 = _mm256_cvtepu8_epi16(topleft_pixel6);

        __m256i thebot_sum4 = _mm256_add_epi32(the_pixel7, bot_pixel7);
        __m256i topleft_sum4 = _mm256_add_epi32(top_pixel7, left_pixel7);
        __m256i botlefttopleft_sum4 = _mm256_add_epi32(botleft_pixel7, topleft_pixel7);

        __m256i thebot_topleft_sum4 = _mm256_add_epi32(thebot_sum4, topleft_sum4);

        __m256i sum_of_pixels4 = _mm256_add_epi32(thebot_topleft_sum4, botlefttopleft_sum4);

        unsigned short pixel_elements4[16];
        _mm256_storeu_si256((__m256i *)pixel_elements4, sum_of_pixels4);

        current_pixel4.red = (unsigned short)(pixel_elements4[0] / 6);
        current_pixel4.green = (unsigned short)(pixel_elements4[1] / 6);
        current_pixel4.blue = (unsigned short)(pixel_elements4[2] / 6);
        current_pixel4.alpha = (unsigned short)(pixel_elements4[3] / 6);

        dst[RIDX(dim - 1, i, dim)] = current_pixel4; // Right edge

        //////////////////////////////////////////////////
    }

    // Corner cases
    pixel current_pixel5;

    // accumulate_sum(&sum5, src[RIDX(0, 0, dim)]);
    __m128i the_pixel8 = _mm_loadu_si128((__m128i *)&src[RIDX(0, 0, dim)]);
    __m256i the_pixel9 = _mm256_cvtepu8_epi16(the_pixel8);

    // accumulate_sum(&sum5, src[RIDX(0, 1, dim)]);
    __m128i bot_pixel8 = _mm_loadu_si128((__m128i *)&src[RIDX(0, 1, dim)]);
    __m256i bot_pixel9 = _mm256_cvtepu8_epi16(bot_pixel8);

    // accumulate_sum(&sum5, src[RIDX(1, 0, dim)]);
    __m128i right_pixel8 = _mm_loadu_si128((__m128i *)&src[RIDX(1, 0, dim)]);
    __m256i right_pixel9 = _mm256_cvtepu8_epi16(right_pixel8);

    // accumulate_sum(&sum5, src[RIDX(1, 1, dim)]);
    __m128i botright_pixel8 = _mm_loadu_si128((__m128i *)&src[RIDX(1, 1, dim)]);
    __m256i botright_pixel9 = _mm256_cvtepu8_epi16(botright_pixel8);

    __m256i thebot_sum5 = _mm256_add_epi32(the_pixel9, bot_pixel9);
    __m256i rightbotright_sum5 = _mm256_add_epi32(right_pixel9, botright_pixel9);
    __m256i sum_of_pixels5 = _mm256_add_epi32(thebot_sum5, rightbotright_sum5);

    unsigned short pixel_elements5[16];
    _mm256_storeu_si256((__m256i *)pixel_elements5, sum_of_pixels5);

    current_pixel5.red = (unsigned short)(pixel_elements5[0] / 4);
    current_pixel5.green = (unsigned short)(pixel_elements5[1] / 4);
    current_pixel5.blue = (unsigned short)(pixel_elements5[2] / 4);
    current_pixel5.alpha = (unsigned short)(pixel_elements5[3] / 4);

    dst[RIDX(0, 0, dim)] = current_pixel5; // Top left

    pixel current_pixel6;

    // accumulate_sum(&sum6, src[RIDX(dim - 1, 0, dim)]);
    __m128i the_pixel10 = _mm_loadu_si128((__m128i *)&src[RIDX(dim - 1, 0, dim)]);
    __m256i the_pixel11 = _mm256_cvtepu8_epi16(the_pixel10);

    // accumulate_sum(&sum6, src[RIDX(dim - 2, 0, dim)]);
    __m128i left_pixel10 = _mm_loadu_si128((__m128i *)&src[RIDX(dim - 2, 0, dim)]);
    __m256i left_pixel11 = _mm256_cvtepu8_epi16(left_pixel10);

    // accumulate_sum(&sum6, src[RIDX(dim - 2, 1, dim)]);
    __m128i botleft_pixel10 = _mm_loadu_si128((__m128i *)&src[RIDX(dim - 2, 1, dim)]);
    __m256i botleft_pixel11 = _mm256_cvtepu8_epi16(botleft_pixel10);

    // accumulate_sum(&sum6, src[RIDX(dim - 1, 1, dim)]);
    __m128i bot_pixel10 = _mm_loadu_si128((__m128i *)&src[RIDX(dim - 1, 1, dim)]);
    __m256i bot_pixel11 = _mm256_cvtepu8_epi16(bot_pixel10);

    __m256i theleft_sum6 = _mm256_add_epi32(the_pixel11, left_pixel11);
    __m256i botleftbot_sum6 = _mm256_add_epi32(botleft_pixel11, bot_pixel11);
    __m256i sum_of_pixels6 = _mm256_add_epi32(theleft_sum6, botleftbot_sum6);

    unsigned short pixel_elements6[16];
    _mm256_storeu_si256((__m256i *)pixel_elements6, sum_of_pixels6);

    current_pixel6.red = (unsigned short)(pixel_elements6[0] / 4);
    current_pixel6.green = (unsigned short)(pixel_elements6[1] / 4);
    current_pixel6.blue = (unsigned short)(pixel_elements6[2] / 4);
    current_pixel6.alpha = (unsigned short)(pixel_elements6[3] / 4);

    dst[RIDX(dim - 1, 0, dim)] = current_pixel6; // Top right

    pixel current_pixel7;

    // accumulate_sum(&sum7, src[RIDX(0, dim - 1, dim)]);
    __m128i the_pixel12 = _mm_loadu_si128((__m128i *)&src[RIDX(0, dim - 1, dim)]);
    __m256i the_pixel13 = _mm256_cvtepu8_epi16(the_pixel12);

    // accumulate_sum(&sum7, src[RIDX(0, dim - 2, dim)]);
    __m128i top_pixel12 = _mm_loadu_si128((__m128i *)&src[RIDX(0, dim - 2, dim)]);
    __m256i top_pixel13 = _mm256_cvtepu8_epi16(top_pixel12);

    // accumulate_sum(&sum7, src[RIDX(1, dim - 2, dim)]);
    __m128i topright_pixel12 = _mm_loadu_si128((__m128i *)&src[RIDX(1, dim - 2, dim)]);
    __m256i topright_pixel13 = _mm256_cvtepu8_epi16(topright_pixel12);

    // accumulate_sum(&sum7, src[RIDX(1, dim - 1, dim)]);
    __m128i right_pixel12 = _mm_loadu_si128((__m128i *)&src[RIDX(1, dim - 1, dim)]);
    __m256i right_pixel13 = _mm256_cvtepu8_epi16(right_pixel12);

    __m256i thetop_sum7 = _mm256_add_epi32(the_pixel13, top_pixel13);
    __m256i toprightright_sum7 = _mm256_add_epi32(topright_pixel13, right_pixel13);
    __m256i sum_of_pixels7 = _mm256_add_epi32(thetop_sum7, toprightright_sum7);

    unsigned short pixel_elements7[16];
    _mm256_storeu_si256((__m256i *)pixel_elements7, sum_of_pixels7);

    current_pixel7.red = (unsigned short)(pixel_elements7[0] / 4);
    current_pixel7.green = (unsigned short)(pixel_elements7[1] / 4);
    current_pixel7.blue = (unsigned short)(pixel_elements7[2] / 4);
    current_pixel7.alpha = (unsigned short)(pixel_elements7[3] / 4);

    dst[RIDX(0, dim - 1, dim)] = current_pixel7; // Bottom left

    pixel current_pixel8;

    // accumulate_sum(&sum8, src[RIDX(dim - 1, dim - 1, dim)]);
    __m128i the_pixel14 = _mm_loadu_si128((__m128i *)&src[RIDX(dim - 1, dim - 1, dim)]);
    __m256i the_pixel15 = _mm256_cvtepu8_epi16(the_pixel14);

    // accumulate_sum(&sum8, src[RIDX(dim - 2, dim - 1, dim)]);
    __m128i left_pixel14 = _mm_loadu_si128((__m128i *)&src[RIDX(dim - 2, dim - 1, dim)]);
    __m256i left_pixel15 = _mm256_cvtepu8_epi16(left_pixel14);

    // accumulate_sum(&sum8, src[RIDX(dim - 1, dim - 2, dim)]);
    __m128i top_pixel14 = _mm_loadu_si128((__m128i *)&src[RIDX(dim - 1, dim - 2, dim)]);
    __m256i top_pixel15 = _mm256_cvtepu8_epi16(top_pixel14);

    // accumulate_sum(&sum8, src[RIDX(dim - 2, dim - 2, dim)]);
    __m128i topleft_pixel14 = _mm_loadu_si128((__m128i *)&src[RIDX(dim - 2, dim - 2, dim)]);
    __m256i topleft_pixel15 = _mm256_cvtepu8_epi16(topleft_pixel14);

    __m256i theleft_sum8 = _mm256_add_epi32(the_pixel15, left_pixel15);
    __m256i toptopleft_sum8 = _mm256_add_epi32(top_pixel15, topleft_pixel15);
    __m256i sum_of_pixels8 = _mm256_add_epi32(theleft_sum8, toptopleft_sum8);

    unsigned short pixel_elements8[16];
    _mm256_storeu_si256((__m256i *)pixel_elements8, sum_of_pixels8);

    current_pixel8.red = (unsigned short)(pixel_elements8[0] / 4);
    current_pixel8.green = (unsigned short)(pixel_elements8[1] / 4);
    current_pixel8.blue = (unsigned short)(pixel_elements8[2] / 4);
    current_pixel8.alpha = (unsigned short)(pixel_elements8[3] / 4);

    dst[RIDX(dim - 1, dim - 1, dim)] = current_pixel8; // Bottom right

    // Normal cases
    for (int i = 1; i < dim - 1; i++)
    {
        for (int j = 1; j < dim - 1; j++)
        {
            pixel current_pixel;
            // pixel current_pixel1;
            // pixel current_pixel2;
            // pixel current_pixel3;

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

            ///////// OPTION A PROGRESS /////////
            // __m256i topleft_top_sum = _mm256_add_epi32(topleft_pixel, top_pixel);
            // __m256i left_the_sum = _mm256_add_epi32(left_pixel, the_pixel);
            // __m256i botleft_left_sum = _mm256_add_epi32(botleft_pixel, left_pixel);

            // __m256i toplefttop_leftthe_sum = _mm256_add_epi32(topleft_top_sum, left_the_sum);
            // __m256i sum_of_pixels = _mm256_add_epi32(toplefttop_leftthe_sum, botleft_left_sum);
            //////////////////////// /////////

            unsigned short pixel_elements[16];
            _mm256_storeu_si256((__m256i *)pixel_elements, sum_of_pixels);

            // assign_sum_to_pixel_norm(&current_pixel9, pixel_elements);
            current_pixel.red = (unsigned short)(pixel_elements[0] / 9);
            current_pixel.green = (unsigned short)(pixel_elements[1] / 9);
            current_pixel.blue = (unsigned short)(pixel_elements[2] / 9);
            current_pixel.alpha = (unsigned short)(pixel_elements[3] / 9);

            dst[RIDX(i, j, dim)] = current_pixel; // Normal case

            // current_pixel1.red = (unsigned short)(pixel_elements[4] / 9);
            // current_pixel1.green = (unsigned short)(pixel_elements[5] / 9);
            // current_pixel1.blue = (unsigned short)(pixel_elements[6] / 9);
            // current_pixel1.alpha = (unsigned short)(pixel_elements[7] / 9);

            // dst[RIDX(i + 1, j, dim)] = current_pixel1; // Normal case

            // current_pixel2.red = (unsigned short)(pixel_elements[8] / 9);
            // current_pixel2.green = (unsigned short)(pixel_elements[9] / 9);
            // current_pixel2.blue = (unsigned short)(pixel_elements[10] / 9);
            // current_pixel2.alpha = (unsigned short)(pixel_elements[11] / 9);

            // dst[RIDX(i + 2, j, dim)] = current_pixel2; // Normal case

            // current_pixel3.red = (unsigned short)(pixel_elements[12] / 9);
            // current_pixel3.green = (unsigned short)(pixel_elements[13] / 9);
            // current_pixel3.blue = (unsigned short)(pixel_elements[14] / 9);
            // current_pixel3.alpha = (unsigned short)(pixel_elements[15] / 9);

            // dst[RIDX(i + 3, j, dim)] = current_pixel3; // Normal case
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