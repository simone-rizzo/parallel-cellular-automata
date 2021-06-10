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
#include "utimer.cpp"

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


int main(){
    utimer tp("completion time");
    int n=100;
    int m=100;
    vector<vector<int>> matrix(n,vector<int>(m));
    init_matrix(matrix,n,m);
    function<int(int,vector<int*>)> f = rule;
    CellularAutomata<int> mcA(n, m, f,
       matrix,
        400,
        8
    );
    return 0;

}
