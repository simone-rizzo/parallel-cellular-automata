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


using namespace std;

struct range{
    pair<int, int> start;
    pair<int, int> end;
};

int main(){
    mutex b1Mutex;
    int b1 = 0;
    condition_variable b1Condition;
    int _parallelism = 5;
    vector<thread> threads(_parallelism);
    for (int i = 0; i < 5; i++) {
        threads[i] = thread([i,&b1, &b1Mutex, &_parallelism, &b1Condition]() {
            int wait_time = rand() % 10 + 1;
            cout << "thread:" << i << " attende: " << wait_time << endl;
            this_thread::sleep_for(std::chrono::seconds(wait_time));
            //fine della computazione
            unique_lock<mutex> lock1(b1Mutex);
            //counter increment
            b1++;
            b1Condition.wait(lock1, [&]() {
                return !(b1 > 0 && b1 < _parallelism);
            });
            cout << "thread:" << i << " mi hanno svegliato: " << wait_time << endl;
            if (b1 == _parallelism) {
                cout << "Sono l'ultimo thread: " << i << endl;
                b1 = 0;
                b1Condition.notify_all();
            }
            lock1.release();
            });
    }
    return 0;

}