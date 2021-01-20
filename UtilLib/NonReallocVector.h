#pragma once

#include "../pch.h"

#include <vector>

#define VECTOR_SHARD_SIZE 1024

template <class T>
class NonReallocVector{
private:
    template <class TT>
    class NonReallocShard{
    private:
        T buffer[VECTOR_SHARD_SIZE];
    public:
        NonReallocShard(void){};
        // inline bool isFull(void) const {return num >= VECTOR_SHARD_SIZE;}
        // inline bool isEmpty(void) const {return num == 0;}
        TT* push(const TT& t, unsigned int num){
            // assert(isFull() == false);
            buffer[num] = t;
            return buffer + num;
        }
    };

    std::vector<NonReallocShard<T>*> shardList;
    unsigned int tailNum;

    inline bool isTailShardFull(void){
        return tailNum == VECTOR_SHARD_SIZE;
    }

    inline bool isTailShardEmpty(void){
        return tailNum == 0;
    }
public:
    NonReallocVector() : tailNum(VECTOR_SHARD_SIZE){};
    ~NonReallocVector(){
        for(auto ptr : shardList){
            delete ptr;
        }
    }
    T* push(const T& t){
        if(isTailShardFull()){
            shardList.push_back(new NonReallocShard<T>());
            tailNum = 0;
        }
        return shardList.back()->push(t, tailNum++);
    }
    
    void popLast(){
        if(isTailShardEmpty()){
            delete shardList.back();
            shardList.pop_back();
            tailNum = VECTOR_SHARD_SIZE;
        }
        tailNum--;
    }
};