

#include <fstream>
#include <iostream>
#include <queue>
#include <tuple>
#include <vector>

//Just for temporary testing
#include <random>
#include <time.h>

#include "GeometryLib/LineSegment.h"

using namespace std;

using TupleType = tuple<unsigned int, unsigned int, unsigned int>;

const string filePath = "81191.0.350000.chaindump";

Point3 readSomePoint(ifstream& is){
    char c;
    while(is >> c){
        if(c == '(') //)
        {
            //this is a valid point
            //construct the point and return it
            double x,y,z;
            
            //extra c reads are for the commas and the parens
            is >> x >> c >> y >> c >> z >> c;
            
            return Point3(x,y,z);
        }
        else if(c == 'N')
        {
            //this is a NOOP
            is >> c >> c >> c; //remainder of NOOP
            return Point3::ANY;
        }
    }

    throw "Illegal file format.";
}

class State{
public:
    Point3 a1Pos;
    Point3 a2Pos;
    unsigned int a1IncidentNOOP;
    unsigned int a2IncidentNOOP;
    unsigned int transitionTime;
    unsigned int partnerIndex;
    unsigned int chainGroup;
    bool hasBeenUsed;
};

ostream& operator<<(ostream& os, const State& state){
    os << state.a1Pos << " ";
    os << state.a2Pos << " ";
    os << state.transitionTime << " ";
    os << state.partnerIndex << " ";
    os << ((state.hasBeenUsed) ? "USED" : "FREE");

    return os;
}

unsigned int getStateTransitionTime(const State& s1, const State& s2){
    //Last thing to do and research could be done
    //TODO
    //TODO
    //TODO
    return rand() % 25;
}

int main(int argc, char** argv){
    srand(time(nullptr));

    cout << "Hello" << endl;


    ifstream ifs(filePath);

    if(ifs.is_open() == false){
        cout << "Could not open file: " << filePath << endl;
        exit(1);
    }else{
        cout << "File opened successfully. " << endl;
    }

    vector<State> stateList;
    unsigned int numChainPairs = 0;
    unsigned int baseTransitionTime = 0;

    while(ifs){
        unsigned int chainLen;
        ifs >> chainLen;

        if(!ifs){
            break;
        }

        Point3 a1Start = readSomePoint(ifs);
        Point3 a1End = readSomePoint(ifs);
        Point3 a2Start = readSomePoint(ifs);
        Point3 a2End = readSomePoint(ifs);

        // cout << chainLen << " " << a1Start << endl;

        State s1;
        State s2;
        s1.a1Pos = a1Start;
        s2.a1Pos = a1End;
        s1.a2Pos = a2Start;
        s2.a2Pos = a2End;

        s1.transitionTime = chainLen;
        s2.transitionTime = chainLen;

        s1.partnerIndex = stateList.size() + 1;
        s2.partnerIndex = s1.partnerIndex - 1;
        

        s1.hasBeenUsed = false;
        s2.hasBeenUsed = false;

        if(a1Start == Point3::ANY){
            //a1 is no-oping, so leading NOOPs is chainLen
            s1.a1IncidentNOOP = chainLen;
            s2.a1IncidentNOOP = chainLen;
        }

        if(a2Start == Point3::ANY){
            s1.a2IncidentNOOP = chainLen;
            s2.a2IncidentNOOP = chainLen;
        }

        s1.chainGroup = numChainPairs;
        s2.chainGroup = numChainPairs;

        stateList.push_back(s1);
        stateList.push_back(s2);

        cout << "Added new state: " << s1 << endl;
        cout << "Added new state: " << s2 << endl;

        //stats
        numChainPairs++;
        baseTransitionTime+=chainLen;
    }

    cout << "Extracted " << numChainPairs << " chain pairs from file. " << endl;
    cout << "Extracted " << stateList.size() << " states from file." << endl;

    priority_queue<TupleType> pq;

    for(unsigned int i = 0; i < stateList.size(); i++){
        for(unsigned int j = i + 1; j < stateList.size(); j++){
            if(stateList.at(i).partnerIndex == j){
                assert(stateList.at(j).partnerIndex == i);
                continue;
            }

            auto t = getStateTransitionTime(stateList.at(i), stateList.at(j));
            pq.emplace(-t, -i, -j);
        }
    }

    cout << "Precompute done, now linking. Precompute QTY: " << pq.size() << endl;

    unsigned int statesUsed = 0;
    while (statesUsed < (stateList.size() - 2))
    {
        if(pq.empty()){
            throw "This is unreachable?";
        }

        auto& t = pq.top();
        const auto transitionTime = -std::get<0>(t);
        const auto state1Index = -std::get<1>(t);
        const auto state2Index = -std::get<2>(t);
        State& state1 = stateList.at(-std::get<1>(t));
        State& state2 = stateList.at(-std::get<2>(t));
        pq.pop();

        if(state1.hasBeenUsed){
            continue;
        }
        if(state2.hasBeenUsed){
            continue;
        }
        if(state1.chainGroup == state2.chainGroup){
            continue;
        }

        statesUsed += 2;

        cout << "Linking states " << state1Index << " " << state2Index << endl;
        cout << "Partners: " << state1.partnerIndex << " " << state2.partnerIndex << endl;

        State& state1Partner = stateList.at(state1.partnerIndex);
        State& state2Partner = stateList.at(state2.partnerIndex);

        assert(state1Partner.hasBeenUsed == false);
        assert(state2Partner.hasBeenUsed == false);
        
        state1.hasBeenUsed = true;
        state2.hasBeenUsed = true;

        // state1.chainGroup = min(state1.chainGroup, state2.chainGroup);
        state2.chainGroup = state1.chainGroup;
        state1Partner.chainGroup = state1.chainGroup;
        state2Partner.chainGroup = state2.chainGroup;

        state1Partner.transitionTime = state1.transitionTime + state2.transitionTime + transitionTime;
        state2Partner.transitionTime = state1Partner.transitionTime;

        state1Partner.partnerIndex = state2.partnerIndex;
        state2Partner.partnerIndex = state1.partnerIndex;
    }

    cout << "Linking finished" << endl;

    //now we do some debugging checks;
    vector<unsigned int> unusedStateIndexes;
    for(unsigned int i = 0; i < stateList.size(); i++){
        if(stateList.at(i).hasBeenUsed == false){
            unusedStateIndexes.push_back(i);
        }
    }
    
    assert(unusedStateIndexes.size() == 2);
    cout << "Unused state indexes: " << unusedStateIndexes[0] << " " << unusedStateIndexes[1] << endl;

    State& s1 = stateList[unusedStateIndexes[0]];
    State& s2 = stateList[unusedStateIndexes[1]];

    cout << "Partner indexes: " << s1.partnerIndex << " " << s2.partnerIndex << endl;

    assert(s1.partnerIndex == unusedStateIndexes[1]);
    assert(s2.partnerIndex == unusedStateIndexes[0]);

    assert(s1.transitionTime == s2.transitionTime);
    assert(s1.transitionTime >= baseTransitionTime);

    cout << "Resolved chain linking" << endl;
    cout << "Base Time: " << baseTransitionTime << endl;
    cout << "Transition time: " << s1.transitionTime - baseTransitionTime << endl;
    cout << "Total time: " << s1.transitionTime << endl;

    cout << "Exited Normally" << endl;
}