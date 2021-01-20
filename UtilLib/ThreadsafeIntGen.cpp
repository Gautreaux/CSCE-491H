#include "ThreadsafeIntGen.h"

ThreadsafeIntGen::ThreadsafeIntGen(const int start, const int incr) : 
    x(start), incr(incr)
{}

// why is this not found?
//  even though the above one is found
// inline int ThreadsafeIntGen::next(void){
//     return x.fetch_add(incr);
// }