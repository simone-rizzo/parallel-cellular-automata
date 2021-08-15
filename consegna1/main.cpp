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

using namespace std;

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
int square__init(){
    return (rand())%2;
}

int main(int argc, char* argv[]){
    if(argc != 5) {
        std::cout << "Usage is: " << argv[0] << " N M number_step number_worker" << std::endl;
        return(-1);
    }
    int n = atoi(argv[1]);
    int m = atoi(argv[2]);
    int iter = atoi(argv[3]);
    int nw = atoi(argv[4]);

    srand(0);
    vector<int> matrix (n*m);  
    std::generate(matrix.begin(), matrix.end(), square__init); 
    
    CellularAutomata<int,vector<int>> ca(
        matrix, n,m, 
        rule, 
        state,
        iter,
        nw
    );       
    utimer tp("completion time");
    ca.run();
    return 0;

}
