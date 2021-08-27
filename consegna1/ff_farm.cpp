#include <ff/ff.hpp>
#include <ff/parallel_for.hpp>
#include <ff/barrier.hpp>
#include <ff/farm.hpp>
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

//Memory deallocationcan be forcedby the assign() function.


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
    function<T(const vector<T>&, int const&, int const&, int const&)> _rule;
    function<C(T const&)> _getCimgStateRepr;
    int _nIterations; //number of columns
    int _nworkers; //parDegree
    vector<thread> _workers; //list of workers
    vector<range> ranges; //list of ranges to be assigned at each worker
   
    vector<CImg<C>> images;

    ff::Barrier ba; 
    ff::ff_Farm<int>* farm;
    std::vector<std::unique_ptr<ff::ff_node> > W;
    

    struct firstThirdStage: ff::ff_node_t<int> {
        vector<CImg<C>>& images;
        int _nIterations;

        firstThirdStage(const size_t length, vector<CImg<C>>& images,
                        int _nIterations)
            :length(length), images(images), _nIterations(_nIterations) {}

        int* svc(int *task) {
            if (task == nullptr) {
                for(size_t i=0; i<length; ++i) {
                    ff_send_out(new int(i));
                }
                return GO_ON;
            }        
            int &t = *task;
            string filename="./frames/"+to_string(ntasks)+".png";
            char name[filename.size()+1];
            strcpy(name, filename.c_str());
            images[ntasks].save(name);
            //delete task;
            if (++ntasks == _nIterations) return EOS;
            return GO_ON;         
        }

        void svc_end() {  }

        const int length;
        int ntasks=0;
    };

    struct secondStage: ff::ff_node_t<int> {
        int _n; //number of rows
        int _m; //number of columns
        vector<vector<T>> &matrices; //the two matrix
        function<T(const vector<T>&, int const&, int const&, int const&)> _rule;
        function<C(T const&)> _getCimgStateRepr;
        int _nIterations; //number of columns
        vector<range> &ranges; //list of ranges to be assigned at each worker
        vector<CImg<C>> &images;
        ff::ffBarrier &ba; 
        secondStage(int nIterations, vector<range>&ranges, int _n, int _m, 
                    vector<vector<T>> &matrices,
                    function<T(const vector<T>&, int const&, int const&, int const&)> _rule,
                    function<C(T const&)> _getCimgStateRepr,
                    vector<CImg<C>>& images, ff::ffBarrier& ba ):
            _n(_n), _m(_m),
            _nIterations(nIterations), ranges(ranges),
            matrices(matrices), _rule(_rule),
            _getCimgStateRepr(_getCimgStateRepr), images(images), ba(ba) {}

        int* svc(int * task) { 
            int &t = *task; 
            bool index=0;
            
            for(int j=0;j<_nIterations;++j){ 
                //utimer tp("compute time");
                for(int k = ranges[t].start; k < ranges[t].end; ++k){
                    auto res=_rule(matrices[index], k, _n, _m);
                    matrices[!index][k]=res;
                    images[j](k/_m, k%_m) = _getCimgStateRepr(res);
                    /*images[j](k/_m, k%_m, 0, 0) = 15;//repr(res);//_getCimgStateRepr(res);
                    images[j](k/_m, k%_m, 0, 1) = 200;
                    images[j](k/_m, k%_m, 0, 2) = 50;*/
                }                  
                ba.doBarrier(t); //Barrier
                if(t==0) {
                    ff_send_out(task);
                }
                index=!index; //change the index of the matrix
            }
            return EOS; 
        }
    };
    
    firstThirdStage*  firstThird;

    void initRanges(){
        int delta { int(_n)*int(_m) / int(_nworkers) };
        for(int i=0; i<_nworkers; ++i) { // split the board into peaces
            ranges[i].start = i*delta;
            ranges[i].end   = (i != (_nworkers-1) ? (i+1)*delta : _n*_m); 
        }        
    }   

    public:
    CellularAutomata(vector<T>& initialState, 
                    int n, int m, 
                    function<T(const vector<T>&, int const&, int const&, int const&)> rule, 
                    function<C(T const&)> getCimgStateRepr,
                    int nIterations,  int nworkers){   
        _n=n;
        _m=m;
        matrices = vector<vector<T>>(2, initialState);
        _rule=rule;
        _nIterations=nIterations;
        _nworkers = nworkers;
        _getCimgStateRepr=getCimgStateRepr;

        images = vector<CImg<C>>(nIterations, CImg<C>(_n,_m));
        ranges= vector<range>(_nworkers);

        initRanges();

        ba.barrierSetup(_nworkers);
        
        firstThird=new firstThirdStage(_nworkers, images, _nIterations);
               
        for(int i=0;i<nworkers;++i) W.push_back(make_unique<secondStage>(_nIterations,
                 ranges,  _n,  _m, matrices,
                    _rule, _getCimgStateRepr, images, ba));

        farm = new ff::ff_Farm<int>(std::move(W), *firstThird);
        (*farm).remove_collector(); // needed because the collector is present by default in the ff_Farm
        (*farm).wrap_around();   // this call creates feedbacks from Workers to the Emitter
    }

    public:
    void run(){ 
        if ((*farm).run_and_wait_end()<0) {
            return;
        }
    }
};

inline int pmod(int v, int m){
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
