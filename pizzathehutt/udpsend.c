/* UDP sender
   usage:
   ./udpsend sport dip dport

   will continuously send udp packets to dip:dport from localhost, on source
   port.  if sport is 0, then the source port will be picked by the system.

   Beware that just because we've sent a udp packet doesn't mean the other
   side will receive it (unlike tcp).
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

/* 
   Concept of client-server initialization doesn't exist with udp -
   There is still a server (with well-known as oppposed to ephemeral port),
   but no handshake is used to establish who the server is, and who the
   client is.  In other words, anybody can call themselves servers - whether
   the "clients" will respect that depends not on the protocol but on how
   the clients will behave.

   recvfrom is a more generalized form of accept
   sendto is a more generalized form of connect

   In other words, each transaction is like a separate connection.

   In UDP, unlike TCP, "client-server" is a more general concepts - it's
   dictated by the nature of the application rather than the protocal.

   There is no acknowlegment service, defragmentation service, or 
   congestion control.

   There is a "buffer" attached to the file descriptor, but this has
   local meaning only.  
*/

#define BUFSIZE 128

int main(int argc, char** args)
  {
  unsigned short sport, dport;
  int sockfd, r, x, y,result, plen, mlen;
  int limit;  
  unsigned char buffer[BUFSIZE];
  struct sockaddr_in peeraddr;  // used by sendto
  struct sockaddr_in myaddr;    // used by recvfrom
  limit = 1000;

  if (argc<4) { printf("need args sport dip dport\n"); exit(1); }
  sport = atoi(args[1]);
  dport = atoi(args[3]);
  if (argc>4) limit = atoi(args[4]);

  sockfd = socket(AF_INET,SOCK_DGRAM,0);

  peeraddr.sin_family = AF_INET;
  peeraddr.sin_addr.s_addr = inet_addr(args[2]);
  peeraddr.sin_port = htons(dport);
  myaddr.sin_family = AF_INET;
  myaddr.sin_addr.s_addr = htonl(INADDR_ANY); // accept from any source ip
  myaddr.sin_port = htons(sport);
  plen = sizeof(peeraddr);  mlen = sizeof(myaddr);
  if (sport>1023)
     bind(sockfd,(struct sockaddr *)&myaddr,mlen);  // bind optional

  getsockname(sockfd,(struct sockaddr *)&myaddr,&mlen);
  unsigned short port = ntohs(myaddr.sin_port);
  printf("Socke bound to port %d\n",port);

  // assign some data (ascii-range values) to buffer:
  // First 4 bytes of buffer will indicate a sequence number:
  for(r=4;r<BUFSIZE;r++) buffer[r] = (unsigned char) ((r % 96)+32);

  for(r=0;r<limit;r++)  
    {
      // put r into start of buffer:
      *((int*)buffer) = r;
      result = sendto(sockfd,buffer,BUFSIZE,0,(struct sockaddr*)&peeraddr,plen);
       printf("sent packet %d to client\n",r);

      // delay loop
      for(x=0;x<10000000;x++) { x++; x--; } 
    }

  /* how do I know if other side closed connection?  YOU DON'T - THERE 
     IS NO "CONNECTION"! 
  */

  close(sockfd); // only have local meaning - no packets exchanged.
  exit(0);
}

  
