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

class my_barrier
{

 public:
    my_barrier(int count)
     : thread_count(count)
     , counter(0)
     , waiting(0)
    {}

    void wait()
    {
        //fence mechanism
        std::unique_lock<std::mutex> lk(*m);
        ++counter;
        ++waiting;
        cv.wait(lk, [&]{return counter >= thread_count;});
        cv.notify_one();
        --waiting;
        if(waiting == 0)
        {
           //reset barrier
           counter = 0;
        }
        lk.unlock();
    }

 private:
      std::mutex *m;
      std::condition_variable cv;
      int counter;
      int waiting;
      int thread_count;
};