#include <stdlib.h>
#include <iostream>
#include <stdio.h>
#include <functional>
#include <vector>
#include <thread>
#include <cmath>
#include <mutex>
#include <condition_variable>


using namespace std;

struct range{
    pair<int, int> start;
    pair<int, int> end;

    
};

template<class T>
class CellularAutomata{
    
    size_t _n;
    size_t _m;
    vector<vector<T>> _matrix;
    function<T(T, T[])> _rule;
    size_t _parallelism;
    size_t _nIterations;
    //bool emitterEnabled;
    
    //thread _emitter;
    vector<thread> _workers;
    vector<range> _ranges;
    int b1=0;
    mutex b1Mutex;
    condition_variable b1Condition;
    int b2=0;
    mutex b2Mutex;
    condition_variable b2Condition;
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
            //cout << i << " " << j << endl;
            //cout << new_i << " " << new_j << endl;
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
        for(size_t i=0; i<_parallelism; i++){
            _workers[i]=thread([&](){
                vector<T> buffer;
                range r=_ranges[i];
                for(size_t j=0; j<_nIterations; j++){
                    for(pair<int, int> curr=r.start; curr<r.end; increment(curr)){
                        T currState=_matrix[curr.first][curr.second];
                        vector<T*> neighbors=getNeighbors();
                        buffer.push_back(_rule(currState, neighbors));
                    }
                   
                    //barrier
                    unique_lock<mutex> lock1(b1Mutex);
                    //counter increment
                    b1++;
                    b1Condition.wait(lock1, [&]{
                            return b1>0 && b1<_parallelism;
                        });
                    if(b1==_parallelism) b1=0;
                    b1Condition.notify_all();
                    lock1.release(); 
                    //write
                    writeBuffer(buffer, r);
                    //barrier
                    unique_lock<mutex> lock2(b2Mutex);
                    //counter increment
                    b2++;
                    b2Condition.wait(lock2, [this]{
                            return b2>0 && b2<_parallelism;
                        });
                    if(b2==_parallelism) b2=0;
                    b2Condition.notify_all();
                    lock2.release(); 
                }
            });
        }
    }

    bool allDone(){
        for(auto it:_barrier){
            if(it==false) return false;
        }
        return true;
    }

    void writeBuffer(vector<T>& buffer, range r){
        size_t k=0;
        for(pair<int, int> curr=r.start; curr<r.end; increment(curr)){
            T currState=getState();
            vector<T*> neighbors=getNeighbors();
            buffer[k]=_rule(neighbors);
            k++;
        }
    }

    void increment(pair<int, int>& pair){
        int j=(pair.second + 1) % _m;
        int r=(pair.second + 1) / _m;
        int i=pair.first + r;
        pair.first=i;
        pair.second=j;
    }

    /*vector<T> getNeighbors(){
        vector<T*> neighborhood(8);
        int neighbors_num = 0;

        //int n = 4;
        int init_i = (((i - 1) % n)+n)%n;
        int init_j = (((j - 1) % n)+n)%n;

        for (int q = init_i; q < init_i + 3; q++) {
            for (int z = init_j; z < init_j + 3; z++) {
                int qa = (((q % n) + n) % n);
                int za = (((z % n) + n) % n);
                if (qa != i || za != j) {
                    //matrix[qa][za] = 1; //only for test
                    neighborhood[neigh_num] = (matrix[qa][za]);
                }
            }
        }
        return neighborhood;
    }*/


    public:
    vector<vector<T>> cellularAutomata(size_t n, size_t m, function<T(T, vector<T*>)> rule, vector<vector<T>> initialState, size_t nIterations, size_t parallelism){
        _rule=rule;
        _n=n;
        _m=m;
        _matrix=initialState;
        //TODO: if less than 2 error
        _parallelism=parallelism;
        _nIterations=nIterations;
        _workers(parallelism);
        _ranges(parallelism);
        _barrirer(0);
        init();
    }

};