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

//Struct that represent the segment of cells assigned to a worker
struct segment{
    pair<int, int> start;
    pair<int, int> end;
    int size;
};

//Class that implement the Cellular automata modelling paradigm.
class CellularAutomata{
    
    size_t _n; //number of rows
    size_t _m; //number of columns
    vector<vector<vector<int>>> matrices; //the matrices    
    function<int(int, vector<int*>&)> _rule; //rule to be applied at each iteratio
    size_t _parallelism; //parDegree
    size_t _nIterations; //number of iteration
    vector<thread> _workers; //list of workers
    vector<segment> _ranges; //list of segment
    vector<int> _states; //the list of the states possibly assigned at eaach cell
    Barrier2 b; //Barrier object
    vector<vector<vector<int>>> collectorBuffer; //Buffer for collecting the list of segment at each iteration
    vector<vector<int*>> neighbors; //list of neighborhoods to assign for each worker
    vector<CImg<unsigned char>> images; //the list of the images, one for each iteration

    void buffer_init()  {
        //initialization of the buffers
        for(int i=0; i < _parallelism; i++){
            for(int j=0; j<_nIterations; j++){
                collectorBuffer[i][j] = vector<int>(_ranges[i].size); 
            }
        } 
    }

    template <typename Tp> int sgn(Tp val) {
        return (Tp(0) < val) - (val < Tp(0));
    }

    int getRangeSize(pair<int,int>& start, pair<int,int>& end){
        int low = end.second - start.second;
        if (low < 0) --end.first;
        int high = end.first - start.first;
        return (((low % int(_m)) + int(_m) )% int(_m) )+ (high *  int(_m))+1;
    }
    void computeRanges(){
        //We cosider the matrix in base _m like decines and units (sequence of blocks)
        auto size = _n * _m; //matrix size
        int workLoad = ceil(double(size) / double(_parallelism))-1; //Workload size for each thread Upper bounded

        pair<int, int> wl_blocco = make_pair(workLoad / _m, workLoad % _m); //Workload size in the matrix
        pair<int,int> index = make_pair(0,0); // Initial index position
        for (int k = 0; k < _parallelism; k++) { //Loop assign ranges for each thread
            int i = index.first;
            int j = index.second;

            int endj = (((j+ wl_blocco.second) % int(_m))+int(_m))% int(_m); //ending J 
            int carry = (double(j+ wl_blocco.second) / double(_m)); //carry as number of remaining cells
            int endi = (i + carry + wl_blocco.first); //ending i
            if (endi >= _n) { //the last range end int the final position of the matrix 
                endi = _n - 1;
                endj = _m - 1;
            }
            auto end = make_pair(endi, endj);
            _ranges[k] = {index, make_pair(endi, endj), getRangeSize(index, end)}; // write the ranges of theads
            carry=0;
            index.second = (((endj+1)% int(_m))+int(_m))%int(_m); //move to the next cell, for the new range start
            carry = (endj+1) / int(_m);
            index.first =  endi + carry;
            
        }
    }


    //The core of the Framework Spawn the nw thread, that make the computation.
    void spawnWorkers(){
        for(size_t i=0; i<_parallelism; i++){
            _workers[i]=thread([=](){
                segment r=_ranges[i]; //Get is segment from the ranges list
                bool index=0;
                for(size_t j=0; j<_nIterations; j++){ //for each iteration
                    int o=0;
                    for(pair<int, int> curr=r.start; curr <= r.end; increment(curr)){ //for each cell assigned by the segment                        
                        //int currState=matrices[index][curr.first][curr.second]; //get current cell state
                        getNeighbors(curr, index,i); //get the neighborhoods
                        matrices[!index][curr.first][curr.second]=_rule(matrices[index][curr.first][curr.second], neighbors[i]); //write on the other matrix the result value by appling the rule
                        collectorBuffer[i][j][o++]=( matrices[!index][curr.first][curr.second]); //write on buffer the new cell value
                    }
                    index=!index; //change the index of the matrix
                    b.wait(); //synchronize on the barrier
                }
                writeImageParallel(i);                
            });
        }
    }

    //Method for making the writing of image parallel
    void writeImageParallel(int i){
        int nprint=ceil(double(_nIterations) / double(_parallelism)); //compute the number of images per worker
        int start=i*nprint; //starting iteration
        int end= min(int(_nIterations), (int(i)+1) * nprint); //ending iteration
        for(int k=start; k<end; k++){     //for each frames to be created by this worker              
            int c=0;
            for (int j = 0; j < _parallelism; j++) { //for each worker
                for (auto &h:collectorBuffer[j][k]) { 
                    //for each item in the buffer of the kth iteration of thread i
                    images[i](c/_m,c%_m)=_states[h];
                    c++; 
                }
            }    
            string filename="./frames/"+to_string(k)+".png";
            char b[filename.size()+1];
            strcpy(b, filename.c_str());
            //img.resize(1000,1000); //possibility to resize the image
            images[i].save_png(b); //save the image under the folder src/frames
        }
    }

    void increment(pair<int, int>& pair){
        int j=(pair.second + 1) % _m;
        int r=(pair.second + 1) / _m;
        int i=pair.first + r;
        pair.first=i;
        pair.second=j;
    }

    void getNeighbors(pair<int,int>& centre_index, bool index, int h){
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
                    neighbors[h][neigh_num++] = &(matrices[index][qa][za]);

                }
            }
        }
    }

    public:
    CellularAutomata(vector<vector<int>>& initialState ,function<int(int, vector<int*>&)> rule, size_t nIterations, vector<int>states, size_t parallelism){
        _rule=rule;
        _n=initialState.size();
        _m=initialState[0].size();
        matrices=vector<vector<vector<int>>>(2, initialState);
        _states=states;
        _parallelism=parallelism;
        _nIterations=nIterations;
        _workers=vector<thread>(_parallelism);
        _ranges=vector<segment>(_parallelism);
        b=Barrier2(_parallelism);
        collectorBuffer= vector<vector<vector<int>>>(_parallelism, vector<vector<int>>(_nIterations));
        neighbors = vector<vector<int*>>(parallelism,vector<int*>(8));
        images = vector<CImg<unsigned char>>(_nIterations, CImg<unsigned char>(_n,_m));
        computeRanges();
        buffer_init();
    }

    public:
    void run(){
        spawnWorkers();
        for (int i = 0; i < _parallelism; i++)
            _workers[i].join();
    }

};