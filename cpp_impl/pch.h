// common headers that may be used across files
#pragma once

#include <assert.h>
#include <iostream>
#include <stdexcept>
#include <stdio.h>
#include <string.h> // for memset


// these file flags control several print statements and checks in all the files
//the higher the number, the more printing that occurs


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