#include "pch.h"

#include <vector>
#include <queue>

#include "RecomputeState.h"

#define QTY_BUCKETS 40

template <class Comparator>
class PQWrapper {
private:
    unsigned int buckets[QTY_BUCKETS];
    double efficiencySum;
    unsigned int totalItems;
    std::priority_queue<RecomputeState, std::vector<RecomputeState>, Comparator> pq;

    inline unsigned int getBucket(const RecomputeState& t) const {
        unsigned int i = (t.getEfficiency()*QTY_BUCKETS)/2;
        if(i >= QTY_BUCKETS){
            i = QTY_BUCKETS-1;
        }
        return i;}
public:
    PQWrapper(void) : pq(){
        for(unsigned int i =0 ; i < QTY_BUCKETS; i++){
            buckets[i] = 0;
        }

        efficiencySum = 0;
        totalItems = 0;
    }

    inline long long unsigned int size() const {return pq.size();}
    inline bool empty() const {return pq.empty();}

    inline const RecomputeState& top() const {return pq.top();}

    void push(const RecomputeState& t){
        //get bucket and update
        buckets[getBucket(t)]++;
        efficiencySum += t.getEfficiency();
        totalItems++;

        //update pq
        pq.push(t);
    }

    void pop(){
        const RecomputeState& t = pq.top();

        //get bucket and decrement
        buckets[getBucket(t)]--;
        efficiencySum -= t.getEfficiency();
        totalItems--;

        //update pq
        pq.pop();
    }

    inline double getAverageEff() const {return efficiencySum/totalItems;}

    void printBuckets(std::ostream& os) const {
        os << "[";
        for(int i =0; i < QTY_BUCKETS; i++){
            if(i != 0){
                os << ", ";
            }
            os << buckets[i];
        }
        os << "]";
    }
};