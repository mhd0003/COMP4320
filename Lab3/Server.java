import java.net.*;

public class Server {
	public static void main(String args[]) throws Exception {
		int portNum;
		byte[] receiveData = new byte[1028];
		byte[] sendData = null;
		byte GID = 28;
      int clientGID = -1;
      byte[] clientIP = null;
      int clientPort = -1;
      byte xy = 0;
      
      		
		if( args.length != 1) {
			System.out.println("Invalid amount of arguments");
			System.exit(0);
		}
		
		portNum = Integer.parseInt(args[0]);
		DatagramSocket serverSocket = new DatagramSocket(portNum);
		while(true) {
         int testValid = 0;
         System.out.println("\nWaiting for packet...");
         sendData = new byte[1024];
			DatagramPacket receivePacket = new DatagramPacket(receiveData, receiveData.length);
			serverSocket.receive(receivePacket);
			InetAddress IPAddress = receivePacket.getAddress();
			System.out.println("Received packet from " + IPAddress);
			
         
         if (valid(receiveData, xy)) {
         
            if(clientIP == null) {
               clientIP = IPAddress.getAddress();
               clientPort = (receiveData[3] & 0xFF) << 2;
               clientPort = (receiveData[4] & 0xFF) | clientPort;
               clientGID = (receiveData[2] & 0xFF);
               
               sendData[0] = (0x12 & 0xFF);
               sendData[1] = (0x34 & 0xFF);
               sendData[2] = (byte) (GID & 0xFF);
               sendData[3] = ((byte) (clientPort & 0x0000FF00));
               sendData[4] = ((byte) (clientPort & 0x000000FF));
            }
            
            if(clientIP != null) {
               sendData[0] = (0x12 & 0xFF);
               sendData[1] = (0x34 & 0xFF);
               sendData[2] = (byte) (clientGID & 0xFF);
               sendData[3] = (byte) (clientIP[0] & 0xFF);
               sendData[4] = (byte) (clientIP[1] & 0xFF);
               sendData[5] = (byte) (clientIP[2] & 0xFF);
               sendData[6] = (byte) (clientIP[3] & 0xFF);
               sendData[7] = (byte) (clientPort & 0x0000FF00);
               sendData[8] = (byte) (clientPort & 0x000000FF);
               
               clientIP = null;
               clientPort = -1;
               clientGID = -1;
            }
         }
         else {
            sendData[0] = (0x12 & 0xFF);
            sendData[1] = (0x34 & 0xFF);
            sendData[2] = (byte) (GID & 0xFF);
            sendData[3] = 0x00;
            sendData[4] = xy;
         }
			DatagramPacket sendPacket = new DatagramPacket(sendData, sendData.length, IPAddress, portNum);
			serverSocket.send(sendPacket);
         System.out.println("UDPServer: Sent Packet");
      }  
   }
   
   private static boolean valid(byte[] data, int xy) {
      boolean valid = true;
      int testValid;
      int gid = (data[2] & 0xFF);
      int portNum;
      
      portNum = (data[3] & 0xFF) << 2;
      portNum = (data[4] & 0xFF) | portNum;
      
      testValid = (data[0] & 0xFF) << 2;
      testValid = (data[1] & 0xFF) | testValid;
      
      
      if (testValid != 0x1234) {
         valid = false;
         xy = 1;
      }
      else if (testLength(data, 5)) {
         valid = false;
         xy = 2;
      }
      else if (portNum < (10010 + (gid * 5)) || portNum < (10014 + (gid * 5))) {
         valid = false;
         xy = 3;
      }
      return valid;
   }
   
   public static boolean testLength(byte[] data, int length) {
		int testLength = 0;
      int i = data.length-1;
      boolean start = false;
      for(int j = i; i >= 0; i--) {
         if(start){
            testLength++; 
         }
         else if (data[i] != 0){
            start = true;
            testLength++;
         }
      }
      if (testLength == length)
			return true;
		else 
			return false;
	}

}
