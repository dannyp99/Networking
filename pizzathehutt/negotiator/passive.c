#include<sys/time.h>
#include<stdio.h>
#include<stdlib.h>
#include<sys/types.h>
#include<sys/socket.h>
#include<netinet/in.h>
#include<pthread.h>
#include<arpa/inet.h>
#include<unistd.h>
#include<netdb.h>
#include<fcntl.h>
#define TCPPORT 29009
#define BUFF 512

struct mesgStruct{
	int mfd;
	int my_seq;
	struct sockaddr_in *myaddr;
};
void readfully(int ufd, void* addr, int size)
{
  int r = 0; // number of bytes read so far
  while (r<size)
    {
      r += read(ufd, (unsigned char*)(addr + r), size-r);
    }
}

void *writemsg(void *param){
	struct mesgStruct peer = *(struct mesgStruct *) param;
	int mfd = peer.mfd;
	int next_seq; 
	int peer_seq;
	int ret;
	int seq = peer.my_seq;
	struct sockaddr_in peeraddr = *(peer.myaddr);
	char thebuff[BUFF];
	while(1<2){
		next_seq = ntohl(seq) + 1;
		ret = sendto(mfd, thebuff, BUFF, 0,(struct sockaddr *) &peeraddr, sizeof(peeraddr));
		peer_seq = *(int *) thebuff;  
		if(peer_seq != next_seq){
			printf("Recieved: %d, Expected: %d!\n", peer_seq, next_seq);
			exit(1);
		}
		next_seq+=2;
		printf("%d: %s", peer_seq, thebuff+4);

	}

}
void passive(char* dfn_ip, char* peer_ip){
	int dfd;
	int ufd;
	int read_ip;
	short udpPort;
	int ret;
	int seq;
	int len;
	int recSeq;
	socklen_t slen,plen;
	struct sockaddr_in dfn; 
	char recv_buf[BUFF];
	char send_buf[BUFF];
	dfn.sin_family = AF_INET;
	dfn.sin_addr.s_addr = inet_addr(dfn_ip);
	dfn.sin_port = htons(TCPPORT);
	struct sockaddr_in saddr;
	struct sockaddr_in peerSock;
	dfd = socket(PF_INET, SOCK_STREAM, 0);
	ufd = socket(PF_INET, SOCK_DGRAM, 0);
	saddr.sin_family = AF_INET;
	saddr.sin_addr.s_addr = htonl(INADDR_ANY);
	saddr.sin_port = 0;
	slen = sizeof(saddr);
	plen = sizeof(peerSock);
	bind(ufd, (struct sockaddr *)&saddr, slen);
	getsockname(ufd, (struct sockaddr *) &saddr, &slen);

	ret = connect(dfd, (struct sockaddr *) &dfd, sizeof(dfd));
	if(ret!=0){
		perror("Connection failed!\n");
		exit(1);
	}
	write(dfd, &(saddr.sin_port), sizeof(short));
	in_addr_t peer = inet_addr(peer_ip);
	readfully(ufd, &read_ip, sizeof(int));
	readfully(ufd,&udpPort,sizeof(short));
	readfully(ufd,&seq, sizeof(int));
	if(udpPort ==0){
		perror("Port is 0!\n");
		exit(1);
	}
	peerSock.sin_family = AF_INET;
	peerSock.sin_addr.s_addr  = peer;
	peerSock.sin_port = udpPort;
	seq = ntohl(seq);
	seq+=1;
	seq = ntohl(seq);
	*(int *) send_buf = seq;
	printf("Sent the enticement: %d\n", ntohl(seq));
	ret = sendto(ufd, send_buf, BUFF,0, (struct sockaddr*) &peerSock, plen);
	ret = recvfrom(ufd, recv_buf, BUFF,0, (struct sockaddr*) &peerSock, &plen);
	int active_seq  = *(int *) recv_buf;
	int next_seq  = ntohl(seq) +1;
	if(ntohl(active_seq) != next_seq && ntohl(active_seq) != -1){
		printf("Sequences are inconsistent\n");
		exit(1);	
	}	
	printf("seq: %d, %s\n", ntohl(active_seq), recv_buf+4);
	ret = recvfrom(ufd, recv_buf, BUFF, 0, (struct sockaddr*)&peerSock,&plen);
	pthread_t tid;
	struct mesgStruct passivestruct;
	passivestruct.my_seq = seq;
	passivestruct.mfd = ufd;
	passivestruct.myaddr = &peerSock;
	pthread_create(&tid, NULL, writemsg, (void *)&peerSock);
	pthread_join(tid, NULL);
	pthread_exit(0);
}

