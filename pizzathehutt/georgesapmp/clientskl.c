/* tcp client skeleton */

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
  if (argc != 6)
  {
	printf("usage: %s <client ip> <internal port> <external port> <protocol> <server ip>\n", argv[0]);
	exit(1);
  }
  // declare structure to hold socket address info:
  struct sockaddr_in serveraddr;
  int retval; // many functions return values to indicate success/failure
  int x;  // some data
  unsigned short mappedport;
  unsigned int extip;
  char* ipstr = argv[1];
  char* internal_portstr = argv[2];  
  char* external_portstr =argv[3]; 
  char* protostr = argv[4];
  char* serveripstr = argv[5];
  short srvport = 6391; 
  unsigned int clientip = (inet_addr(ipstr));
  unsigned short internal_port = htons((short) atoi(internal_portstr));
  unsigned short external_port = htons((short) atoi(external_portstr));
  unsigned char proto = (char) atoi(protostr);

  serveraddr.sin_family = AF_INET;  // always for IPv4
  serveraddr.sin_addr.s_addr = inet_addr(serveripstr);
  serveraddr.sin_port = htons(srvport);

  cfd = socket(AF_INET,SOCK_STREAM,0); // register tcp socket with OS

  // make connection to server.
  retval = connect(cfd,(struct sockaddr*)&serveraddr,sizeof(serveraddr));
  // note the type cast to more generic type (polymorphism)
  if (retval != 0) { perror("socket failure!"); exit(1); }
  
  // at this point, tcp connection is in ESTABLISHED state, and
  // communication with peer can begin:  (yeah!)
  write(cfd, &clientip, 4);
  write(cfd, &internal_port, 2);
  write(cfd, &external_port, 2);
  write(cfd, &proto, 1);

  readfully(cfd,&mappedport,sizeof(short)); // read 4 bytes
  mappedport = ntohs(mappedport);
  readfully(cfd,&extip,sizeof(int));       // read x bytes into buffer
  struct in_addr s;
  s.s_addr = extip;
  printf("port: %d ip: %s\n", mappedport, inet_ntoa(s));
  printf("complete\n");
  
  close(cfd); // close socket
  exit(0);
}

/*
   Compile on Linux with "gcc clientskl.c"

   Solaris and other Unix systems (possibly cygwin also) may require
   "gcc -lnsl -lsocket clientskl.c"
*/
