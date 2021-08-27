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
template <class T, class C>
class CellularAutomata{
        
    typedef struct {
        int start;
        int end;
    } range;

    int _n; //number of rows
    int _m; //number of columns
    vector<vector<T>> matrices; //the two matrix
    function<T(vector<T>&, int const&, int const&, int const&)> _rule;
    function<C(T const&)> _getCimgStateRepr;
    int _nIterations; //number of columns
    int _nworkers; //parDegree
    vector<thread> _workers; //list of workers
    vector<range> ranges; //list of ranges to be assigned at each worker
   
    vector<CImg<C>> images;

    ff::Barrier ba; //the FF barrier
    ff::ParallelFor* pf; //Parallel for reference
    
    void initRanges(){
        int delta { int(_n)*int(_m) / int(_nworkers) };
        for(int i=0; i<_nworkers; i++) { // split the board into peaces
            ranges[i].start = i*delta;
            ranges[i].end   = (i != (_nworkers-1) ? (i+1)*delta : _n*_m); 
        }        
    }   

    public:
    CellularAutomata(vector<T>& initialState, 
                    int n, int m, 
                    function<T(vector<T>&, int const&, int const&, int const&)> rule, 
                    function<C(T const&)> getCimgStateRepr,
                    int nIterations,  int nworkers){   
        _n=n;
        _m=m;
        matrices = vector<vector<T>>(2, initialState);
        _rule=rule;
        _nIterations=nIterations;
        _nworkers = nworkers;
        _getCimgStateRepr=getCimgStateRepr;
        _workers=vector<thread>(_nworkers);
        images = vector<CImg<C>>(nIterations, CImg<C>(_n,_m));
        ranges= vector<range>(_nworkers);
        initRanges();

        ba.barrierSetup(_nworkers);
        
        pf = new ff::ParallelFor(_nworkers);        
        pf->disableScheduler(true); //disabled  
    }

    public:
    void run(){ 
        pf->parallel_for_thid(0,ranges.size(),1,0,[&](const long i, const int thid) { 
            bool index=0;
            for(int j=0;j<_nIterations;j++){ 
                for(int k = ranges[i].start; k < ranges[i].end; k++){
                    auto res=_rule(matrices[index], k, _n, _m);
                    matrices[!index][k]=res; 
                    images[j](k/_m, k%_m) = _getCimgStateRepr(res);
                }                  
                ba.doBarrier(thid); //Barrier
                 
                index=!index; //change the index of the matrix
            }
            int nprint=ceil(double(_nIterations) / double(_nworkers));
            int wstart=thid*nprint;
            int wend= min(int(_nIterations), (int(thid)+1) * nprint);
            for(int k=wstart; k<wend; k++){
                string filename="./frames/"+to_string(k)+".png";
                char name[filename.size()+1];
                strcpy(name, filename.c_str());
                images[k].save(name);
            }
        },_nworkers);
        
    }
};

inline int pmod(int const& v, int const& m){
    return v % m < 0 ? v % m + m : v % m;
}
//Simpler rule Taken from Game of Life by Conways
int rule(const vector<int>& matrix, int const& index, int const& n, int const& m){
    int row = index/m;
    int col = index%m;
    int rm1 = pmod(row-1, m);
    int cm1 = pmod(col-1, n);
    int rp1 = pmod(row+1, m);
    int cp1 = pmod(col+1, m);
    int sum=matrix[rm1*m + col];    //up
    sum += matrix[rm1*m + cm1];     //up_left   
    sum += matrix[rm1*m + cp1];     //up_right       
    sum += matrix[rp1*m + col];     //down
    sum += matrix[row*m + cm1];     //left
    sum += matrix[row*m + cp1];     //right
    sum += matrix[rp1*m + cm1];     //down_left
    sum += matrix[rp1*m + cp1];     //down_right

    int s=matrix[index];

    if(sum==3){
        return 1;
    }
    if(s==1 && (sum==3 || sum==2)){
        return s;
    }
    if((sum == 0 || sum == 1)|| sum >3){
        return 0;
    }
    return 0;
}

unsigned char state(int const& s){
    return s==0 ? 0 : 255;
}
int random_init(){
    return (std::rand())%2;
}

int main(int argc, char* argv[]){
    if(argc != 5) {
        cout << "Usage is: " << argv[0] << " N M number_step number_worker" << endl;
        return(-1);
    }
    int n = atoi(argv[1]);
    int m = atoi(argv[2]);
    int iter = atoi(argv[3]);
    int nw = atoi(argv[4]);
    
    std::srand(0);
    vector<int> matrix (n*m);  
    std::generate(matrix.begin(), matrix.end(), random_init);
    CellularAutomata<int, unsigned char> ca(matrix,n,m, 
        rule,
        state,
        iter,
        nw
    );

    utimer tp("completion time");
    ca.run();
    return 0;
}
