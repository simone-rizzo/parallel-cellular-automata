#include <stdio.h>
#include <iostream>
#include <stdlib.h>
#include <tuple>
#include <vector>
using namespace std;

vector<int> get_neighborhood(vector<vector<int>>& matrix, pair<int,int> centre_index)
{
    vector<int> neighborhood(8);
    int neigh_num = 0;
    int n = matrix.size();
    int m = matrix[0].size();
    int i = centre_index.first;
    int j = centre_index.second;

    int init_i = (((i - 1) % n)+n)%n;
    int init_j = (((j - 1) % m)+m)%m;

    for (int q = init_i; q < init_i + 3; q++) {
        for (int z = init_j; z < init_j + 3; z++) {
            int qa = (((q % n) + n) % n);
            int za = (((z % m) + m) % m);
            if (qa != i || za != j) {
                matrix[qa][za] = 1; //only for test
                neighborhood[neigh_num] = (matrix[qa][za]);
            }
        }
    }
    return neighborhood;
}

void printMatrix(vector<vector<int>> mat) {
    
    int n = mat.size();
    int m = mat[0].size();

	for (int i = 0; i < n; i++) {
		for (int j = 0; j < m; j++)
			cout << mat[i][j] << " ";
		cout << endl;
	}
}

int main() {

    vector<vector<int>> matrix(4,vector<int>(3,0));
    pair<int, int> index = make_pair(0, 0);
    vector<int> list = get_neighborhood(matrix,index);
    printMatrix(matrix);
    return 0;
}