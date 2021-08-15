#include <ff/ff.hpp>
#include <ff/parallel_for.hpp>
#include <ff/barrier.hpp>
#include <stdlib.h>
#include <iostream>
#include <stdio.h>
#include <functional>
#include <vector>
#include <thread>
#include <cmath>
#include <algorithm>
#include <string>
#include <cstring>
#define cimg_use_png

#include "./cimg/CImg.h"
#include <cstdint>
#include <cassert>
#include "utimer.cpp"

using namespace std;
using namespace cimg_library;

//Class that implement the Cellular automata modelling paradigm.
template <class T>
class CellularAutomata{
        
    typedef struct {
        int start;
        int end;
    } range;

    int _n; //number of rows
    int _m; //number of columns
    vector<vector<T>> matrices; //the two matrix
    function<T(vector<T>&, int const&, int const&, int const&)> _rule;
    function<unsigned char(T const&)> _getCimgStateRepr;
    int _nIterations; //number of columns
    int _parallelism; //parDegree
    vector<thread> _workers; //list of workers
    vector<range> ranges; //list of ranges to be assigned at each worker
   
    /*#ifdef PARALLEL_WRITE
    vector<CImg<unsigned char>> images;
    #endif
    #ifndef PARALLEL_WRITE
    CImg<unsigned char> img; //used for sequential write
    #endif
    */
    vector<CImg<unsigned char>> images;

    ff::ffBarrier ba; //the FF barrier
    ff::ParallelFor* pf; //Parallel for reference
    


    

    void initRanges(){
        int delta { int(_n)*int(_m) / int(_parallelism) };
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
        /*#ifdef PARALLEL_WRITE
        images = vector<CImg<unsigned char>>(nIterations,CImg<unsigned char>(_n,_m));
        #endif
        #ifndef PARALLEL_WRITE
        img=CImg<unsigned char>(_n,_m); //used for sequential write
        #endif
        */
        images = vector<CImg<unsigned char>>(nIterations,CImg<unsigned char>(_n,_m));
        ranges= vector<range>(_parallelism);
        initRanges();

        ba.barrierSetup(_parallelism);
        pf = new ff::ParallelFor(_parallelism);        
        (*pf).disableScheduler(true); //disabled  
    }

    public:
    void run(){ 
        (*pf).parallel_for_thid(0,ranges.size(),1,0,[&](const long i, const int thid) { 
            bool index=0;
            for(int j=0;j<_nIterations;j++){ 
                for(int k = ranges[i].start; k < ranges[i].end; k++){
                    auto res=_rule(matrices[index], k, _n, _m);
                    matrices[!index][k]=res; 
                    images[j](k/_m, k%_m) = _getCimgStateRepr(res);
                }                  
                ba.doBarrier(thid); //Barrier
                #ifndef PARALLEL_WRITE
                if(thid==0){
                    string filename="./frames/"+to_string(j)+".png";
                    char name[filename.size()+1];
                    strcpy(name, filename.c_str());
                    images[j].save(name);
                }
                #endif    
                index=!index; //change the index of the matrix
            }
            #ifdef PARALLEL_WRITE
            int nprint=ceil(double(_nIterations) / double(_parallelism));
            int wstart=thid*nprint;
            int wend= min(int(_nIterations), (int(thid)+1) * nprint);
            for(int k=wstart; k<wend; k++){
                string filename="./frames/"+to_string(k)+".png";
                char name[filename.size()+1];
                strcpy(name, filename.c_str());
                images[k].save(name);
            }
            #endif

        },_parallelism);
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
    return (std::rand())%2;
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
    
    std::srand(0);
    vector<int> matrix (n*m);  
    std::generate(matrix.begin(), matrix.end(), square__init);
    CellularAutomata<int> ca(matrix,n,m, 
        rule,
        state,
        iter,
        nw
    );       
    utimer tp("completion time");
    ca.run();
    return 0;

}
