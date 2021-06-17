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
#include <string>
#include <cstring>
#include "jpeglib.h"
#define cimg_use_jpeg
#include "./cimg/CImg.h"
#include <cstdint>
#include <cassert>
#include "barrier_2.cpp"
//#define PARALLEL_WRITE

using namespace std;
using namespace cimg_library;

struct segment{
    pair<int, int> start;
    pair<int, int> end;
};

class CellularAutomata{
    
    size_t _n;
    size_t _m;

    
    vector<vector<vector<int>>> matrices;
    
    function<int(int, vector<int*>)> _rule;
    size_t _parallelism;
    size_t _nIterations;
    //bool emitterEnabled;
    
    //thread _emitter;
    vector<thread> _workers;
    vector<segment> _ranges;
    Barrier2 b;
    vector<vector<vector<int>>> collectorBuffer;

    void computeRanges(){
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

    void spawnWorkers(){
        for(size_t i=0; i<_parallelism; i++){
            _workers[i]=thread([=](){
                segment r=_ranges[i];
                bool index=0;
                for(size_t j=0; j<_nIterations; j++){                    
                    vector<int> buffer; // buffer for saving the changes in the matrix segment in each iteration      
                    for(pair<int, int> curr=r.start; curr <= r.end; increment(curr)){
                        //cout<<"thread: "<<i<<" start: "<<r.start.first<<","<<r.start.second<<" end: "<<r.end.first<<","<<r.end.second<<"curr: "<<curr.first<<","<<curr.second<<endl;
                        int currState=matrices[index][curr.first][curr.second];
                        vector<int*> neighbors=getNeighbors(curr, index);
                        int new_state = _rule(currState, neighbors);             
                        buffer.push_back(new_state); // save the state in the buffer                        
                        matrices[!index][curr.first][curr.second]=new_state;
                    }
                    index=!index;
                    
                    //write buffer to the collector queue      
                    collectorBuffer[i].push_back(buffer);
                    b.wait(); // wait on barrier                    

                }
                //After the cellular automat task start to create the image and save on disk.
                writeImageParallel(i);
            });
        }
    }



    void writeImage(int index, int iteration){
        CImg<unsigned char> img(_n,_m); //create new image                    
        int c=0;
        auto matrix=matrices[index];
        for(int i=0; i<_n; i++){
            for(int j=0; j<_m; j++){
                img(i,j)=matrix[i][j]>0?255:0;
            }
        }
        string filename="./frames/"+to_string(iteration)+".jpeg";
        char b[filename.size()+1];
        strcpy(b, filename.c_str());
        //img.resize(1000,1000);
        //img.save_png(b);
        
    }

    void writeImageParallel(int i){
        int nprint=ceil(double(_nIterations) / double(_parallelism));
        int start=i*nprint;
        int end= min(int(_nIterations), (int(i)+1) * nprint);
        //string s="thread: "+to_string(i)+" start: "+to_string(start)+" end: " +to_string(end);
        //cout<<s<<endl;
        for(int k=start; k<end; k++){
            CImg<unsigned char> img(_n,_m); //create new image                    
            int c=0;
            for (int j = 0; j < _parallelism; j++) { //for each worker
                for (auto x : collectorBuffer[j][k]) { 
                    //for each item in the buffer of the kth iteration of thread i
                    img(c/_m,c%_m)=x>0?255:0;
                    c++; 
                }
            }    
            string filename="./frames/"+to_string(k)+".png";
            char b[filename.size()+1];
            strcpy(b, filename.c_str());
            //img.resize(1000,1000);
            img.save_png(b);
        }
    }

    void writeBufferInMatrix(vector<int>& buffer, segment r, bool index){
        size_t k=0;
        for(pair<int, int> curr = r.start; curr<=r.end; increment(curr)){
            matrices[index][curr.first][curr.second] = buffer[k];
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

    vector<int*> getNeighbors(pair<int,int> centre_index, bool index){
        vector<int*> neighborhood(8);
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
                    neighborhood[neigh_num++] = &(matrices[index][qa][za]);

                }
            }
        }
        return neighborhood;
    }

    public:
    CellularAutomata(vector<vector<int>>& initialState, function<int(int, vector<int*>)> rule,  size_t nIterations, size_t parallelism){
        _rule=rule;
        _n=initialState.size();
        _m=initialState[0].size();
        matrices=vector<vector<vector<int>>>(2, initialState);
        //matrices[0]=initialState;
        //matrices[1]=new vector<vector<T>>(_n, vector<T>(_m));

        //TODO: if less than 2 error
        _parallelism=parallelism;
        _nIterations=nIterations;
        _workers=vector<thread>(_parallelism);
        _ranges=vector<segment>(_parallelism);
        b=Barrier2(_parallelism);
        collectorBuffer= vector<vector<vector<int>>>(_parallelism, vector<vector<int>>());        
        computeRanges();
    }

    public:
    void run(){
        spawnWorkers(); 
        for (int i = 0; i < _parallelism; i++)
            _workers[i].join();
    }

};