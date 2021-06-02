#include <stdlib.h>
#include <iostream>
#include <stdio.h>
#include <functional>
#include <vector>
#include <thread>
#include <cmath>

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
    //bool emitterEnabled;
    
    //thread _emitter;
    vector<thread> _workers;
    vector<range> _ranges;
    //TODO blocking queue

    void init ()  {
        //initEmitter();
        initRanges();
        initWorkers();
    }

    void initRanges(){
        auto size = _n * _m;
        int workLoad = ceil(double(size) / double(_parallelism));
        pair<int, int> wl_blocco = make_pair(workLoad / _m, workLoad % _m);
        pair<int,int> index = make_pair(0,0);
        for (int k = 0; k < _parallelism; k++) {
            int i = index.first;
            int j = index.second;
            int new_j = (j - 1 + wl_blocco.second) % _m;
            int riporto = (j + wl_blocco.second) / _m;
            int new_i = (i + riporto + wl_blocco.first);
            if (new_i >= _n) {
                new_i = _n - 1;
                new_j = _m - 1;
            }
            cout << i << " " << j << endl;
            cout << new_i << " " << new_j << endl;
            index.first =  new_i;
            index.second = (new_j+1)%_m;
        }
    }

    /*void initEmitter(){
        _emitter = thread([&]{
            for(int i=0; i<_nIterations; i++){
                _ranges=assignRanges();
                //set start
                //barrier
                //write
            }
        });
    }*/

    void initWorkers(){
        
        _workers.push_back([](){
            for(int i=0; i<_parallelism-1; i++){
                T currState=getState();
                T[] neighbors=getNeighbors();
                _rule(neighbors);
                //barrier 
            }
        })
        
    }

    T[] getNeighbors(){

    }


    public:
    T** cellularAutomata(size_t n, size_t m, function<T(T, T[])> rule, T** initialState, size_t nIterations, size_t parallelism){
        _rule=rule;
        _n=n;
        _m=m;
        _matrix=initialState;
        //TODO: if less than 2 error
        _parallelism=parallelism;
        _nIterations=nIterations;
        _workers();
        _ranges();
        init();
    }

};