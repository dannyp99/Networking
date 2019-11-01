/* tcp client skeleton */

#include<sys/time.h>
#include<stdio.h>
#include<stdlib.h>
#include<sys/types.h>
#include<sys/socket.h>
#include<netinet/in.h>
#include<arpa/inet.h>
#include<unistd.h>
#include<netdb.h>
#include<fcntl.h>


/*
   argv[0] is always the name of the program itself (implicit)
   argv[1] will be ip address of server
   argv[2] will be port number of server
*/

/*
   The read command of c takes an argument indicating the MAXIMUM
   number of bytes to read, and returns the actual number of bytes read,
   so to keep reading until we've read exactly a cetain number of bytes,
   we should use the following:
*/

void readfully(int cfd, void* addr, int size) 
{
  int r = 0; // number of bytes read so far
  while (r<size)
    {
      r += read(cfd, (unsigned char*)(addr + r), size-r);
    }
}


int main(int argc, char **argv)
{
  int cfd; // communication file descriptor (handle on socket)
  // declare structure to hold socket address info:
  unsigned short addrPort = 6391;
  struct sockaddr_in serveraddr;
  struct in_addr sock;
  int retval; // many functions return values to indicate success/failure
  int x;  // some data
  char* internal_ip = argv[1];//grab the arguments and store them
  char* host_port = argv[2];
  char* req_port = argv[3];
  char* prot = argv[4];
  char* server_ip = argv[5];
  //Now we convert the data into it's proper types
  unsigned int writeClient = (inet_addr(internal_ip));
  unsigned short writeHost = htons((short) atoi(host_port));
  unsigned short writeReqport = htons((short) atoi(req_port));
  unsigned char writeprot = (char) atoi(prot);

  serveraddr.sin_family = AF_INET;  // always for IPv4
  serveraddr.sin_addr.s_addr = inet_addr(server_ip);
  serveraddr.sin_port = htons(addrPort);

  cfd = socket(AF_INET,SOCK_STREAM,0); // register tcp socket with OS

  // make connection to server.
  retval = connect(cfd,(struct sockaddr*)&serveraddr,sizeof(serveraddr));
  
  // note the type cast to more generic type (polymorphism)
  if (retval != 0) { perror("socket failure!"); exit(1); }
  // at this point, tcp connection is in ESTABLISHED state, and
  // communication with peer can begin:  (yeah!)
  write(cfd,&writeClient,4);
  write(cfd,&writeHost,2);
  write(cfd,&writeReqport,2);
  write(cfd,&writeprot,1);
  unsigned short port;
  unsigned int external_ip;
  readfully(cfd,&port,sizeof(short)); // read 2 bytes
  port = ntohs(port);
  readfully(cfd,&external_ip,sizeof(int)); //read 4 bytes
  sock.s_addr = external_ip;	 
  printf("port: %d, ip: %s\n", port,inet_ntoa(sock));
  printf("complete\n");
  
  close(cfd); // close socket
  exit(0);
}

/*
   Compile on Linux with "gcc clientskl.c"

   Solaris and other Unix systems (possibly cygwin also) may require
   "gcc -lnsl -lsocket clientskl.c"
*/


