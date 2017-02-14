#include <iostream>
#include <fstream>
#include <thread>
#include <cstdlib>
#include <string.h>
#include <sys/types.h>
#include <sys/socket.h>
#include <netinet/in.h>
#include <arpa/inet.h>
#include <stdlib.h>
#include <unistd.h>
#include <ctime>
using namespace std;
int counter=0;
int vector[10][10];
int lambda,alpha;
int counts(){
	string line;
ifstream File("sample.txt");
	while(getline(File,line))
		counter++;
	return counter;
}
int thread_server(int portNum,int own_identity){
    int client, server,k;
    int buffersize=2*sizeof(int);
   	int data[buffersize];
   	struct sockaddr_in server_addr;
    socklen_t size;
    time_t now = time(0);
    char* dt = ctime(&now);

    tm *ltm = localtime(&now);
    dt = asctime(ltm);

    /* ---------- ESTABLISHING SOCKET CONNECTION ----------*/
    /* --------------- socket() function ------------------*/

    server = socket(AF_INET, SOCK_STREAM, 0);

    if (server < 0) 
    {
        //cout << "\nError establishing server socket..." << endl;
        //exit(1);
    }

     //cout << "\n=> Socket server has been created..." << endl;

    

    server_addr.sin_family = AF_INET;
    server_addr.sin_addr.s_addr = INADDR_ANY;
    server_addr.sin_port = htons(portNum);

    if ((bind(server, (struct sockaddr*)&server_addr,sizeof(server_addr))) < 0) 
    {
       // cout << "=> Error binding connection, the socket has already been established..." << endl;
        return -1;
    }

    
    size = sizeof(server_addr);
    //cout << "=> Looking for clients..." << endl;

    
    listen(server, 10);

    int clientCount = 1;

    while(1){
    if ((client = accept(server,(struct sockaddr *)&server_addr,&size)) >= 0) {
    

    	recv(client, data, buffersize, 0);
    //	cout << "received message for server " << portNum << ": data[0] = " << data[0] << "data[1] = " << data[1] << endl;
    	vector[own_identity][own_identity]=vector[own_identity][own_identity]+lambda;
    	if(data[0]>vector[own_identity][data[1]])
    		vector[own_identity][data[1]]=data[0];
        cout<<"Process "<<own_identity<<" received message"<<data[1]<<own_identity<<" from process"<<data[1]<<" at "<<dt<<" its vector clock is [";
            for(k=0;k<counter-1;k++)
            cout<<vector[own_identity][k]<<" ";
            cout<<" ]\n";
    	close(client);
    }
}
        return 0;
}
int thread_client(int own_identity,int neigh1,int neigh2){
	 int client,flag=0,k,l,m,i;
     // NOTE that the port number is same for both client and server
    bool isExit = false;
    int bufsize = 2*sizeof(int);
    int portNum;
    int buffer[bufsize];
    char* ip = "127.0.0.1";
    time_t now = time(0);
    char* dt = ctime(&now);

    tm *ltm = localtime(&now);
    dt = asctime(ltm);
//    cout<<"client is runing %d"<<own_identity;
    for(l=0;l<alpha;l++){// internal event
 		vector[own_identity][own_identity]=vector[own_identity][own_identity]+lambda;
        cout<<"Process "<<own_identity<<" executes internal event e"<<own_identity<<l<< " at "<<dt<<" its vector clock is [";
        for(k=0;k<counter-1;k++)
        cout<<vector[own_identity][k]<<" ";
        cout<<" ]\n"; 
 	}
 	for(m=0;m<1;m++){//send event
 		/*client code is there */
 		vector[own_identity][own_identity]=vector[own_identity][own_identity]+lambda;
 		if(flag%2==0){
 			portNum=4000+neigh1*10;
 			struct sockaddr_in server_addr;
 			client = socket(AF_INET, SOCK_STREAM, 0);
            cout<<"Process "<<own_identity<<" send message"<<own_identity<<neigh1<<" to process"<<neigh1<<" at "<<dt<<" its vector clock is [";
            for(k=0;k<counter-1;k++)
            cout<<vector[own_identity][k]<<" ";
            cout<<" ]\n"; 


 			if (client < 0){
  //      	cout << "\nError establishing client 0 socket..." << endl;
        	continue;
        	//exit(1);
    		}
//    		cout << "\n=> Socket client has been created..." << endl;
    		server_addr.sin_family = AF_INET;
		    server_addr.sin_port = htons(portNum);
		    inet_pton(AF_INET, ip, &server_addr.sin_addr);
		    if (connect(client,(struct sockaddr *)&server_addr, sizeof(server_addr)) == 0) {
  //  	        cout << "=> Connection to the server port number: " << portNum << endl;
    	    	buffer[0]=vector[own_identity][own_identity];
    	    	buffer[1]=own_identity;
    	    	send(client, buffer, bufsize, 0);
    	    	close(client);
		    }
    
 			
 		}
 		if(flag%2==1){
 			portNum=4000+neigh2*10;
 			struct sockaddr_in server_addr;
 			client = socket(AF_INET, SOCK_STREAM, 0);
            cout<<"Process "<<own_identity<<" send message"<<own_identity<<neigh2<<" to process"<<neigh2<<" at "<<dt<<" its vector clock is [";
            for(k=0;k<counter-1;k++)
            cout<<vector[own_identity][k]<<" ";
            cout<<" ]\n"; 
 			if (client < 0){
        	//cout << "\nError establishing client 1 socket..." << endl;
        	//exit(1);
        	continue;
    		}
    		//cout << "\n=> Socket client has been created..." << endl;
    		server_addr.sin_family = AF_INET;
		    server_addr.sin_port = htons(portNum);
		    inet_pton(AF_INET, ip, &server_addr.sin_addr);
			if (connect(client,(struct sockaddr *)&server_addr, sizeof(server_addr)) == 0) {
	        //	cout << "=> Connection to the server port number: " << portNum << endl;
				buffer[0]=vector[own_identity][own_identity];
		    	buffer[1]=own_identity;
		    	for(i=0;i<2;i++)
		    	send(client, buffer, bufsize, 0);
		    	close(client);
        	}    
 		}
 		flag++;
 		}
 		}
 

int thread_function(int i,int neigh1,int neigh2,int portNum){
thread t1(thread_server,portNum,i);
thread t2(thread_client,i,neigh1,neigh2);
t1.join();
t2.join();
return 0;
 }




int main(){
	int k,l; 
	int i=0,j=0,neigh1,neigh2,data,portnumber=4000;
	ifstream File("sample.txt");
	int count=counts();
	count--;
    for(i=0;i<count;i++)
        for(j=0;j<count;j++)
            vector[i][j]=0;
	File>>data;
	File>>data;
	lambda=data;
	File>>data;
	alpha=data;
	thread threads[count];
	cout<<"********************************************************************************** \n";
    cout<<"*********** IMPLEMENTATION OF VECTOR CLOCK WITH SOCKET FOR COMMUNICATION **********\n";
	cout<<"********************************************************************************** \n";
    for(i=0;i<count;i++){
		File>>data;
		File>>data;
		neigh1=data;
		File>>data;
		neigh2=data;
		threads[i]=thread(thread_function,i,neigh1-1,neigh2-1,portnumber);
		
		portnumber=portnumber+10;
	}
	
	for(i=0;i<count;i++)
		threads[i].join();
	
	 
return 0;
}
