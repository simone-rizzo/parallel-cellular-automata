#include <stdio.h>
#include <iostream>
#include <stdlib.h>
#include <cmath>
#include <vector>
using namespace std;

struct range{
    pair<int, int> start;
    pair<int, int> end;
};

int main(){
    vector<vector<int>> prova(8, vector<int>(3, 0));
    
    cout<<prova[0][0]<<endl;
    return 0;

}