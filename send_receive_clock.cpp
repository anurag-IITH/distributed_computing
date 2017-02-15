#include <iostream>
#include <thread>
#include <chrono>
#include <ctime>
#include <fstream>
#include <random>
#include <mutex>
#include <unistd.h>
#include <sys/time.h>
#include <sys/types.h>
#include <sys/socket.h>
#include <netinet/in.h>
#include <arpa/inet.h>
#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#define BUFSIZE 1024
#define PORTNO 10000

using namespace std;
using std::chrono::system_clock;
std::mutex mtx;
int internalEventNumber;
int sendEventNumber;
int receiveEventNumber;
char *servIP = "127.0.0.1";

long long int getTime() 
{
  // get time at that instant
  struct timeval time1;
  gettimeofday(&time1, NULL);
  return time1.tv_sec;
  //*1000000 + time1.tv_usec
}

class Vector{

    int *clock;
    //static thread_local vector<int> clock;

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
    void internalEvent(int processID) 
    {
        clock[processID] = clock[processID] + 1;
    }
    
    void msgSend(int processID)
    {

        clock[processID] = clock[processID] + 1;
    }

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

void testInternalEvents(Vector &Test, int processID, int nofInternalEvents, int lambda)
{
		std::default_random_engine generator;
        std::exponential_distribution<double> distribution (1.0/lambda);
        
        for(int i = 1 ; i <= nofInternalEvents; i++)
    {						
	    internalEventNumber = internalEventNumber + 1;
        mtx.lock();
        ofstream myfile;
        myfile.open ("file.txt", ios::app);
        myfile << "Process" << processID<< " executes";        
        Test.internalEvent(processID); 
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
       
        myfile <<" internal event e" << processID<< internalEventNumber << " at " << hour << ":" << min <<":" << sec << ", vc: [";
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

void testSendEvents(Vector &Test, int processID, int nofSendEvents, int lambda, int topology[], int size, int nodes)
{
        std::default_random_engine generator;
        std::exponential_distribution<double> distribution (1.0/lambda);
        //int servPort = 10001;
        for(int i = 1 ; i <= nofSendEvents; i++)
    {                       
        int RandIndex = rand() % size;
        int servPort = PORTNO + topology[RandIndex];
        sendEventNumber = sendEventNumber + 1;
        mtx.lock();
        ofstream myfile;
        myfile.open ("file.txt", ios::app);
        myfile << "Process" << processID<< " sends message ";        
        Test.msgSend(processID); 
        double sleep_time = distribution (generator);       
        int *a1;
        a1 = Test.getClock();
        time_t currentTime;
        struct tm *local;
        time(&currentTime);

        //Create a socket
        int sockfd = socket(AF_INET, SOCK_STREAM, IPPROTO_TCP);
        if (sockfd < 0) {
            perror("socket() failed");
            exit(-1);
        }

        // Set the server address
        struct sockaddr_in servAddr;
        memset(&servAddr, 0, sizeof(servAddr));
        servAddr.sin_family = AF_INET;
        int err = inet_pton(AF_INET, servIP, &servAddr.sin_addr.s_addr);
        if (err <= 0) {
            perror("inet_pton() failed");
            exit(-1);
        }
        servAddr.sin_port = htons(servPort);
        
        // Connect to server
        if (connect(sockfd, (struct sockaddr *) &servAddr, sizeof(servAddr)) < 0) {
            perror("connect() failed");
            exit(-1);
        }
        
        int *a2 = new int[nodes+1];
        int x;
        for(x = 0; x < nodes; x++){
            a2[x] = a1[x];
        }
        a2[nodes] = -1;
        int length = sizeof(a2);
        printf("%d",length);
        // Send string to server
        int sentLen = send(sockfd, a2, sizeof(a2), 0);
        if (sentLen < 0) {
            perror("send() failed");
            exit(-1);
        } else if (sentLen != length) {
            perror("send(): sent unexpected number of bytes");
            exit(-1);
        }   
        
        close(sockfd);
        
        local = localtime(&currentTime);
        int hour   = local->tm_hour;
        int min    = local->tm_min;
        int sec    = local->tm_sec;
       
        myfile <<"m" << processID<< sendEventNumber << " to process" << processID <<  " at " << hour << ":" << min <<":" << sec << ", vc: [";
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


void testRecvEvents(Vector &Test, int processID)
{                      
    // Run your own socket connection, and go on running infinite amount
    
    int servSock, servPort = 10000 + processID;
    if ((servSock = socket(AF_INET, SOCK_STREAM, IPPROTO_TCP)) < 0) {
        perror("socket() failed");
        exit(-1);
    }
    // Set local parameters
    struct sockaddr_in servAddr;
    memset(&servAddr, 0, sizeof(servAddr));
    servAddr.sin_family = AF_INET;
    servAddr.sin_addr.s_addr = htonl(INADDR_LOOPBACK);
    servAddr.sin_port = htons(servPort); 

    // Bind to the local address
    if (bind(servSock, (struct sockaddr *) &servAddr, sizeof(servAddr)) < 0) {
        perror("bind() failed");
        exit(-1);
    }

    // Listen to the client
    if (listen(servSock, 10) < 0) {
        perror("listen() failed");
        exit(-1);
    }

    // Server Loop
    for (;;) {
        struct sockaddr_in clntAddr;
        socklen_t clntAddrLen = sizeof(clntAddr);

        // Wait for a client to connect
        int clntSock = 
                accept(servSock, (struct sockaddr *) &clntAddr, &clntAddrLen);
        if (clntSock < 0) {
            perror("accept() failed");
            exit(-1);
        }


        // Receive data
        int buffer[BUFSIZE];
        memset(buffer, 0, BUFSIZE);
        int recvLen = recv(clntSock, buffer, BUFSIZE - 1, 0);
        if (recvLen < 0) {
            perror("recv() failed");
            exit(-1);
        }
        int i = 0;
        while(buffer[i]!= -1){
            i = i+1;
        }
        //printf("\n %d length of data is", recvLen);
        
        close(clntSock);
        // printf("End of Server Loop\n");
    


    //int a[3] = {1,2,3};
    receiveEventNumber = receiveEventNumber + 1;
    mtx.lock();
    ofstream myfile;
    myfile.open ("file.txt", ios::app);
    myfile << "Process" << processID<< " receives";    
    Test.msgRecv(buffer);
    int *a1;
    a1 = Test.getClock();
    time_t currentTime;
    struct tm *local;
    time(&currentTime);                   
    local = localtime(&currentTime);
    int hour   = local->tm_hour;
    int min    = local->tm_min;
    int sec    = local->tm_sec;
    myfile <<" m" << processID<< receiveEventNumber << " at " << hour << ":" << min <<":" << sec << ", vc: [";
    for(int i =0; i < 3; i++)
    {
        myfile << a1[i] <<" ";
    }
    myfile << "]"<<endl;
    myfile.close();
    mtx.unlock();
}

}

int test(int processID, int local_port, int lambda, int internalEvent, int sendEvent, int topology[], int size, int nodes)
{
    //int *threadLocalClock = Test.getClock();
    std::thread t1, t2, t3;
    //cout << "I am here";
    Vector vect(nodes);
    t1 = std::thread(testRecvEvents, std::ref(vect), processID);
    t2 = std::thread(testInternalEvents, std::ref(vect), processID, internalEvent, lambda);
    t3 = std::thread(testSendEvents, std::ref(vect), processID, sendEvent, lambda, topology, size, nodes);
    t1.join();
    t2.join();
    t3.join();
    return 0;
}

int main()
{
	//read the input from the file
    
    std::fstream myfile("inp-params.txt", std::ios_base::in);

    int nodes, lambda, internal, pid , size = 1, portno = PORTNO, send = 2;
    int topology[2][1] = {{1},
                           {0}
                           };
    
    myfile >> nodes >> lambda >> internal;

    //Vector *vect = new Vector(n0odes);
    
    // Vector vect(nodes);

    //std::thread t1, t2, t3;
    std::thread t[nodes];

    for(pid = 0; pid < nodes; pid++){
        
        t[pid]= std::thread(test, pid, portno, lambda, internal, send, topology[pid], size, nodes);
        
        portno=portno+1;
    }

    // int topology[1] = {1};
    
    // t1 = std::thread(testInternalEvents, std::ref(vect), pid, alpha, lambda);
    // for (int i = 0 ; i < 100; i++){
    //     if (i >=99)
    // t2 = std::thread(testSendEvents, std::ref(vect), pid, alpha, lambda, topology, length, nodes);
    // }
    // t3 = std::thread(testRecvEvents, std::ref(vect), pid);
    for(pid = 0; pid < nodes; pid++)
        t[pid].join();
    // t1.join();
    // t2.join();
    // t3.join();

    return 0;
}