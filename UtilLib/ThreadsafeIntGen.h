#pragma once

#include "../pch.h"

#include <atomic>

class ThreadsafeIntGen{
private:
    std::atomic<int> x;
    const int incr;
public:
    //start - starting value (first value generated)
    //incr - amount to increment after generation
    ThreadsafeIntGen(const int start = 0, const int incr = 1); 

    //get the next int atomically
    //  may overflow (or underflow on negative incr)
    inline int next(void){
        return x.fetch_add(incr);
    }
};