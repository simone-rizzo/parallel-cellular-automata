#include <stdio.h>
#include <iostream>
#include <stdlib.h>
#include <tuple>
#include <vector>
#include <atomic>
#include <thread>
#include <mutex>
#include <condition_variable>
#include <chrono>
#include <functional>
#include "./cellular_automata.cpp"

//#include "../fastflow/ff/ff.hpp"

using namespace std;

int rule(int s, vector<int*> vect){
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


int main(int argc, char* argv[]){
    utimer tp("completion time");
    int n=400;
    int m=400;
    int iter = 400;
    int nw = 8;
    if(argc>1){
        n = atoi(argv[1]);
        m = atoi(argv[2]);
        iter = atoi(argv[3]);
        nw = atoi(argv[4]);
    }
    vector<vector<int>> matrix(n,vector<int>(m));
    init_matrix(matrix,n,m);
    function<int(int,vector<int*>)> f = rule;
    CellularAutomata<int> mcA(n, m, f,
       matrix,
        iter,
        nw
    );
    return 0;

}
