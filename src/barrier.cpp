#include <stdlib.h>
#include <iostream>
#include <stdio.h>
#include <functional>
#include <vector>
#include <thread>
#include <cmath>
#include <mutex>
#include <condition_variable>


using namespace std;

class Barrier{
    int b=0;
    mutex *bMutex = new mutex();
    condition_variable* bCondition=new condition_variable();
    int max=0;

    
public:
    void wait(){
        unique_lock<mutex> lock(*bMutex);
        //counter increment
        b++;
        (*bCondition).wait(lock, [&]() {
            return !(b != 0 && b != max);
            });
        if (b == max) {
            //cout << "Sono l'ultimo thread: " << i << endl;
            b = 0;
            (*bCondition).notify_all();
        }
    }

    
    Barrier(){}
    Barrier(int m):max(m){}
        
};