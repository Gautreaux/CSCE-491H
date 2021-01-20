#pragma once

#include "../pch.h"

#include <map>

using std::map;

template<class A, class B>
class BiMap{
//far from the most efficient but whatever
private:
    map<A, B> mapAtoB;
    map<B, A> mapBtoA;
public:
    BiMap(){
        // implicitly calls default constructor for the maps
    }

    void insert(const A a, const B b){
        mapAtoB.insert(std::pair<A, B>(a,b));
        mapBtoA.insert(std::pair<B, A>(b,a));
    }

    typename map<A,B>::const_iterator findByA(const A a) const {
        return mapAtoB.find(a);
    }

    typename map<B,A>::const_iterator findByB(const B b) const {
        return mapBtoA.find(b);
    }

    int countByA(const A a){
        return mapAtoB.count(a);
    }

    int countByB(const B b){
        return mapBtoA.count(b);
    }

    typename map<A,B>::iterator findByABegin(void){
        return mapAtoB.begin();
    }

    typename map<A,B>::iterator findByAEnd(void){
        return mapAtoB.end();
    }

    typename map<B, A>::iterator findByBBegin(void){
        return mapBtoA.begin();
    }

    typename map<B, A>::iterator findByBEnd(void){
        return mapBtoA.end();
    }

    typename map<A,B>::const_iterator findByABegin(void) const {
        return mapAtoB.begin();
    }

    typename map<A, B>::const_iterator findByAEnd(void) const {
        return mapAtoB.end();
    }

    typename map<B, A>::const_iterator findByBBegin(void) const {
        return mapBtoA.begin();
    }

    typename map<B, A>::const_iterator findByBEnd(void) const {
        return mapBtoA.end();
    }


    //TODO - add removeByA, removeByB
};
