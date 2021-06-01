#include <stdlib.h>
#include <iostream>
#include <stdio.h>
#include <functional>
#include <vector>
#include <thread>

using namespace std;

struct range{
    pair<int, int> start;
    pair<int, int> end;


};

template<class T>
class CellularAutomata{
    
    size_t _n;
    size_t _m;
    T** _matrix;
    function<T(T, T[])> _rule;
    size_t _parallelism;
    size_t _nIterations;
    bool emitterEnabled;
    
    thread _emitter;
    vector<thread> _workers;
    vector<range> _ranges;
    //TODO blocking queue

    void init ()  {
        //initEmitter();
        initWorkers();
    }

    void initEmitter(){
        _emitter = thread([&]{
            for(int i=0; i<_nIterations; i++){
                _ranges=assignRanges();
                //set start
                //barrier
                //write
            }
        });
    }

    void initWorkers(){
        
        _workers.push_back([](){
            for(int i=0; i<_parallelism-1; i++){
                T currState=getState();
                T[] neighbors=getNeighbors();
                _rule(neighbors);
            }
        })
        
    }

    T[] getNeighbors(){

    }


    public:
    T** cellularAutomata(size_t n, size_t m, function<T(T, T[])> rule, T** initialState, size_t nIterations, size_t parallelism) : _workers(parallelism){
        _rule=rule;
        _n=n;
        _m=m;
        _matrix=initialState;
        //TODO: if less than 2 error
        _parallelism=parallelism;
        _nIterations=nIterations;
    }

};