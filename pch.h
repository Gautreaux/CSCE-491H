// common headers that may be used across files
#pragma once

#include <assert.h>
#include <iostream>
#include <stdexcept>
#include <stdio.h>
#include <stdlib.h>
#include <string.h> // for memset


//some util stuff
#define DOUBLE_COMPARISON_THRESHOLD 0.000001
#define DOUBLE_EQUAL(a,b) (abs(a-b) <= DOUBLE_COMPARISON_THRESHOLD)
#define DOUBLE_NEQ(a,b) (!DOUBLE_EQUAL(a,b))
#define DOUBLE_LT(a,b) ((b-a-DOUBLE_COMPARISON_THRESHOLD) > 0)
#define DOUBLE_GT(a,b) ((a-b-DOUBLE_COMPARISON_THRESHOLD) > 0)
#define DOUBLE_LEQ(a,b) (!DOUBLE_GT(a,b))
#define DOUBLE_GEQ(a,b) (!DOUBLE_LT(a,b))

#define MB_TO_B (1024*1024)
#define B_TO_MB (1.0/(MB_TO_B))

#define CEIL_DIVISION(x, y) ((x % y) ? ((x / y) + 1) : (x / y))

// these file flags control several print statements and checks in all the files
//the higher the number, the more printing that occurs

#ifdef __NVCC__
#define NVCC_HD __host__ __device__
#else
#define NVCC_HD
#endif

// Ensure that the n-1 debug flag esists for the n debug flag
#ifdef DEBUG_4
#ifndef DEBUG_3
#define DEBUG_3
#endif //DEBUG_3
#endif //DEBUG_4

#ifdef DEBUG_3
#ifndef DEBUG_2
#define DEBUG_2
#endif //DEBUG_2
#endif //DEBUG_3

#ifdef DEBUG_2
#ifndef DEBUG_1
#define DEBUG_1
#endif //DEBUG_1
#endif //DEBUG_2

#ifdef DEBUG_1
#ifndef DEBUG
#define DEBUG
#endif  //DEBUG
#endif  //DEBUG_1

//now additional flags cane be enabled on the above ones
//  this allows things to be more segmented