#include <ff/ff.hpp>
#include <ff/parallel_for.hpp>
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

//Implementation of Cellular Automata with FastFlow framework
class CellularAutomata{
    
    size_t _n; //number of rows
    size_t _m; //number of columns
    vector<vector<vector<int>>> matrices; //the matrices      
    function<int(int, vector<int*>&)> _rule; //rule to be applied at each iteration
    size_t _parallelism; //parDegree
    size_t _nIterations; //number of iteration

    vector<segment> _ranges; //list of segment

    ff::ParallelFor* pf; //Parallel for reference
    
    vector<vector<vector<int>>> collectorBuffer; //Buffer for collecting the list of segment at each iteration
    vector<int> _states; //the list of the states possibly assigned at eaach cell
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
            if (endi >= _n) { //the last range end int the final position of the matrix 
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
        bool b=0;
        //First cycle on iteration by launching more time the ff parallel for, by reusing the same
        //threads and make the synchronization automatically at the end of the parfor.
        for(int f=0;f<_nIterations;f++){
            (*pf).parallel_for_static(0,_parallelism,1,0,[=](const long i) {                
                segment r=_ranges[i]; //Get is segment from the ranges list
                int o=0;                   
                    for(pair<int, int> curr=r.start; curr <= r.end; increment(curr)){                        
                            int currState=matrices[b][curr.first][curr.second]; //get current cell state
                            vector<int*> neighbors=getNeighbors(curr, b, i); //get the neighborhoods                         
                            matrices[!b][curr.first][curr.second]=_rule(currState, neighbors); //write on the other matrix the result value by appling the rule                           
                            collectorBuffer[i][f][o++] = matrices[!b][curr.first][curr.second]; //write on buffer the new cell value
                    } 
                                        
            },_parallelism);
            b=!b; //change the index of the matrix
        }
        //After computing the iteration, we launch another parallel for, this time on the image loop.
        (*pf).parallel_for_static(0,_nIterations,1,0,[=](const long i) {          
                    int c=0;
                    for (int j = 0; j < _parallelism; j++) { //for each worker
                        for (int h=0;h< collectorBuffer[j][i].size(); h++) { 
                            //for each item in the buffer of the kth iteration of thread i
                            images[i](c/_m,c%_m)=_states[collectorBuffer[j][i][h]]; //collector buffer shape (nthread, iteration, segment)
                            c++; 
                        }
                    }    
                    string filename="./frames/"+to_string(i)+".png";
                    char b[filename.size()+1];
                    strcpy(b, filename.c_str());
                    images[i].save_png(b);    
        },_parallelism);
        
    }

    void increment(pair<int, int>& pair){
        int j=(pair.second + 1) % _m;
        int r=(pair.second + 1) / _m;
        int i=pair.first + r;
        pair.first=i;
        pair.second=j;
    }

    vector<int*> getNeighbors(pair<int,int> centre_index, int b, int h){
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
                    neighbors[h][neigh_num++] = &(matrices[b][qa][za]);
                }
            }
        }
        return neighbors[h];
    }

    public:
    CellularAutomata(vector<vector<int>>& initialState ,function<int(int, vector<int*>&)> rule, size_t nIterations, vector<int>states, size_t parallelism){
        _rule=rule;
        _n=initialState.size();
        _m=initialState[0].size();
        matrices=vector<vector<vector<int>>>(2, initialState);
        _parallelism=parallelism;
        _nIterations=nIterations;
        _ranges=vector<segment>(_parallelism);
        pf = new ff::ParallelFor(_parallelism);        
        (*pf).disableScheduler(true); //disabled
        collectorBuffer= vector<vector<vector<int>>>(_parallelism, vector<vector<int>>(nIterations));
        neighbors = vector<vector<int*>>(parallelism,vector<int*>(8));
        images = vector<CImg<unsigned char>>(_nIterations, CImg<unsigned char>(_n,_m));
        _states=states;
        computeRanges();
        buffer_init();
    }
    public:
    void run(){
        spawnWorkers();
    }

};



void init_matrix(vector<vector<int>>& matrix, int n, int m)
{    
    srand(0); //fix the seed for testing
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

//Simpler rule Taken from Game of Life by Conways
int rule(int s, vector<int*>& vect){
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
    utimer tp("completion time");
    int n=100;
    int m=100;
    int iter = 10;
    int nw = 8;
    if(argc>1){
        n = atoi(argv[1]);
        m = atoi(argv[2]);
        iter = atoi(argv[3]);
        nw = atoi(argv[4]);
    }
    vector<vector<int>> matrix(n,vector<int>(m));
    init_matrix(matrix,n,m);
    vector<int> states = vector<int>(2);
    states[0]=0;
    states[1]=255;
    function<int(int,vector<int*>&)> f = rule;
    CellularAutomata mcA(matrix, f,
        iter,
        states,
        nw
    );
    mcA.run();
    return 0;
}
