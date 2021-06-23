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

/*
    Implementation of the Barrier, for making the synchronization of the worker at the end of each 
    iteration.
*/

class Barrier2{
    int b1=0; //first counter
    mutex *b1Mutex = new mutex();
    condition_variable* b1Condition=new condition_variable();
    int b2=0; //second counter
    mutex *b2Mutex = new mutex();
    condition_variable* b2Condition=new condition_variable();
    int max=0; //number of maximal worker

    
public:
    void wait(){
        {
            unique_lock<mutex> lock(*b1Mutex); //take the first lock
            //counter increment
            b1++;
            (*b1Condition).wait(lock, [&]() { //wait coidition
                return !(b1 != 0 && b1 != max);
                });
            if (b1 == max) { //the last thread update the counter and notify all the threads
                //cout << "I'm the last thread: " << i << endl;
                b1 = 0;
                (*b1Condition).notify_all();
            }
        }
        //The same procedure is repeted here. This garantee that the all thread have finished theire work.
        {
            unique_lock<mutex> lock(*b2Mutex);
            b2++;
            (*b2Condition).wait(lock, [&]() {
                return !(b2 != 0 && b2 != max);
                });
            if (b2 == max) {
                //cout << "I'm the last thread: " << i << endl;
                b2 = 0;
                (*b2Condition).notify_all();
            }
        }
    }

    
    Barrier2(){}
    Barrier2(int m):max(m){}
        
};