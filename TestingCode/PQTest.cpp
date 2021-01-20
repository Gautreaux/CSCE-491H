
#include "../pch.h"

#include <stdlib.h>
#include <time.h>

#include "../UtilLib/DynamicBitset.h"
#include "../RecomputeState.h"
#include "../PrunedAStar.h"


#define NUM_PUSH 10

DynamicBitset getRandomDBS(unsigned int dbssize = 8){
    DynamicBitset dbs(dbssize);

    for(unsigned int i = 0; i < dbssize; i++){
        auto j = rand() % 2;
        dbs.set(i, j == 0);
    }
    return dbs;
}

RecomputeState getRandomRS(const RecomputeState* parent = nullptr){
    return RecomputeState(rand() % 10, rand() % 10, rand() % 10, getRandomDBS(), parent);
}

void ManualTest(){
    State_PQ pq;
    DynamicBitset dbs1(8);
    dbs1.set(1);
    //dbs1.set(6);
    RecomputeState rs1(1, 1, 2, dbs1, nullptr);

    DynamicBitset dbs2(8);
    dbs2.set(2);
    dbs2.set(3);
    dbs2.set(4);
    dbs2.set(5);
    RecomputeState rs2(1,1,2, dbs2, nullptr);

    DynamicBitset dbs3(8);
    for(unsigned int i = 0; i < 8; i++){
        dbs3.set(i);
    }
    RecomputeState rs3(3, 4, 4, dbs3, &rs2);

    for(unsigned int i = 0; i < NUM_PUSH; i++){
        std::cout << "Pushing new state " << rs1 << std::endl;
        pq.push(rs1);
        std::cout << "Pushing new state " << rs2 << std::endl;
        pq.push(rs2);
        std::cout << "Pushing new state " << rs3 << std::endl;
        pq.push(rs3);
    }

    for(unsigned int i = 0; i < NUM_PUSH*3; i++){
        auto state = pq.top();
        std::cout << "Current state: " << state << std::endl;
        // assert(state == rs1);
        pq.pop();
    }

    assert(pq.size() == 0);
}


int main(int argc, char** argv){
    srand(time(NULL));

    ManualTest();
    return 0;
    
    State_PQ pq;
    for(unsigned int i = 0; i < NUM_PUSH; i++){
        auto rs = getRandomRS();
        std::cout << "Pushing new state " << rs << std::endl;
        pq.push(rs);
    }

    for(unsigned int i = 0; i < NUM_PUSH; i++){
        auto state = pq.top();
        std::cout << "Current state: " << state << std::endl;
        pq.pop();
    }

    assert(pq.size() == 0);

    printf("PQTest exited normally\n");

    return 0;
}