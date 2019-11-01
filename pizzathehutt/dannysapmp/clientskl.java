/* tcp client skeleton - Java version */

import java.io.*;
import java.net.*;

public class clientskl
{
  // argv[0] is server ip addr, argv[1] is server port

  public static void main(String[] argv)
    {
      Socket cfd;      // communication socket "fd" is carry-over from C
      DataInputStream din;   // binary communicators
      DataOutputStream dout;  
      byte[] buffer = new byte[128];  // data buffer
      int x, y;

      try
      {
	  // connect to server:
	  cfd = new Socket(argv[0],Integer.parseInt(argv[1]));
 	  din = new DataInputStream(cfd.getInputStream());
	  dout = new DataOutputStream(cfd.getOutputStream());
	  
	  // now in ESTABLISHED state
	  
	  x = din.readInt();
	  readFully(din,buffer,0,x);
	  dout.write(buffer,120,8);
	  System.out.println("complete");
	  cfd.close();
      } catch (Exception ee) {ee.printStackTrace(); System.exit(1);}
    } // main


    // the following function only returns after EXACTLY n bytes are read:

    static void readFully(DataInputStream din, byte[] buffer, int start, int n)
           throws IOException
    {
        int r = 0; // number of bytes read
	while (r<n)
	    { 
		r += din.read(buffer,start+r,n-r);
	    }
    }//readFully
}

