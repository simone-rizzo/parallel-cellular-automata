#include <stdlib.h>
#include <iostream>
#include <stdio.h>
#include <functional>
#include <vector>
#include <thread>
#include <cmath>
#include <mutex>
#include <condition_variable>
#include <algorithm>
#include <string>
#include <cstring>
#define cimg_use_png

#include "./cimg/CImg.h"
#include <cstdint>
#include <cassert>
#include "utimer.cpp"

//#include <ff/ff.hpp>
//#include <ff/barrier.hpp>

//#define PARALLEL_WRITE //writes the images in parallel
//#define LAST_WRITE //write the result image

using namespace std;
using namespace cimg_library;

//Class that implement the Cellular automata modelling paradigm.
template <class T>
class CellularAutomata{
        
    typedef struct {
        int start;
        int end;
    } range;
    //ff::ffBarrier ba; //the FF barrier
    int _n; //number of rows
    int _m; //number of columns
    vector<vector<T>> matrices; //the two matrices that will rotate
    function<T(vector<T>&, int const&, int const&, int const&)> _rule;
    function<unsigned char(T const&)> _getCimgStateRepr;
    int _nIterations; //number of columns
    int _parallelism; //parDegree
    vector<thread> _workers; //list of workers
    vector<range> ranges; //list of ranges to be assigned at each worker
    mutex bMutex; 
    int count=0;
    bool global = 0;
    vector<CImg<unsigned char>> images;

    /**
    * Computes the ranges that will be assigned to each worker
    * */
    void initRanges(){
        int delta { _n*_m / _parallelism };
        for(int i=0; i<_parallelism; i++) { // split the board into peaces
            ranges[i].start = i*delta;
            ranges[i].end   = (i != (_parallelism-1) ? (i+1)*delta : _n*_m); 
        }        
    }

   
    public:
    CellularAutomata(vector<T>& initialState, 
                    int n, int m, 
                    function<T(vector<T>&, int const&, int const&, int const&)> rule, 
                    function<unsigned char(T const&)> getCimgStateRepr,
                    int nIterations,  int parallelism){
        _n=n;
        _m=m;
        matrices = vector<vector<T>>(2, initialState);
        _rule=rule;
        _nIterations=nIterations;
        _parallelism = parallelism;
        _getCimgStateRepr=getCimgStateRepr;
        _workers=vector<thread>(_parallelism);

        images = vector<CImg<unsigned char>>(nIterations,CImg<unsigned char>(_n,_m));
        
        ranges= vector<range>(_parallelism);
        //ba.barrierSetup(_parallelism);
        initRanges();
    }

    public:
    void run(){  
        
        //used for the barrier 
        
        for(int i=0;i<_parallelism;i++){    
            _workers[i]=thread([=](int start, int end){
                bool local = 0;                        
                bool index=0;  
                for(int j=0;j<_nIterations;j++){                    
                    for(int k = start; k < end; k++){
                        T res=_rule(matrices[index], k, _n, _m);
                        matrices[!index][k]=res; 
                        images[j](k/_m, k%_m) = _getCimgStateRepr(res);
                    }     

                    //Barrier -------------------
                    //ba.doBarrier(i); //Barrier
                    local = !local;            
                    unique_lock<mutex> lock(bMutex);
                    count++;
                    if(count==_parallelism){
                        count=0;
                        global=local;
                    }
                    lock.unlock();
                    while(global!=local){}
                    //End barrier ---------------
                    
                    #ifndef PARALLEL_WRITE
                    if(i==0){
                        string filename="./frames/"+to_string(j)+".png";
                        char name[filename.size()+1];
                        strcpy(name, filename.c_str());
                        images[j].save(name);
                    }
                    #endif
                    index=!index; //change the index of the matrix
                }
                //parallel write
                #ifdef PARALLEL_WRITE
                int nprint=ceil(double(_nIterations) / double(_parallelism));
                int wstart=i*nprint;
                int wend= min(int(_nIterations), (int(i)+1) * nprint);
                for(int k=wstart; k<wend; k++){
                    string path="./frames/"+to_string(k)+".png";
                    char name[path.size()+1];
                    strcpy(name, path.c_str());
                    images[k].save(name);
                }
                #endif                
            }, ranges[i].start, ranges[i].end);
        }
        for (int i = 0; i < _parallelism; i++){
                _workers[i].join();
        }
    }    
};

//Simpler rule Taken from Game of Life by Conways
int rule(vector<int>& matrix, int const& index, int const& n, int const& m){
    int row = index/m;
    int col = index%m;
    int sum = (row == 0) ? 0: matrix[(row - 1)*m + col];                            //up
    sum += (row == 0 || col == 0) ? 0: matrix[(row - 1)*m + col - 1];               //up_left   
    sum += (row == 0 || col == m - 1) ? 0: matrix[(row - 1)*m + col + 1];          //up_right       
    sum += (row == n - 1) ? 0: matrix[(row + 1)*m + col];                          //down
    sum += (col == 0) ? 0: matrix[(row)*m + col - 1];                               //left
    sum += (col == m - 1) ? 0: matrix[(row)*m + col + 1];                          //right
    sum += ((row == n - 1) || col == 0) ? 0: matrix[(row + 1)*m + col - 1];        //down_left
    sum += ((row == n - 1) || col == m - 1) ? 0: matrix[(row + 1)*m + col + 1];   //down_right
    int s=matrix[index];
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

unsigned char state(int const& s){
    return s==0 ? 0 : 255;
}
int square__init(){
    return (rand())%2;
}

int main(int argc, char* argv[]){
    if(argc != 5) {
        std::cout << "Usage is: " << argv[0] << " N M number_step number_worker" << std::endl;
        return(-1);
    }
    int n = atoi(argv[1]);
    int m = atoi(argv[2]);
    int iter = atoi(argv[3]);
    int nw = atoi(argv[4]);

    srand(0);
    vector<int> matrix (n*m);  
    std::generate(matrix.begin(), matrix.end(), square__init); 
    
    CellularAutomata<int> ca(
        matrix, n,m, 
        rule, 
        state,
        iter,
        nw
    );       
    utimer tp("completion time");
    ca.run();
    return 0;

}