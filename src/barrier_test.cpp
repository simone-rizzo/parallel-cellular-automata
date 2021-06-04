// ProvaCpiupiu.cpp : Questo file contiene la funzione 'main', in cui inizia e termina l'esecuzione del programma.
//

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
#include <algorithm>
#include <cmath>
using namespace std;
int main() {
    mutex b1Mutex;
    int b1 = 0;
    condition_variable b1Condition;
    vector<int> v(100, 1);

    mutex b2Mutex;
    int b2 = 0;
    condition_variable b2Condition;

    int _parallelism = 12;
    int range=ceil(100.0/double(_parallelism));
    vector<vector<vector<int>>> collectorBuffer(_parallelism, vector<vector<int>>());

    vector<thread> threads(_parallelism);
    for (int i = 0; i < _parallelism; i++) {
        threads[i] = thread([i,&b1, &b1Mutex,&b2,&b2Mutex, &_parallelism, &b1Condition, &b2Condition, range, &v, &collectorBuffer]() {
            vector<int> buffer(range);
            for(int k=0; k<10; k++){
                
                for(int j=i*range; j<min(100, (range*(i+1))); j++){
                    buffer[j%range]=v[(((j-1)%100)+100)%100]+v[j]+v[j%100];
                }
                //srand(i);
                //int wait_time = rand() % 10 + 1;
                //cout << "thread:" << i << " attende: " << wait_time << endl;
                //this_thread::sleep_for(std::chrono::seconds(wait_time));
                collectorBuffer[i].push_back(buffer);
                //fine della computazione
                {
                    unique_lock<mutex> lock1(b1Mutex);
                    //counter increment
                    b1++;
                    b1Condition.wait(lock1, [&]() {
                        return !(b1 != 0 && b1 != _parallelism);
                    });
                    if (b1 == _parallelism) {
                        //cout << "Sono l'ultimo thread: " << i << endl;
                        b1 = 0;
                        b1Condition.notify_all();
                    }
                    //cout << "thread:" << i << " mi hanno svegliato: " << wait_time << endl;    
                }
                for(int j=i*range; j<min(100, (range*(i+1))); j++){
                    v[j]=buffer[j%range];
                }
                //this_thread::sleep_for(std::chrono::seconds(wait_time));
                {
                    unique_lock<mutex> lock2(b2Mutex);
                    //counter increment
                    b2++;
                    b2Condition.wait(lock2, [&]() {
                        return !(b2 != 0 && b2 != _parallelism);
                        });
                    if (b2 == _parallelism) {
                        //cout << "Sono l'ultimo thread: " << i << endl;
                        b2 = 0;
                        b2Condition.notify_all();
                    }
                    //cout << "thread:" << i << " mi hanno svegliato: " << wait_time << endl;
                }
            }
            });
    }
    /*for(int i=0;i<_parallelism;i++)
        threads[i].join();
    for(auto x:v){
        cout<<x;
    }
    cout<<endl;*/
    for(int k=0; k<10; k++){
        for(int i=0; i<_parallelism; i++){
            while(v.size()-1<k){
                
            }
            for(auto x:collectorBuffer[i][k]){
                    cout<<x<<" ";
            }
        }
        cout<<endl;
    }

    for(int i=0;i<_parallelism;i++)
        threads[i].join();
    
    return 0;
}