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

class Barrier2{
    int b1=0;
    mutex *b1Mutex = new mutex();
    condition_variable* b1Condition=new condition_variable();
    int b2=0;
    mutex *b2Mutex = new mutex();
    condition_variable* b2Condition=new condition_variable();
    int max=0;

    
public:
    void wait(){
        {
            unique_lock<mutex> lock(*b1Mutex);
            //counter increment
            b1++;
            (*b1Condition).wait(lock, [&]() {
                return !(b1 != 0 && b1 != max);
                });
            if (b1 == max) {
                //cout << "Sono l'ultimo thread: " << i << endl;
                b1 = 0;
                (*b1Condition).notify_all();
            }
        }
        {
            unique_lock<mutex> lock(*b2Mutex);
            b2++;
            (*b2Condition).wait(lock, [&]() {
                return !(b2 != 0 && b2 != max);
                });
            if (b2 == max) {
                //cout << "Sono l'ultimo thread: " << i << endl;
                b2 = 0;
                (*b2Condition).notify_all();
            }
        }
    }

    
    Barrier2(){}
    Barrier2(int m):max(m){}
        
};