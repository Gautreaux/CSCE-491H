#pragma once

#include "../pch.h"

#include <algorithm>
#include <functional>

//DataType must support std::swap and copy-assignment
//maximizing queue
template<class DataType, class ComparatorType = std::less<DataType>>
class PriorityQueue{
private:
    //data members
    ComparatorType comparator; //called as less than
    DataType* buffer;
    unsigned int maxSize;
    unsigned int currentSize;

    //helper functions
    unsigned int inline getParentUnsafe(const unsigned int i) const {return (i-1)/2;}
    unsigned int inline getLeftChidUnsafe(const unsigned int i) const {return (2*i)+1;}
    unsigned int inline getRightChildUnsafe(const unsigned int i) const {return (i+1)*2;}
    void inline upheap(const unsigned int i){
        if(i == 0){
            return;
        }
        unsigned int pi = getParentUnsafe(i);
        assert(pi >= 0);
        if(comparator(buffer[pi], buffer[i])){
            std::swap(buffer[i], buffer[pi]);
            upheap(pi);
        }
    }
    void inline downheap(const unsigned int i){
        if(i >= currentSize){
            return;
        }

        unsigned int ri = getRightChildUnsafe(i);
        unsigned int li = getLeftChidUnsafe(i);

        if(ri < currentSize){
            if(comparator(buffer[i], buffer[li]) && comparator(buffer[i], buffer[ri])){
                //i < li and i < ri but ri ?<? li
                if(comparator(buffer[li], buffer[ri])){
                    // i < li < ri
                    std::swap(buffer[i], buffer[ri]);
                    downheap(ri);
                }else{
                    //THIS MIGHT NOT BE STABLE?
                    // i < ri <= li
                    std::swap(buffer[i], buffer[li]);
                    downheap(li);
                }

            }else if(comparator(buffer[i], buffer[li])){
                //ri < i < li, swap i and li
                std::swap(buffer[i], buffer[li]);
                downheap(li);
            }else if(comparator(buffer[i], buffer[ri])){
                //li < i < ri, swap i and ri
                std::swap(buffer[i], buffer[ri]);
                downheap(ri);
            }else{
                //li < i and ri < i but ri ?<? li
                //do nothing
            }
        }else if(li < currentSize){
            // only li is valid
            if(comparator(buffer[i], buffer[li])){
                std::swap(buffer[i], buffer[li]);
                //we already know li is the trailing element so no need to downheap
            }
        }else{
            //no children for this node
        }
    }

public:
    class EmptyQueueException : public std::exception{
        // private:
        //     std::string message;
        // public:
        //     EmptyQueueException(const std::string& msg) : message(msg) {};
        //     const std::string& getMessage(void) {return message;}
    };

    PriorityQueue() : comparator() , maxSize(0), currentSize(0), buffer(nullptr){}

    PriorityQueue(const ComparatorType& c) : maxSize(0), currentSize(0), buffer(nullptr) {
        comparator = c;
    }

    unsigned int inline size(void) const {return currentSize;}
    unsigned int inline empty(void) const {return currentSize == 0;}

    const DataType& top() const {
        if(currentSize <= 0){
            throw PriorityQueue::EmptyQueueException();
        }
        return buffer[0];
    }

    void pop(void) {
        if(currentSize <= 0){
            throw PriorityQueue::EmptyQueueException();
        } else if(currentSize == 1){
            currentSize = 0;
            return;
        }

        buffer[0] = buffer[currentSize];
        currentSize--;
        downheap(0);
    }

    void push(const DataType& dt){
        if(currentSize == maxSize){
            //realloc
            unsigned int newSize = (currentSize == 0 ? 1 : currentSize);
            newSize *= 2;
            auto t = realloc(buffer, sizeof(DataType)*newSize);
            if(t == nullptr){
#ifdef DEBUG
                printf("realloc failed for target size %d\n", newSize);
#endif
                throw std::runtime_error("Realloc failed during priority queue push");
            }
            buffer = (DataType*)t;
            maxSize = newSize;
        }
        buffer[currentSize++] = dt;
    }

    const DataType& at(unsigned int i){
        if(currentSize <= 0){
            throw PriorityQueue::EmptyQueueException();
        }else if(i >= currentSize){
            //TODO - should be an out of bounds or something idk
            throw PriorityQueue::EmptyQueueException();
        }
        return buffer[i];
    }
};