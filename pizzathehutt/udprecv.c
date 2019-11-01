/* UDP receiver

Usage:

./udprecv dport

continuously wait for packets to arrive on dport, prints source ip and source
port of the packet.
*/

 
#include<stdio.h>
#include<stdlib.h>
#include<sys/types.h>
#include<sys/socket.h>
#include<netinet/in.h>
#include<arpa/inet.h>
#include<unistd.h>
#include<netdb.h>
#include<fcntl.h>

#define SETLOCALINFO bind
#define SETPEERINFO connect

#define BUFSIZE 128

int main(int argc, char** args)  
{

  unsigned int dport, sport;
  int sockfd, i, x, y,result, mlen;
  socklen_t plen;
  unsigned char buffer[BUFSIZE];
  struct sockaddr_in peeraddr;  // used by sendto
  struct sockaddr_in myaddr;    // used by recvfrom
  if (argc!=2) { printf("need arg destination port\n"); exit(1); }
  dport = atoi(args[1]);
  sockfd = socket(AF_INET,SOCK_DGRAM,0);
  myaddr.sin_family = AF_INET;
  myaddr.sin_addr.s_addr = htonl(INADDR_ANY); // accept from any source ip
  myaddr.sin_port = htons(dport);
  plen = sizeof(peeraddr);  mlen = sizeof(myaddr);
  bind(sockfd,(struct sockaddr *)&myaddr,mlen);  // bind needed.

  for(i=0;i<10000;i++)
    {
      result = recvfrom(sockfd,buffer,BUFSIZE,0,(struct sockaddr*)&peeraddr,&plen);
      // extract and print sequence number inside first 4 bytes of buffer:
      x = *((int*)buffer);
      printf("packet %d received from source ip %s, source port %d\n",x,inet_ntoa(peeraddr.sin_addr), ntohs(peeraddr.sin_port));

      for(x=0;x<10000000;x++) { x++; x--; } 
      
    } // for i


  close(sockfd); // only have local meaning - no packets exchanged.
  exit(0);
}

  
