#include <sys/types.h>   
#include <sys/socket.h>
#include <stdio.h>
#include <string.h>
#include <stdlib.h>
#include <netinet/in.h>
#include <fcntl.h>
#include <resolv.h>
#include <pthread.h>

#include "queue.h"

#define PIC_PENALTY 2
#define VIDEO_PENALTY 3
#define MUSIC_PENALTY 2

#define MAX_SIZE 3
#define RECV_SIZE 5

#define ip1 "192.168.0.101"
#define ip2 "192.168.0.102"
#define ip3 "192.168.0.103"



#define __OPEN_SERVERS do{ \
			if(open_servers(serv,ip1)){ \
				printf("failed to open server sockets\n "); \
				return -1; \
			} \
			if(open_servers(&serv[1],ip2)){ \
				printf("failed to open server sockets\n "); \
				return -1; \
			} \
			if(open_servers(&serv[2],ip3)){ \
				printf("failed to open server sockets\n "); \
				return -1; \
			}}while(0)
			
typedef struct handler_type{
	int sfd; //socket fd
	JobQueue queue;
	int load;
} handler_t;

handler_t serv[3];

void* handler(void* param){
	handler_t* h = (handler_t*) param;
	char buf[MAX_SIZE];
	while(1){
		Job job;
		if((job = dequeue(h->queue)){
			send(h->sfd,job,2,0);
			int ret = recv(h->sfd,buf,MAX_SIZE,0); //recv is a blocking call;
			send(job->clientfd,buf,ret,0);
			close(job->cientfd);
			destroy_job(job);
		}
		
	}
	return NULL;
}

int getMin(int L1,int L2,int L3){
	int tmp = L1 < L2 ? L1 : L2;
	return tmp < L3 ? tmp : L3;
}

int choose_best_server(char* buf){
	short length = buf[1]-'0';
	int best=-1;
	switch(buf[0]){
		case 'M':
			best =  getMin(serv[0].load+length*MUSIC_PENALTY,serv[1].load+length*MUSIC_PENALTY,serv[2].load+length);
			serv[best].load+= best<2?length*MUSIC_PENALTY:length;
		case 'V':
			best =  getMin(serv[0].load+length,serv[1].load+length,serv[2].load+length*VIDEO_PENALTY);
			serv[best].load+= best==2?length*VIDEO_PENALTY:length;
		case 'P':
			best =  getMin(serv[0].load+length,serv[1].load+length,serv[2].load+length*PIC_PENALTY);
			serv[best].load+= best==2?length*PIC_PENALTY:length;
	}
	return best;
}
void listener(){
	int sfd;
	struct sockaddr_in serv_addr,out_addr;
	sfd = socket(AF_INET,SOCK_STREAM,0);
	if(sfd < 0){
		printf("Error opening socket\n");
		return;
	}
	 serv_addr.sin_family = AF_INET;
     serv_addr.sin_addr.s_addr = INADDR_ANY;
     serv_addr.sin_port = htons(80);
	 if(bind(sfd, (struct sockaddr *) &serv_addr,sizeof(serv_addr)) < 0){
		printf("Error binding socket to address\n");
		return;
	 }
	 if(listen(sfd,100)<0){
		printf("Failed to listen\n");
		return;
	 }
	 char buf[RECV_SIZE];
	 while(1){
		socklen_t socksize = sizeof(struct sockaddr);
		int clientSock = accept(sfd,(struct sockaddr*)&out_addr,&socksize);
		if(recv(clientSock,buf,2,0)<0){
			printf("Listener error on recv\n");
			return;
		}
		int serverID = choose_best_server(buf);
		Job job = job_create(clientSock,buf[0],buf[1]);
		enqueue(serv[serverID].queue,job);
		//decide which server to send to;
	 }
	 return;
}
int open_servers(handler_t* h,char* ip){
	struct sockaddr_in serv_addr;
	serv_addr.sin_family = AF_INET;
    serv_addr.sin_port = htons(80);
	h->sfd = socket(AF_INET,SOCK_STREAM,0);
	if(h->sfd < 0){
		printf("failed to create server socket");
		return -1;
	}
	if(inet_aton(ip,&serv_addr.sin_addr.s_addr)<0){
		perror(ip);
		return -1;
	}
	if(connect(h->sfd,(struct sockaddr*)&serv_addr,sizeof(serv_addr))<0){
		printf("failed to connect to server socket %d\n",h->sfd);
		return -1;
	}
	return 0;
}
int start_threads(){
	pthread_t server1, server2, server3;
	if(pthread_create(&server1,NULL,handler,(void*)serv)<0){
		printf("failed to create server handler thread1\n");
		return -1;
	}
	if(pthread_create(&server2,NULL,handler,(void*)(serv+1))<0){
		printf("failed to create server handler thread2\n");
		return -1;
	}
	if(pthread_create(&server3,NULL,handler,(void*)(serv+2))<0){
		printf("failed to create server handler thread3\n");
		return -1;
	}
}
int main(){	
	for(int i=0;i<3;++i){
		serv[i].queue = create_jobqueue();
		serv[i].load = 0;
	}
	__OPEN_SERVERS;
	if(start_threads()<0){
		for(int i=0;i<3;++i){
			destroy_jobqueue(serv[i].queue);
		}
		return -1;
	}
	listener();
}
