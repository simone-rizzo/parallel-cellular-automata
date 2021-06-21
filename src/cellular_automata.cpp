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
#define cimg_use_png

#include "./cimg/CImg.h"
#include <cstdint>
#include <cassert>
#include "barrier_2.cpp"

#include "utimer.cpp"
//#define PARALLEL_WRITE

using namespace std;
using namespace cimg_library;

struct range{
    pair<int, int> start;
    pair<int, int> end;
    int size;
};

template<class T>
class CellularAutomata{
    
    size_t _n;
    size_t _m;

    
    vector<vector<vector<T>>> matrices;
    
    function<T(T, vector<T*>)> _rule;
    size_t _parallelism;
    size_t _nIterations;
    //bool emitterEnabled;
    
    //thread _emitter;
    vector<thread> _workers;
    vector<range> _ranges;

    #ifndef PARALLEL_WRITE
    Barrier b1;
    Barrier b2;
    #endif
    #ifdef PARALLEL_WRITE
    Barrier2 b;
    vector<vector<vector<T>>> collectorBuffer;
    #endif
    


    void init ()  {
        //initEmitter();
        initRanges();
        
        #ifdef PARALLEL_WRITE
        //initialization of the buffers
        for(int i=0; i < _parallelism; i++){
            for(int j=0; j<_nIterations; j++){
                collectorBuffer[i][j] = vector<T>(_ranges[i].size); 
            }
        }
        #endif
        initEmitterCollector();  
    }

    void initEmitterCollector(){
        //TODO add emitter collector thread
        initWorkers();

        for (int i = 0; i < _parallelism; i++)
            _workers[i].join();
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
    void initRanges(){
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



    void initWorkers(){
        for(size_t i=0; i<_parallelism; i++){
            _workers[i]=thread([=](){
                range r=_ranges[i];
                bool index=0;
                for(size_t j=0; j<_nIterations; j++){
                    string s="matrix it:"+to_string(j);
                    //utimer somma("somma");
                    {   //utimer tp(s);
                        int o=0;
                        for(pair<int, int> curr=r.start; curr <= r.end; increment(curr)){
                            //cout<<"thread: "<<i<<" start: "<<r.start.first<<","<<r.start.second<<" end: "<<r.end.first<<","<<r.end.second<<"curr: "<<curr.first<<","<<curr.second<<endl;
                            T currState=matrices[index][curr.first][curr.second];
                            vector<T*> neighbors=getNeighbors(curr, index);
                            matrices[!index][curr.first][curr.second]=_rule(currState, neighbors);
                            #ifdef PARALLEL_WRITE
                            collectorBuffer[i][j][o++]=( matrices[!index][curr.first][curr.second]);                            
                            //buffer.push_back(_rule(currState, neighbors));
                            #endif
                            
                        }
                        index=!index;
                    }
                        #ifdef PARALLEL_WRITE
                        //write buffer to the collector queue      
                        //collectorBuffer[i].push_back(buffer);
                        b.wait();
                        #endif

                        #ifndef PARALLEL_WRITE
                        b1.wait();
                        if(i==0) //hardcoded the first worker writes the image
                        {
                            //utimer tp1("image:");
                            writeImage(index, j);
                        }
                        b2.wait();
                        #endif
                        
                    

                }
                
                #ifdef PARALLEL_WRITE
                //utimer tp("image par:");
                writeImageParallel(i);
                #endif
                
            });
        }
    }



    void writeImage(int index, int iteration){
        CImg<unsigned char> img(_n,_m); //create new image                    
        auto matrix=matrices[index];
        for(int i=0; i<_n; i++){
            for(int j=0; j<_m; j++){
                img(i,j)=matrix[i][j]>0?255:0;
            }
        }
        string filename="./frames/"+to_string(iteration)+".png";
        char b[filename.size()+1];
        strcpy(b, filename.c_str());
        //img.resize(1000,1000);
        img.save_png(b);
        
    }

#ifdef PARALLEL_WRITE
    void writeImageParallel(int i){

        int nprint=ceil(double(_nIterations) / double(_parallelism));
        int start=i*nprint;
        int end= min(int(_nIterations), (int(i)+1) * nprint);
        //string s="thread: "+to_string(i)+" start: "+to_string(start)+" end: " +to_string(end);
        //cout<<s<<endl;
        for(int k=start; k<end; k++){
            //utimer gg("single image parallel");
            CImg<unsigned char> img(_n,_m); //create new image                    
            int c=0;
            for (int j = 0; j < _parallelism; j++) { //for each worker
                for (int h=0;h<collectorBuffer[j][k].size();h++) { 
                    //for each item in the buffer of the kth iteration of thread i
                    img(c/_m,c%_m)=collectorBuffer[j][k][h]>0?255:0;
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
#endif
    void writeBufferInMatrix(vector<T>& buffer, range r, bool index){
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

    vector<T*> getNeighbors(pair<int,int> centre_index, bool index){
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
                    neighborhood[neigh_num++] = &(matrices[index][qa][za]);

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
        matrices=vector<vector<vector<T>>>(2, initialState);
        //matrices[0]=initialState;
        //matrices[1]=new vector<vector<T>>(_n, vector<T>(_m));

        //TODO: if less than 2 error
        _parallelism=parallelism;
        _nIterations=nIterations;
        _workers=vector<thread>(_parallelism);
        _ranges=vector<range>(_parallelism);
        #ifndef PARALLEL_WRITE
        b1=Barrier(_parallelism);
        b2=Barrier(_parallelism);
        #endif
        #ifdef PARALLEL_WRITE
        b=Barrier2(_parallelism);
        collectorBuffer= vector<vector<vector<T>>>(_parallelism, vector<vector<T>>(_nIterations));
        #endif
       init();
    }

};