#pragma once

#include "../pch.h"

#include <algorithm>
#include <functional>
#include <vector>

//DataType must support std::swap and copy-assignment
//maximizing queue
template<class DataType, class ComparatorType = std::less<DataType>>
class PriorityQueue{
private:
    //data members
    ComparatorType comparator; //called as less than
    std::vector<DataType> buffer;

    //helper functions
    unsigned int inline getParentUnsafe(const unsigned int i) const {return (i-1)/2;}
    unsigned int inline getLeftChidUnsafe(const unsigned int i) const {return (2*i)+1;}
    unsigned int inline getRightChildUnsafe(const unsigned int i) const {return (i+1)*2;}
    void inline upheap(const unsigned int i){
#ifdef DEBUG_4
        std::cout << "PQ upheap on " << i << std::endl;
#endif
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
        if(i >= buffer.size()){
            return;
        }

        unsigned int ri = getRightChildUnsafe(i);
        unsigned int li = getLeftChidUnsafe(i);

        if(ri < buffer.size()){
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
        }else if(li < buffer.size()){
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

    PriorityQueue() : comparator(){}

    PriorityQueue(const ComparatorType& c) {
        comparator = c;
    }

    ~PriorityQueue(){
        while(empty() == false){
            pop();
        }
    }

    unsigned int inline size(void) const {return buffer.size();}
    unsigned int inline empty(void) const {return buffer.size() == 0;}

    const DataType& top() const {
        if(buffer.size() <= 0){
            throw PriorityQueue::EmptyQueueException();
        }
        return buffer[0];
    }

    void pop(void) {
        if(buffer.size() <= 0){
            throw PriorityQueue::EmptyQueueException();
        } else if(buffer.size() == 1){
            buffer.pop_back();
            return;
        }

        buffer[0] = std::move(buffer.back());
        buffer.pop_back();
        downheap(0);
    }

    void push(const DataType& dt){
#ifdef DEBUG_4
        std::cout << "PQ Pushed new DataType: " << dt << std::endl;
#endif
        buffer.push_back(dt);
        upheap(buffer.size()-1);
    }

    const DataType& at(unsigned int i){
        if(buffer.size() <= 0){
            throw PriorityQueue::EmptyQueueException();
        }else if(i >= buffer.size()){
            //TODO - should be an out of bounds or something idk
            throw PriorityQueue::EmptyQueueException();
        }
        return buffer[i];
    }
};