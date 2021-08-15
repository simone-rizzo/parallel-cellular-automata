#include <stdlib.h>
#include <iostream>
#include <stdio.h>
#include <functional>
#include <vector>
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

template <class T>
class CellularAutomata{
        
    
    //ff::ffBarrier ba; //the FF barrier
    int _n; //number of rows
    int _m; //number of columns
    vector<vector<T>> matrices; //the two matrices that will rotate
    function<T(vector<T>&, int const&, int const&, int const&)> _rule;
    function<unsigned char(T const&)> _getCimgStateRepr;
    int _nIterations; //number of columns
    vector<CImg<unsigned char>> images;

    public:
    CellularAutomata(vector<T>& initialState, 
                    int n, int m, 
                    function<T(vector<T>&, int const&, int const&, int const&)> rule, 
                    function<unsigned char(T const&)> getCimgStateRepr,
                    int nIterations){
        _n=n;
        _m=m;
        matrices = vector<vector<T>>(2, initialState);
        _rule=rule;
        _nIterations=nIterations;
        _getCimgStateRepr=getCimgStateRepr;

        images = vector<CImg<unsigned char>>(nIterations,CImg<unsigned char>(_n,_m));

    }

    public:
    void run(){  
        bool index=0;
        //used for the barrier 
        for(int j=0;j<_nIterations;j++){                    
            for(int k = 0; k < _n*_m; k++){
                T res=_rule(matrices[index], k, _n, _m);
                matrices[!index][k]=res; 
                images[j](k/_m, k%_m) = _getCimgStateRepr(res);
            }
            string filename="./frames/"+to_string(j)+".png";
            char name[filename.size()+1];
            strcpy(name, filename.c_str());
            images[j].save(name);
            index=!index;
        }            
    }    
};

int square__init(){
    return (std::rand())%2;
}


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

int main(int argc, char * argv[]) {

    if(argc != 4) {
        std::cout << "Usage is: " << argv[0] << " n m iterations" << std::endl;
        return(-1);
    }

    int n = atoi(argv[1]);
    int m = atoi(argv[2]);
    int iter = atoi(argv[3]);
    
    srand(0);
    vector<int> matrix (n*m);  
    std::generate(matrix.begin(), matrix.end(), square__init); 
    
    CellularAutomata<int> ca(
        matrix, n,m, 
        rule, 
        state,
        iter
    );       
    utimer tp("completion time");
    ca.run();
    return(0);
}