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

struct segment{
    pair<int, int> start;
    pair<int, int> end;
    int size;
};

class CellularAutomata{
    
    size_t _n;
    size_t _m;
    vector<vector<vector<int>>> matrices;    
    function<int(int, vector<int*>)> _rule;
    size_t _parallelism;
    size_t _nIterations;
    vector<thread> _workers;
    vector<segment> _ranges;
    vector<int> _states;
    Barrier2 b;
    vector<vector<vector<int>>> collectorBuffer;
    vector<vector<int*>> neighbors;
    vector<CImg<unsigned char>> images;

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

    int getRangeSize(pair<int,int> start, pair<int,int> end){
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
            if (endi >= _n) { //foundamental 
                endi = _n - 1;
                endj = _m - 1;
            }
            _ranges[k] = {index, make_pair(endi, endj), getRangeSize(index, make_pair(endi, endj))}; // write the ranges of theads
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
                segment r=_ranges[i];
                bool index=0;
                for(size_t j=0; j<_nIterations; j++){
                    int o=0;
                    for(pair<int, int> curr=r.start; curr <= r.end; increment(curr)){                        
                        int currState=matrices[index][curr.first][curr.second];
                        vector<int*> neighbors=getNeighbors(curr, index,i);
                        matrices[!index][curr.first][curr.second]=_rule(currState, neighbors);
                        collectorBuffer[i][j][o++]=( matrices[!index][curr.first][curr.second]); 
                    }
                    index=!index;
                    b.wait(); //wait on barrier
                }
                writeImageParallel(i);                
            });
        }
    }


    void writeImageParallel(int i){
        int nprint=ceil(double(_nIterations) / double(_parallelism));
        int start=i*nprint;
        int end= min(int(_nIterations), (int(i)+1) * nprint);
        for(int k=start; k<end; k++){
            //CImg<unsigned char> img(_n,_m); //create new image                    
            int c=0;
            for (int j = 0; j < _parallelism; j++) { //for each worker
                for (int h=0;h<collectorBuffer[j][k].size();h++) { 
                    //for each item in the buffer of the kth iteration of thread i
                    images[i](c/_m,c%_m)=_states[collectorBuffer[j][k][h]];
                    c++; 
                }
            }    
            string filename="./frames/"+to_string(k)+".png";
            char b[filename.size()+1];
            strcpy(b, filename.c_str());
            //img.resize(1000,1000); //possibility to resize the image
            images[i].save_png(b);
        }
    }

    void increment(pair<int, int>& pair){
        int j=(pair.second + 1) % _m;
        int r=(pair.second + 1) / _m;
        int i=pair.first + r;
        pair.first=i;
        pair.second=j;
    }

    vector<int*> getNeighbors(pair<int,int> centre_index, bool index, int h){
        //vector<int*> neighborhood(8);
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
                    neighbors[h][neigh_num++] = &(matrices[index][qa][za]);

                }
            }
        }
        return neighbors[h];
    }

    public:
    CellularAutomata(vector<vector<int>>& initialState ,function<int(int, vector<int*>)> rule, size_t nIterations, vector<int>states, size_t parallelism){
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