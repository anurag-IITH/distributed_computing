#include <iostream>
#include <thread>
#include <chrono>
#include <ctime>
#include <fstream>
#include <random>
#include <mutex>
#include <sys/time.h>

using namespace std;
using std::chrono::system_clock;
std::mutex mtx;
int internalEventNumber;
int sendEventNumber;
int receiveEventNumber;

long long int getTime() 
{
  // get time at that instant
  struct timeval time1;
  gettimeofday(&time1, NULL);
  return time1.tv_sec;
  //*1000000 + time1.tv_usec
}

class Vector{

    int* clock;
    int n;
    public:
        Vector(int n)
        {
            this->n = n;
            clock = new int[n];
            for (int i = 0; i < n; i++) {
                clock[i] = 0;
            }
        }
    //~Vector();
    void internalEvent(int threadid) 
    {
        clock[threadid] = clock[threadid] + 1;
    }
    
    // void msgSend(int threadid)
    // {

    // }

    void msgRecv(int *clock_recv)
    {
        for(int i = 0; i < n ; i++)
        {
            if(clock[i] < clock_recv[i])
            {
                clock[i] = clock_recv[i];
            }
        }
    }

    int* getClock(){
        return clock;
    }

};

void testInternalEvents(Vector &Test, int threadid, int nofInternalEvents, int lambda)
{
		std::default_random_engine generator;
        std::exponential_distribution<double> distribution (1.0/lambda);
        
        for(int i = 1 ; i <= nofInternalEvents; i++)
    {						
	    internalEventNumber = internalEventNumber + 1;
        mtx.lock();
        ofstream myfile;
        myfile.open ("file.txt", ios::app);
        myfile << "Process" << threadid+1 << " executes";        
        Test.internalEvent(threadid); 
        double sleep_time = distribution (generator);       
        int *a1;
        a1 = Test.getClock();
        time_t currentTime;
        struct tm *local;
        time(&currentTime);                   
        local = localtime(&currentTime);
        int hour   = local->tm_hour;
        int min    = local->tm_min;
        int sec    = local->tm_sec;
       
        myfile <<" internal event e" << threadid+1 << internalEventNumber << " at " << hour << ":" << min <<":" << sec << ", vc: [";
        for(int i =0; i < 3; i++)
        {
            myfile << a1[i] <<" ";
        }
        myfile << "]"<<endl;
        myfile.close();
        mtx.unlock();
        std::this_thread::sleep_for(std::chrono::duration<double> (sleep_time));
	}

}

void testRecvEvents(Vector &Test, int threadid)
{                      
    int a[3] = {1,2,3};
    receiveEventNumber = receiveEventNumber + 1;
    mtx.lock();
    ofstream myfile;
    myfile.open ("file.txt", ios::app);
    myfile << "Process" << threadid+1 << " receives";    
    Test.msgRecv(a);
    int *a1;
    a1 = Test.getClock();
    time_t currentTime;
    struct tm *local;
    time(&currentTime);                   
    local = localtime(&currentTime);
    int hour   = local->tm_hour;
    int min    = local->tm_min;
    int sec    = local->tm_sec;
    myfile <<" m" << threadid+1 << receiveEventNumber << " at " << hour << ":" << min <<":" << sec << ", vc: [";
    for(int i =0; i < 3; i++)
    {
        myfile << a1[i] <<" ";
    }
    myfile << "]"<<endl;
    myfile.close();
    mtx.unlock();
}

int main()
{
	//read the input from the file

    std::fstream myfile("inp-params.txt", std::ios_base::in);

    int nodes, lambda, alpha, i = 0;
    
    myfile >> nodes >> lambda >> alpha;

    //Vector *vect = new Vector(nodes);
    
    Vector vect(nodes);

    std::thread t1, t2, t3;
    
    t1 = std::thread(testInternalEvents, std::ref(vect), i, alpha, lambda);
    
    t3 = std::thread(testRecvEvents, std::ref(vect), i);
    
    int *a;
    a = vect.getClock();
    
    t1.join();
    // t2.join();
    t3.join();

    return 0;
}