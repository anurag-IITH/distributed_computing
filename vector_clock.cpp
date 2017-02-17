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
#include <vector>
#define BUFSIZE 1024
#define PORTNO 10000

using namespace std;
using std::chrono::system_clock;
int topologyglobal[120];
int topoglobalcounter = 0;
std::mutex mtx;
char *servIP = "127.0.0.1";

int getarray(char line[], int size){
    
    int k , count = 0, arraysize = 0;
    std::vector<int> v;
    
    for(int i = 0; line[i] != '\0'; i++){
        if(line[i] != ' '){
        k = line[i] - 48;
        v.push_back (k);
        count++;            
        }    
    }
    
    arraysize = count -1;
        
    for (int j = 1; j < count; j++){
        topologyglobal[topoglobalcounter++] = v[j];                
    }

    return arraysize;   
}

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
    // int internalEventNumber;
    // int sendEventNumber;
    // int receiveEventNumber;
    public:
        Vector(int n)
        {
            this->n = n;
            internalEventNumber = 0;
            sendEventNumber = 0;
            receiveEventNumber = 0;
            clock = new int[n];
            for (int i = 0; i < n; i++) {
                clock[i] = 0;
            }
        }
    //~Vector(); 
    
    int internalEventNumber;
    int sendEventNumber;
    int receiveEventNumber;   
        
    void internalEvent(int processID) 
    {
        clock[processID] = clock[processID] + 1;
        internalEventNumber = internalEventNumber + 1;
    }
    
    void msgSend(int processID)
    {

        clock[processID] = clock[processID] + 1;
        sendEventNumber = sendEventNumber + 1;
    }

    void msgRecv(int *clock_recv, int processID)
    {
        clock[processID] = clock[processID] + 1;
        for(int i = 0; i < n ; i++)
        {
            if(clock[i] < clock_recv[i])
            {
                clock[i] = clock_recv[i];
            }
        }
        receiveEventNumber = receiveEventNumber + 1;
    }

    int* getClock(){
        return clock;
    }

};

void testInternalEvents(Vector &Test, int processID, int nofInternalEvents, int lambda, int nodes)
{
		std::default_random_engine generator;
        std::exponential_distribution<double> distribution (1.0/lambda);
        
        for(int i = 1 ; i <= nofInternalEvents; i++)
    {						
	    //internalEventNumber = internalEventNumber + 1;
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
       
        myfile <<" internal event e" << processID<< Test.internalEventNumber << " at " << hour << ":" << min <<":" << sec << ", vc: [";
        for(int i =0; i < nodes; i++)
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
        
        for(int i = 1 ; i <= nofSendEvents; i++){

        int RandIndex = rand() % size;
        int servPort = PORTNO + topology[RandIndex];
        cout << servPort << " " << topology[RandIndex] << " " << size;
        int senderID = topology[RandIndex];
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
        unsigned int microseconds = 6000000;
        while(connect(sockfd, (struct sockaddr *) &servAddr, sizeof(servAddr)) < 0) {
            perror("connect() failed");
            usleep(microseconds);
        }
        
        int *a2 = new int[nodes+1];
        int x;
        for(x = 0; x < nodes; x++){
            a2[x] = a1[x];
        }
        a2[nodes] = -1;
        int length = sizeof(a2);
        //printf("%d",length);
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
       
        myfile <<"m" << processID<< Test.sendEventNumber << " to process" << senderID <<  " at " << hour << ":" << min <<":" << sec << ", vc: [";
        for(int i =0; i < nodes; i++)
        {
            myfile << a1[i] <<" ";
        }
        myfile << "]"<<endl;
        myfile.close();
        mtx.unlock();
        std::this_thread::sleep_for(std::chrono::duration<double> (sleep_time));
    }

}

void testRecvEvents(Vector &Test, int processID, int nodes)
{                      
    // Run your own socket connection, and go on running infinite amount
    
    int servSock, servPort = PORTNO + processID;
    if ((servSock = socket(AF_INET, SOCK_STREAM, IPPROTO_TCP)) < 0) {
        perror("socket() failed");
        exit(-1);
    }
    // Set local parameters
    struct sockaddr_in servAddr;
    memset(&servAddr, 0, sizeof(servAddr));
    servAddr.sin_family = AF_INET;
    servAddr.sin_addr.s_addr = htonl(INADDR_ANY);
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

        // char clntIpAddr[INET_ADDRSTRLEN];
        // if (inet_ntop(AF_INET, &clntAddr.sin_addr.s_addr, 
        //         clntIpAddr, sizeof(clntIpAddr)) != NULL) {
        //     printf("----\nHandling client %s %d\n", 
        //             clntIpAddr, ntohs(clntAddr.sin_port));}
        int receiverID = ntohs(clntAddr.sin_port);
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
        //int receiverID = ntohs(clntAddr.sin_port);
        close(clntSock);
        // printf("End of Server Loop\n");

    mtx.lock();
    ofstream myfile;
    myfile.open ("file.txt", ios::app);
    myfile << "Process" << processID<< " receives";    
    Test.msgRecv(buffer, processID);
    int *a1;
    a1 = Test.getClock();
    time_t currentTime;
    struct tm *local;
    time(&currentTime);                   
    local = localtime(&currentTime);
    int hour   = local->tm_hour;
    int min    = local->tm_min;
    int sec    = local->tm_sec;
    myfile <<" m" << receiverID<< Test.receiveEventNumber << " from" << receiverID << " at " << hour << ":" << min <<":" << sec << ", vc: [";
    for(int i =0; i < nodes; i++)
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
    t1 = std::thread(testRecvEvents, std::ref(vect), processID, nodes);
    t2 = std::thread(testInternalEvents, std::ref(vect), processID, internalEvent, lambda, nodes);
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

    // Read the first line separately
    int SIZE = 80, line1[4], k = 0;
    char line[SIZE];
    myfile.getline(line, SIZE);    

    for(int i = 0; line[i] != '\0'; i++){
        if(line[i] != ' ' && line[i] != '/'){
            line1[k++] = line[i] - 48;             
        }
    }

    int nodes = line1[0], lambda = line1[1], internal = line1[2], send = line1[3];
    int pid , size[nodes], portno = PORTNO , j = 0;
    
    for(int i = 0; i < nodes; i++)
    {
        if(myfile.eof() != 0)
            break;  

    myfile.getline(line, SIZE);    
    size[j++] = getarray(line, SIZE);

    }
           
    myfile.close();

    std::thread t[nodes];
    int sz = 0;
    for(pid = 0; pid < nodes; pid++){
        int *arr = new int[size[pid]];
        for(int i = 0; i < size[pid] ; i++)
            arr[i] = topologyglobal[i + sz];
        sz += size[pid];
        t[pid]= std::thread(test, pid, portno, lambda, internal, send, arr, size[pid], nodes);
        
        portno=portno+1;
    }
    
    for(pid = 0; pid < nodes; pid++)
        t[pid].join();
   
    return 0;
}