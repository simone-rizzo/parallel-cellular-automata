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
#include "./cellular_automata2.cpp"

using namespace std;

//Simpler rule Taken from Game of Life by Conways
int rule(int s, int sum){
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

int square__init(){
    return (rand())%2;
}

int main(int argc, char* argv[]){
    if(argc != 5) {
        std::cout << "Usage is: " << argv[0] << " N M number_step number_worker" << std::endl;
        return(-1);
    }
    int n=400;
    int m=400;
    int iter = 10;
    int nw = 1;

    if(argc>1){
        n = atoi(argv[1]);
        m = atoi(argv[2]);
        iter = atoi(argv[3]);
        nw = atoi(argv[4]);
    }
    
    srand(0);
    vector<int> matrix (n*m);  
    std::generate(matrix.begin(), matrix.end(), square__init); 
    vector<int> states = vector<int>(2);
    states[0]=0;
    states[1]=255;
    function<int(int,int)> f = rule;
    CellularAutomata mcA(matrix,n,m, f,
        iter,
        states,
        nw
    );       
    utimer tp("completion time");
    mcA.run();
    return 0;

}
