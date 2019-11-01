#include <stdio.h>
#include <unistd.h>
#include<string.h>
#include<stdlib.h>
#include<sys/types.h>
#include<sys/socket.h>
#include<netinet/in.h>
#include<arpa/inet.h>
#include<unistd.h>
#include<netdb.h>
#include<fcntl.h>
#include <pthread.h>
#define BUFSIZE 512
#define DFN_PORT 29009

struct udp_peer {
	int ufd;
	int seq;
	struct sockaddr_in* peeraddr;
};
void readfully(int server_fd, void* addr, int size) 
{
  int r = 0; // number of bytes read so far
  while (r<size)
    {
      r += read(server_fd, (unsigned char*)(addr + r), size-r);
    }
}
void* readmsg(void* ptr)
{
	struct udp_peer peer = *(struct udp_peer*) ptr;
	int ufd = peer.ufd;
	int seq = peer.seq;
	int next_seq;
	int peer_seq;
	int ret;
	struct sockaddr_in peeraddr = *(peer.peeraddr);
	int plen = sizeof(peeraddr);
	char buf[512];
	while(1)
	{
		next_seq = ntohl(seq) + 1;
		ret = recvfrom(ufd, buf, BUFSIZE, 0, (struct sockaddr*) &peeraddr, &plen);
		peer_seq = ntohl(*(int*) buf);
		if (peer_seq != next_seq)
		{
			printf("Got seq: %d but expected %d\n!", peer_seq, next_seq);
			exit(1);

		}
		next_seq += 2;
		printf("%d: %s", peer_seq, buf+ 4);

	}	

}
void *writemsg(void *param){
	struct udp_peer peer = *(struct udp_peer *) param;
	int mfd = peer.ufd;
	int next_seq; 
	int peer_seq;
	int ret;
	int seq = peer.seq;
	struct sockaddr_in peeraddr = *(peer.peeraddr);
	char thebuff[BUFSIZE];
	while(1<2){
		next_seq = ntohl(seq) + 1;
		ret = sendto(mfd, thebuff, BUFSIZE, 0,(struct sockaddr *) &peeraddr, sizeof(peeraddr));
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
	char recv_buf[BUFSIZE];
	char send_buf[BUFSIZE];
	dfn.sin_family = AF_INET;
	dfn.sin_addr.s_addr = inet_addr(dfn_ip);
	dfn.sin_port = htons(DFN_PORT);
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
	ret = sendto(ufd, send_buf, BUFSIZE,0, (struct sockaddr*) &peerSock, plen);
	ret = recvfrom(ufd, recv_buf, BUFSIZE,0, (struct sockaddr*) &peerSock, &plen);
	int active_seq  = *(int *) recv_buf;
	int next_seq  = ntohl(seq) +1;
	if(ntohl(active_seq) != next_seq && ntohl(active_seq) != -1){
		printf("Sequences are inconsistent\n");
		exit(1);	
	}	
	printf("seq: %d, %s\n", ntohl(active_seq), recv_buf+4);
	ret = recvfrom(ufd, recv_buf, BUFSIZE, 0, (struct sockaddr*)&peerSock,&plen);
	pthread_t tid;
	pthread_t tid2;
	struct udp_peer passivestruct;
	passivestruct.seq = seq;
	passivestruct.ufd = ufd;
	passivestruct.peeraddr = &peerSock;
	pthread_create(&tid, NULL, writemsg, (void*) &peerSock);
	pthread_create(&tid2, NULL, readmsg, (void*) &peerSock);
	pthread_join(tid, NULL);
	pthread_join(tid2,NULL);
	pthread_exit(0);
}

pthread_mutex_t mutex;
int active(char* dfn_ip, char* peer_ip)
{
	int dfd;
	int ufd;
	int ret;
	short peer_port;
	int seq;
	struct sockaddr_in dfn;
	char recv_buf[BUFSIZE];
	char send_buf[BUFSIZE];
	dfn.sin_family = AF_INET;
	dfn.sin_addr.s_addr = inet_addr(dfn_ip);
	dfn.sin_port = htons(DFN_PORT);
	struct sockaddr_in peeraddr;
	socklen_t plen, mlen;
	struct sockaddr_in myaddr;
	dfd = socket(AF_INET,SOCK_STREAM, 0);
	ufd = socket(AF_INET,SOCK_DGRAM,0);
	myaddr.sin_family = AF_INET;
	myaddr.sin_addr.s_addr = htonl(INADDR_ANY); // accept from any source ip
	myaddr.sin_port = 0;
	mlen = sizeof(myaddr);
	plen = sizeof(peeraddr);
	bind(ufd,(struct sockaddr *) &myaddr, mlen);  
	getsockname(ufd, (struct sockaddr*) &myaddr, &mlen);

	ret = connect(dfd, (struct sockaddr*) &dfn, sizeof(dfn));
	if (ret !=0 )
	{
		perror("Can't connect to DFN:");
		exit(1);
	}
	write(dfd, &(myaddr.sin_port), 2);
	in_addr_t peer = inet_addr(peer_ip); 
	write(dfd, &peer, 4); 
	readfully(dfd, &peer_port, 2);
	readfully(dfd,&seq, 4);	
	if (peer_port == 0)
	{
		printf("got error from DFN");
		exit(1);
	}
	peeraddr.sin_family = AF_INET;
	peeraddr.sin_addr.s_addr = peer;
	peeraddr.sin_port = peer_port; //already in network byte order
	sleep(2);
  	seq = ntohl(seq);
	seq += 1;
	seq = htonl(seq);
	*((int *)send_buf) = seq;	
	printf("Sent enticment with seq %d\n", ntohl(seq));
	ret = sendto(ufd, send_buf, BUFSIZE,0, (struct sockaddr*) &peeraddr, plen);
	ret = recvfrom(ufd, recv_buf, BUFSIZE, 0, (struct sockaddr*) &peeraddr,&plen);
	int client_seq = *((int*) recv_buf);
	int expected_seq = ntohl(seq) + 1;
	if(ntohl(client_seq) != expected_seq && ntohl(client_seq) != -1)
	{
		printf("Got message with seq: %d but expected seq: %d!\n", client_seq, expected_seq);
		exit(1);
	} 
	printf("seq=%d %s\n", ntohl(client_seq), recv_buf+4); seq = ntohl(seq);
	seq += 2;
	seq = htonl(seq);
	*((int*) send_buf) = seq;
	sprintf(send_buf+4,"We need to talk....");
	ret = sendto(ufd, send_buf, BUFSIZE, 0, (struct sockaddr*) &peeraddr, sizeof(peeraddr));
	pthread_t tid;
	pthread_t tid2;
	struct udp_peer peerstruct;
	peerstruct.seq = seq;
	peerstruct.ufd = ufd;
	peerstruct.peeraddr = &peeraddr;
	pthread_create(&tid, NULL, readmsg, (void*) &peerstruct);
	pthread_create(&tid2, NULL, writemsg, (void*) &peerstruct);
	pthread_join(tid, NULL);
	pthread_join(tid2, NULL);
	return 0;

}


int main(int argc, char* argv[])
{
	if(argc == 4 && strcmp(argv[3], "active") == 0)
	{
		printf("active");
		active(argv[1], argv[2]);
	}
	else if (argc==3)
	{
		printf("passive");
		passive(argv[1], argv[2]);
	}
}
