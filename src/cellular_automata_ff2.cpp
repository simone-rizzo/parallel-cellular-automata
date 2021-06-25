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
#include <mutex>
#include <condition_variable>
#include <algorithm>
#include <string>
#include <cstring>
#define cimg_use_png

#include "./cimg/CImg.h"
#include <cstdint>
#include <cassert>
#include "barrier_2.cpp"
#include "utimer.cpp"

using namespace std;
using namespace cimg_library;

//#define PARALLEL_WRITE //writes the images in parallel
//#define LAST_WRITE //write the result image


//Class that implement the Cellular automata modelling paradigm.
class CellularAutomata{
        
    typedef struct {
        int start;
        int end;
    } RANGE;

    int _n; //number of rows
    int _m; //number of columns
    vector<vector<int>> matrices; //the two matrix
    function<int(int,int)> _rule; //the rule
    int _nIter; //number of columns
    int _parallelism; //parDegree
    vector<int> _states; //the list of the states possibly assigned at eaach cell
    ff::ffBarrier ba; //the FF barrier
    vector<RANGE> ranges;

    ff::ParallelFor* pf; //Parallel for reference
    #ifdef PARALLEL_WRITE
    vector<CImg<unsigned char>> images;
    #endif


    void update_cell(int j, std::vector<int> &board, std::vector<int> &previus_board){
        int row = j/_m;
        int col = j%_m;
        int sum = (row == 0) ? 0: previus_board[(row - 1)*_m + col];                            //up
        sum += (row == 0 || col == 0) ? 0: previus_board[(row - 1)*_m + col - 1];               //up_left   
        sum += (row == 0 || col == _m - 1) ? 0: previus_board[(row - 1)*_m + col + 1];          //up_right       
        sum += (row == _n - 1) ? 0: previus_board[(row + 1)*_m + col];                          //down
        sum += (col == 0) ? 0: previus_board[(row)*_m + col - 1];                               //left
        sum += (col == _m - 1) ? 0: previus_board[(row)*_m + col + 1];                          //right
        sum += ((row == _n - 1) || col == 0) ? 0: previus_board[(row + 1)*_m + col - 1];        //down_left
        sum += ((row == _n - 1) || col == _m - 1) ? 0: previus_board[(row + 1)*_m + col + 1];   //down_right
        board[row*_m + col] = _rule(previus_board[row*_m + col], sum); // apply the rule and update the new matrix
       }

    void compute_ranges(){
        int delta { int(_n)*int(_m) / int(_parallelism) };
        for(int i=0; i<_parallelism; i++) { // split the board into peaces
            ranges[i].start = i*delta;
            ranges[i].end   = (i != (_parallelism-1) ? (i+1)*delta : _n*_m); 
        }        
        int a = 10;
    }   

    public:
    CellularAutomata(vector<int>& initialState, int n, int m, function<int(int, int)> rule, int nIterations, vector<int> states, int parallelism){
        _n=n;
        _m=m;
        matrices = vector<vector<int>>(2, initialState);
        _rule=rule;
        _nIter=nIterations;
        _parallelism = parallelism;
        _states = states;
        ranges= vector<RANGE>(parallelism);
        ba.barrierSetup(_parallelism);
        pf = new ff::ParallelFor(_parallelism);        
        (*pf).disableScheduler(true); //disabled        
        ranges= vector<RANGE>(parallelism);        
        compute_ranges(); 
        #ifdef PARALLEL_WRITE
        images = vector<CImg<unsigned char>>(nIterations,CImg<unsigned char>(_n,_m));
        #endif       
    }

    public:
    void run(){
       
                   
            (*pf).parallel_for_thid(0,ranges.size(),1,0,[&](const long i, const int thid) { 
                bool b=0;
                for(int j=0;j<_nIter;j++){ 
                    for(int k = ranges[i].start; k < ranges[i].end; k++){
                        update_cell(k, matrices[!b], matrices[b]);
                        #ifdef PARALLEL_WRITE
                        images[j](k/_m, k%_m) = _states[matrices[!b][k]];
                        #endif
                    }                      
                    ba.doBarrier(thid); //Barrier
                    b=!b; //change the index of the matrix
                }
                #ifdef PARALLEL_WRITE
                int nprint=ceil(double(_nIter) / double(_parallelism));
                int _start=thid*nprint;
                int _end= min(int(_nIter), (int(thid)+1) * nprint);
                for(int k=_start; k<_end; k++){
                    string filename="./frames/"+to_string(k)+".png";
                    char bb[filename.size()+1];
                    strcpy(bb, filename.c_str());
                    images[k].save(bb);
                }
                #endif

            },_parallelism);

            #ifdef LAST_WRITE        
            CImg<unsigned char> img(_n,_m); //create new image
            for(int i=0; i<_n*_m; i++){
                    img(i/_m,i%_m)=_states[matrices[(_nIter%2)][i]];
            }
            string filename="./frames/final_frame.png";
            char bb[filename.size()+1];
            strcpy(bb, filename.c_str());
            img.save_png(bb);        
            #endif
    }
};

int rule(int s, int sum){
    if(sum==3)
    {
        return 1;
    }
    else if(s==1 && (sum==3 || sum==2))
    {
        return s;
    }
    else if((sum == 0 || sum == 1)|| sum >3)
    {
        return 0;
    }
    return 0;
}

int square__init(){
    return (std::rand())%2;
}

int main(int argc, char* argv[]){
    if(argc != 5) {
        std::cout << "Usage is: " << argv[0] << " N M number_step number_worker" << std::endl;
        return(-1);
    }
    int n=100;
    int m=100;
    int iter = 10;
    int nw = 1;
    if(argc>1){
        n = atoi(argv[1]);
        m = atoi(argv[2]);
        iter = atoi(argv[3]);
        nw = atoi(argv[4]);
    }
    std::srand(0);
    vector<int> matrix (n*m);  
    std::generate(matrix.begin(), matrix.end(), square__init); 
    vector<int> states = vector<int>(2);
    states[0]=0;
    states[1]=255;
    function<int(int,int)> f = rule;
    CellularAutomata mcA(matrix,n,m, f,
        iter,
        states,
        nw
    );       
    utimer tp("completion time");
    mcA.run();
    return 0;

}
