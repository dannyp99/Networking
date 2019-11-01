/* tcp server skeleton - Java version */

import java.io.*;
import java.net.*;

public class serverskl
{
    // args[0] is port to bind to.
    public static void main(String[] argv)
    {
      ServerSocket sfd; // listening socket
      Socket cfd;       // communication socket
      DataInputStream din;   // binary communicators
      DataOutputStream dout;  
      byte[] buffer = new byte[128];  // data buffer
      int x, y;
      try 
      {
	 sfd = new ServerSocket(Integer.parseInt(argv[0]));
	 while (true) // server loop
	     {
		 // wait for client connection:
		 cfd = sfd.accept();

		 System.out.println("connection from "+cfd.getInetAddress());
	
		 // create io channels:
		 din = new DataInputStream(cfd.getInputStream());
		 dout = new DataOutputStream(cfd.getOutputStream());

		 // send number of bytes peer should expect
		 x = 10;
		 dout.writeInt(x);
		 dout.write(buffer,0,10); // write 1st 10 bytes of buffer
		 din.read(buffer,120,8); // read to last 8 bytes of buffer

		 cfd.close(); // closes everything
	     } // server loop
      } catch (Exception ee) {ee.printStackTrace(); System.exit(1);}
    } // main
}

// can create BufferedReader/PrintWriter for easy string io.

