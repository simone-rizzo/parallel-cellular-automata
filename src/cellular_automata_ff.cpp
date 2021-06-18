#include <ff/ff.hpp>
#include <iostream>

#include <stdlib.h>
#include <iostream>
#include <stdio.h>
#include <functional>
#include <vector>
#include <thread>
#include <cmath>
#include <mutex>
#include <condition_variable>
#include "barrier.cpp"
#include <algorithm>
#include <fstream>
#include <ff/parallel_for.hpp>

#define cimg_use_jpeg
#include "./cimg/CImg.h"
#include "utimer.cpp"

using namespace std;
using namespace cimg_library;

struct segment{
    pair<int, int> start;
    pair<int, int> end;
};

template<class T>
class CellularAutomata{
    
    size_t _n;
    size_t _m;
    vector<vector<vector<T>>> _matrix;
    function<T(T, vector<T*>)> _rule;
    size_t _parallelism;
    size_t _nIterations;
    //bool emitterEnabled;
    
    //thread _emitter;
    vector<thread> _workers;
    vector<segment> _ranges;
    /*int b1=0;
    mutex b1Mutex;
    condition_variable b1Condition;*/
    Barrier b1;
    Barrier b2;
    /*int b2=0;
    mutex b2Mutex;
    condition_variable b2Condition;*/

    ff::ParallelFor* pf;
    
    vector<vector<vector<T>>> collectorBuffer;


    void init ()  {
        //initEmitter();
        initRanges();
        initEmitterCollector();  
    }

    void initEmitterCollector(){
        //TODO add emitter collector thread
        initWorkers();
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

        bool b=0;
        for(int f=0;f<_nIterations;f++){
            (*pf).parallel_for_static(0,_ranges.size(),1,0,[=](const long i) {
                segment r=_ranges[i];
                vector<T> buffer;
                for(pair<int, int> curr=r.start; curr <= r.end; increment(curr)){                        
                        T currState=_matrix[b][curr.first][curr.second];
                        vector<T*> neighbors=getNeighbors(curr, b);
                        int new_value = _rule(currState, neighbors);
                        _matrix[!b][curr.first][curr.second]=new_value;
                        buffer.push_back(_rule(currState, neighbors));           
                    }                
                collectorBuffer[i].push_back(buffer);               
            },_parallelism);
            b=!b;
        }
        (*pf).parallel_for_static(0,_nIterations,1,0,[=](const long i) {
             CImg<unsigned char> img(_n,_m); //create new image                    
                    int c=0;
                    for (int j = 0; j < _parallelism; j++) { //for each worker
                        for (auto x : collectorBuffer[j][i]) { 
                            //for each item in the buffer of the kth iteration of thread i
                            img(c/_m,c%_m)=x>0?255:0;
                            c++; 
                        }
                    }    
                    string filename="./frames/"+to_string(i)+".png";
                    char b[filename.size()+1];
                    strcpy(b, filename.c_str());
                    //img.save_png(b);     
        },_parallelism);
        
    }

    void writeBufferInMatrix(vector<T>& buffer, segment r){
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

    vector<T*> getNeighbors(pair<int,int> centre_index, int b){
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
                    neighborhood[neigh_num++] = &(_matrix[b][qa][za]);

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
        _matrix=vector<vector<vector<int>>>(2, initialState);
        //TODO: if less than 2 error
        _parallelism=parallelism;
        _nIterations=nIterations;
        _workers=vector<thread>(_parallelism);
        _ranges=vector<segment>(_parallelism);
        pf = new ff::ParallelFor(_parallelism);        
        (*pf).disableScheduler(true);
        collectorBuffer= vector<vector<vector<T>>>(_parallelism, vector<vector<T>>());
        init();
    }

};



void init_matrix(vector<vector<int>>& matrix, int n, int m)
{
    for(int i=0;i<n;i++){
        for(int j=0;j<m;j++){
           int v1 = rand() % 10;
           if(v1>6){
               matrix[i][j]=1;
           } 
           else{
               matrix[i][j]=0;
           }
        }
    }
}

int rule(int s, vector<int*> vect){
    int sum = 0;
    for(int i=0; i<vect.size();i++)
    {
        sum += *vect[i];
    }
    if(sum==3)
    {
        return 1;
    }
    if(s==1 && (sum==3 || sum==2))
    {
        return s;
    }
    if((sum == 0 || sum == 1)|| sum >3)
    {
        return 0;
    }
    return 0;
}


int main(int argc, char *argv[]){ 
    utimer tp("Completion time");
    int n=100;
    int m=100;
    int iter = 400;
    int nw = 12;
    if(argc>1){
        n = atoi(argv[1]);
        m = atoi(argv[2]);
        iter = atoi(argv[3]);
        nw = atoi(argv[4]);
    }
    vector<vector<int>> matrix(n,vector<int>(m));
    init_matrix(matrix,n,m);
    function<int(int,vector<int*>)> f = rule;
    CellularAutomata<int> mcA(n, m, f,
       matrix,
        iter,
        nw
    );
}

    /*int m=10;
    vector<vector<int>> A(2,vector<int>(m,1));
    ff::ParallelFor pf(8);
    bool b=0;
    pf.disableScheduler(true);
    for(int f=0;f<4;f++){
        pf.parallel_for_static(0,m,1,0,[&A, &m, &b](const long i) {
            std::thread::id this_id = std::this_thread::get_id();
            A[!b][i] = A[b][0]+A[b][(((i-1)%m)+m)%m]+A[b][(((i+1)%m)+m)%m];            
        },8);
        pf.parallel_for_static(0,m,1,0,[&A, &m, &b](const long i) {
            string out = to_string(A[!b][i])+" ";
            cout<<out;         
        },8);
        cout<<endl;
        b=!b;
    }*/