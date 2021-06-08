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

int rule(int s, vector<int*> vect){
    for(int i=0; i<vect.size();i++)
    {
        int num = *vect[i];
        s+=num;
    }
    return s;
}

int main(){
    vector<vector<int>> m(4,vector<int>(5,1));
    function<int(int,vector<int*>)> f = rule;
    CellularAutomata<int> mcA(4, 5, f,
       m,
        6,
        5
    );

    return 0;

}
