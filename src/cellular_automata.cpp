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
    function<T(T, vector<T*>)> _rule;
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
    
    vector<vector<vector<T>>> collectorBuffer;


    void init ()  {
        //initEmitter();
        initRanges();
        initEmitterCollector();  
    }

    void initEmitterCollector(){
        //TODO add emitter collector thread
        initWorkers();
        for (int k = 0; k < _nIterations; k++) { //for each iteration
            int c=0;
            for (int i = 0; i < _parallelism; i++) { //for each worker
                int cnum = collectorBuffer[i].size() - 1;
                while (cnum  < k) { //while the kth result isn't arrived wait
                    cnum = collectorBuffer[i].size() - 1;
                }
                for (auto x : collectorBuffer[i][k]) { 
                    //for each item in the buffer of the kth iteration of thread i
                    cout << x << " ";
                    c++; 
                    if(c%_m==0) //to write as a matrix
                        cout<<endl;
                }
            }
            cout << endl;
        }

        for (int i = 0; i < _parallelism; i++)
            _workers[i].join();
    }

    void initRanges(){
        //We cosider the matrix in base _m like decines and units (sequence of blocks)
        auto size = _n * _m; //matrix size
        int workLoad = ceil(double(size) / double(_parallelism)); //Workload size for each thread Upper bounded

        pair<int, int> wl_blocco = make_pair(workLoad / _m, workLoad % _m); //Workload size in the matrix
        pair<int,int> index = make_pair(0,0); // Initial index position
        for (int k = 0; k < _parallelism; k++) { //Loop assign ranges for each thread
            int i = index.first;
            int j = index.second;

            int endj = (((j - 1 + wl_blocco.second) % _m)+_m)%_m; //ending J 
            int carry = (j - 1 + wl_blocco.second) / _m; //carry as number of remaining cells
            int endi = (i + carry + wl_blocco.first); //ending i
            if (endi >= _n) { //foundamental 
                endi = _n - 1;
                endj = _m - 1;
            }
            _ranges[k] = {index, make_pair(endi, endj)}; // write the ranges of theads
            carry=0;
            index.second = (((endj+1)% _m)+_m)%_m; //move to the next cell, for the new range start
            carry = (endj+1) / _m;
            index.first =  endi + carry;
            
        }
    }

    void initWorkers(){
        for(size_t i=0; i<_parallelism; i++){
            _workers[i]=thread([=](){
                
                range r=_ranges[i];
                for(size_t j=0; j<_nIterations; j++){
                    vector<T> buffer;
                    for(pair<int, int> curr=r.start; curr <= r.end; increment(curr)){
                        //cout<<"thread: "<<i<<" start: "<<r.start.first<<","<<r.start.second<<" end: "<<r.end.first<<","<<r.end.second<<"curr: "<<curr.first<<","<<curr.second<<endl;
                        T currState=_matrix[curr.first][curr.second];
                        vector<T*> neighbors=getNeighbors(curr);
                        buffer.push_back(_rule(currState, neighbors));
                    }
                   
                    
                    //write buffer to the collector queue
                    collectorBuffer[i].push_back(buffer);
                    
                    //first barrier
                    computationBarrier();
                    
                    //write
                    writeBufferInMatrix(buffer, r);
                    
                    //second barrier
                    writingBarrier();
                    
                    
                }
            });
        }
    }

    void computationBarrier(){
        unique_lock<mutex> lock1(b1Mutex);
        //counter increment
        b1++;
        b1Condition.wait(lock1, [&]() {
            return !(b1 != 0 && b1 != _parallelism);
            });
        if (b1 == _parallelism) {
            //cout << "Sono l'ultimo thread: " << i << endl;
            b1 = 0;
            b1Condition.notify_all();
        }
    }

    void writingBarrier(){
        unique_lock<mutex> lock2(b2Mutex);
        //counter increment
        b2++;
        b2Condition.wait(lock2, [&]() {
            return !(b2 != 0 && b2 != _parallelism);
            });
        if (b2 == _parallelism) {
            b2 = 0;
            b2Condition.notify_all();
        }
    }

    void writeBufferInMatrix(vector<T>& buffer, range r){
        size_t k=0;
        for(pair<int, int> curr = r.start; curr<=r.end; increment(curr)){
            _matrix[curr.first][curr.second] = buffer[k];
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

    vector<T*> getNeighbors(pair<int,int> centre_index){
        vector<T*> neighborhood(8);
        int neigh_num = 0;
    
        int n = _n;
        int m = _m;
        int i = centre_index.first;
        int j = centre_index.second;

        int init_i = (((i - 1) % n)+n)%n;
        int init_j = (((j - 1) % m)+m)%m;

        for (int q = init_i; q < init_i + 3; q++) {
            for (int z = init_j; z < init_j + 3; z++) {
                int qa = (((q % n) + n) % n);
                int za = (((z % m) + m) % m);
                if (qa != i || za != j) {
                    //matrix[qa][za] = 1; //only for test
                    neighborhood[neigh_num++] = &(_matrix[qa][za]);

                }
            }
        }
        return neighborhood;
    }


    public:
    CellularAutomata(size_t n, size_t m, function<T(T, vector<T*>)> rule, vector<vector<T>>& initialState, size_t nIterations, size_t parallelism){
        _rule=rule;
        _n=n;
        _m=m;
        _matrix=initialState;
        //TODO: if less than 2 error
        _parallelism=parallelism;
        _nIterations=nIterations;
        _workers=vector<thread>(_parallelism);
        _ranges=vector<range>(_parallelism);
        collectorBuffer= vector<vector<vector<T>>>(_parallelism, vector<vector<T>>());
        init();
    }

};