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
#include <ff/barrier.hpp>
using namespace std;
using namespace cimg_library;


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
    Barrier2 b; //Barrier object
    vector<thread> _workers; //list of workers
    vector<RANGE> ranges;
    ff::ffBarrier ba; //the FF barrier

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
        _workers=vector<thread>(_parallelism);        
        b=Barrier2(_parallelism);
        compute_ranges();
        
        ba.barrierSetup(_parallelism);
    }

    public:
    void run(){       
        for(int i=0;i<_parallelism;i++){    
            _workers[i]=thread([=](int ini, int end){                    
                bool index=0;     
                for(int j=0;j<_nIter;j++){                    
                    for(int k = ini; k < end; k++){
                        update_cell(k, matrices[!index], matrices[index]);                        
                    }                                     
                    ba.doBarrier(i);                  
                    index=!index;
                }
            }, ranges[i].start, ranges[i].end);
        }
        
        for (int i = 0; i < _parallelism; i++){
                _workers[i].join();
        }
   
    }
                  
                    

};