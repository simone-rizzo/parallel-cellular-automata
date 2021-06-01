#include <stdio.h>
#include <iostream>
#include <stdlib.h>
#include <tuple>
#include <vector>
using namespace std;

vector<int> get_neighborhood(int matrix[4][4], int i, int j)
{
    vector<int> neighborhood(8);
    int neigh_num = 0;
    //int i = get<0>(ij);
    //int j = get<1>(ij);

    int n = 4;
    int init_i = (((i - 1) % n)+n)%n;
    int init_j = (((j - 1) % n)+n)%n;

    for (int q = init_i; q < init_i + 3; q++) {
        for (int z = init_j; z < init_j + 3; z++) {
            int qa = (((q % n) + n) % n);
            int za = (((z % n) + n) % n);
            if (qa != i || za != j) {
                matrix[qa][za] = 1; //only for test
                neighborhood[neigh_num] = (matrix[qa][za]);
            }
        }
    }
    return neighborhood;
}

void printMatrix(int mat[4][4], int n, int m) {
	for (int i = 0; i < n; i++) {
		for (int j = 0; j < m; j++)
			cout << mat[i][j] << " ";
		cout << endl;
	}
}

int main() {

    int a[4][4] = {
    0,0,0,0,
    0,0,0,0,
    0,0,0,0,
    0,0,0,0,
    };
    tuple<int, int> index = make_tuple(0, 0);
    vector<int> list = get_neighborhood(a,2,2);
    printMatrix(a,4,4);
    return 0;
}