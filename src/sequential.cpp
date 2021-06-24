#include "utimer.cpp"
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


using namespace std;
using namespace cimg_library;

    size_t _n;
    size_t _m;
    size_t _parallelism;
    size_t _nIterations;
    
    vector<vector<vector<int>>> matrices;
    vector<int*> neighborhood(8);   

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


void init_matrix(vector<vector<int>>& matrix, int n, int m)
{
    srand(0);
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

void getNeighbors(int i,int j, bool index){
    int neigh_num = 0;

    int n = _n;
    int m = _m;

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
    img.save_png(b);
    
}


int main(int argc, char *argv[]){
    utimer ct("completion time");
    _n=100;
    _m=100;
    _nIterations = 100;
    if(argc>1){
        _n = atoi(argv[1]);
        _m = atoi(argv[2]);
        _nIterations = atoi(argv[3]);
    }    
    vector<vector<int>> tmp_matrix(_n,vector<int>(_m));
    init_matrix(tmp_matrix,_n,_m);
    matrices = vector<vector<vector<int>>>(2,tmp_matrix);
    bool index = 0;
    for(int i=0;i<_nIterations;i++){
        for(int n=0;n<_n;n++){
            for(int m=0;m<_m;m++){
                getNeighbors(n,m,index);
                int new_state = rule(matrices[index][n][m], neighborhood);
                matrices[!index][n][m] = new_state;
            }
        }
        writeImage(index,i);
        index=!index;
    }

    return 0;
}