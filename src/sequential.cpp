#include <iostream>
#include <algorithm>
#include <vector>
#include <stdlib.h>
#include <unistd.h>
#include "./cimg/CImg.h"
#include "utimer.cpp"
#define cimg_use_png

using namespace std;
using namespace cimg_library;

int square__init(){
    return (std::rand())%2;
}


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

void update_cell(int j,int _n,int _m, std::vector<int> &board, std::vector<int> &previus_board){
    int row = j/_m;
    int col = j%_m;
    int sum = (row == 0) ? 0: previus_board[(row - 1)*_m + col];                            //up
    sum += (row == 0 || col == 0) ? 0: previus_board[(row - 1)*_m + col - 1];               //up_left   
    sum += (row == 0 || col == _m - 1) ? 0: previus_board[(row - 1)*_m + col + 1];          //up_right       
    sum += (row == _n - 1) ? 0: previus_board[(row + 1)*_m + col];                          //down
    sum += (col == 0) ? 0: previus_board[(row)*_m + col - 1];                               //left
    sum += (col == _m - 1) ? 0: previus_board[(row)*_m + col + 1];                          //right
    sum += ((row == _n - 1) || col == 0) ? 0: previus_board[(row + 1)*_m + col - 1];        //down_left
    sum += ((row == _n - 1) || col == _m - 1) ? 0: previus_board[(row + 1)*_m + col + 1];   //down_right
    board[row*_m + col] = rule(previus_board[row*_m + col], sum); // apply the rule and update the new matrix
    }


int main(int argc, char * argv[]) {

    if(argc != 4) {
        std::cout << "Usage is: " << argv[0] << " N M number_step seed" << std::endl;
        return(-1);
    }

    int N = atoi(argv[1]);
    int M = atoi(argv[2]);
    int num_steps = atoi(argv[3]);
    srand(0);

    int index = 0; 
    std::vector<std::vector<int>>  boards(2, std::vector<int> (N*M) );  
    std::generate(boards[index].begin(), boards[index].end(), square__init); 
     
    utimer ut("completion time");
    for (int i=0; i< num_steps; i++) {
        for(int j = 0; j<N*M; j++){
            update_cell(j, N, M, boards[!index], boards[index]);
        }        
        index = !index;//previus_board = board;
        /*CImg<unsigned char> img(N,M); //create new image
        for(int p=0; p<N*M; p++){
            img(p/M,p%M)=boards[index][p]>0?255:0;
        }
        string filename="./frames/"+to_string(i)+".png";
        char b[filename.size()+1];
        strcpy(b, filename.c_str());
        img.save_png(b);*/
    }
    return(0);
}