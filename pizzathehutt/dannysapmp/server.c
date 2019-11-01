/* tcp server skeleton */

#include<stdio.h>
#include<stdlib.h>
#include<sys/types.h>
#include<sys/socket.h>
#include<netinet/in.h>
#include<arpa/inet.h>
#include<unistd.h>
#include<netdb.h>
#include<fcntl.h>

/* argv[1] will be port that server "binds" to */

int main(int argc, char **argv)
{
  int sfd;  // server (listening) socket
  int cfd;  // communications socket with client
  struct sockaddr_in saddr;  // info concerning server
  struct sockaddr_in caddr;  // info concerning connecting clients
  unsigned char buffer[128]; // arbitrary data buffer
  int x, y, retval;             // some values to play with
  socklen_t xlen;

  saddr.sin_family = AF_INET; 
  saddr.sin_addr.s_addr = htonl(INADDR_ANY); // wildcard
  saddr.sin_port = htons(atoi(argv[1]));  // local port to bind to

  sfd = socket(AF_INET,SOCK_STREAM,0); // create server socket;
  bind(sfd,(struct sockaddr*)&saddr,sizeof(saddr));
  listen(sfd,16); // set socket queue length

  while(1)  // main server loop
    {

     // wait for client connection:
     xlen = sizeof(caddr);  // in-out parameter to accept:
     cfd = accept(sfd,(struct sockaddr*)&caddr,&xlen);
     if (cfd < 0) { perror("socket error\n"); exit(1); }
 
     // at this point, tcp connection is in ESTABLISHED state, and
     // communication with peer can begin (yeah!)
    
     x = 10; // number of bytes to send;
     y = htonl(x); // convert to network byte ordering
     write(cfd,&y,sizeof(int)); // send number of bytes to expect
     write(cfd,buffer,x); // write data
     retval = read(cfd,buffer,8); // read up to 8 bytes
     if (retval != 8) {printf("didn't get 8 bytes!\n"); exit(1);}

     // might want to know who just connected to you:
     printf("client from %s has been served\n",inet_ntoa(saddr.sin_addr));
     
     close(cfd); // note, only communication socket closed here
    } // server loop  
} // main

// this program must be killed externally (cntl-c or kill -9)

