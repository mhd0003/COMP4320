import java.net.*;

public class UDPServer {
	public static void main(String args[]) throws Exception {
		int portNum;
		byte[] receiveData = new byte[1028];
		byte[] sendData = null;
		short length;
		int checksum;
		byte GID;
		byte requestID;
		String[] hostnames;
		short numipAddresses = 0;
		
		if( args.length != 1) {
			System.out.println("Invalid amount of arguments");
			System.exit(0);
		}
		
		portNum = Integer.parseInt(args[0]);
		DatagramSocket serverSocket = new DatagramSocket(portNum);
		while(true) {
         System.out.println("\nWaiting for packet...");
         sendData = new byte[1024];
			DatagramPacket receivePacket = new DatagramPacket(receiveData, receiveData.length);
			serverSocket.receive(receivePacket);
			InetAddress IPAddress = receivePacket.getAddress();
			System.out.println("Received packet from " + IPAddress);
			length = getLength(receiveData);
			if (!testLength(receiveData, length)){
            System.out.println("UDPServer: bad length");
				sendData[0] = 0x01;
				sendData[1] = 127;
				sendData[2] = 127;
				sendData[3] = 0x00;
				sendData[4] = 0x00;
				DatagramPacket sendPacket = new DatagramPacket(sendData, sendData.length, IPAddress, portNum);
				serverSocket.send(sendPacket);
            
         } else if(!testChecksum(receiveData)) {
            System.out.println("UDPServer: bad checksum");
            GID = getGID(receiveData);
				requestID = getRequestID(receiveData);
            sendData[1] = GID;
            sendData[2] = requestID;
            sendData[3] = 0x00;
            sendData[4] = 0x00;
            sendData[0] = (byte)calcChecksum(sendData);
            DatagramPacket sendPacket = new DatagramPacket(sendData, sendData.length, IPAddress, portNum);
				serverSocket.send(sendPacket);
            
         }else {
            System.out.println("UDPServer: Valid checksum and length");
				GID = getGID(receiveData);
				requestID = getRequestID(receiveData);
				numipAddresses = getNumIP(receiveData);
            hostnames = getHostnames(numipAddresses, receiveData, length);
				length = (short) (5 + numipAddresses * 4);
				packLength(sendData, length);
				sendData[3] = GID;
				sendData[4] = requestID;
				
				
				packIP(sendData, hostnames);
            
            sendData[2] = (byte) (calcChecksum(sendData) & 0xFF);
				DatagramPacket sendPacket = new DatagramPacket(sendData, sendData.length, IPAddress, portNum);
				serverSocket.send(sendPacket);
			}
         System.out.println("UDPServer: Sent Packet");
		}
	}
	
   // returns the length from buffer
	public static short getLength(byte[] data) {
		 return (short) ( ((data[0] << 8) | (data[1]))& 0xFF);
	}
	
   // returns the checksum from buffer
	public static int getChecksum(byte[] data) {
		 return (int) data[2];
	}
	
   // returns the GID from the buffer
	public static byte getGID(byte[] data) {
		 return data[3];
	}
	
   // returns the requestID from the buffer
	public static byte getRequestID(byte[] data) {
		 return data[4];
	}
	
   // calculates the checksum from the buffer
   public static int calcChecksum(byte[] data)
   {
      int sum = 0;
      int tmp;
		for (int i = 0; i < data.length; i++) {
			sum += data[i];
         while (sum > 255){
            tmp = sum & 0xFF;
            tmp++;
            sum = tmp;
         }
		}
		
		sum = ~sum & 0xff;
      return sum;
   }
   
   // recalculates the checksum from the buffer and compares it to the one in the buffer
	public static boolean testChecksum(byte[] data) {
		int sum = 0;
      int tmp;
		for (int i = 0; i < data.length; i++) {
         if (i != 2){
   			sum += data[i];
            while (sum > 255){
               tmp = sum & 0xFF;
               tmp++;
               sum = tmp;
            }
         }
		}
      sum = ~sum;
		if (sum == data[2] || (sum & 0x000000FF) == data[2])
			return true;
		
		return false; 
	}
	
   // calculates the length and compares to the buffer values of length
	public static boolean testLength(byte[] data, short length) {
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
	
   // validates both length and checksum
	public static boolean valid(byte[] data, short length) {
		if (testLength(data, length) && testChecksum(data))
			return true;
		else
			return false;
	}
	
   // calculates the number of IPs 
	public static short getNumIP(byte[] data) {
		short num = 0;
		for (int i = 0; i < data.length; i++) {
			if (data[i] == 126)
				num++;
		}
		return (short) (num);
	}
	
   // creates the hostnames from the buffer and stores into an array
	public static String[] getHostnames(int num, byte[] data, int length) {
		String hostname = "";
		String hostnames[] = new String[num];
		
		int counter  = 0;
      
		for(int i = 6; i <= length; i++) {
			if (data[i] == 126 || length == i) {
				hostnames[counter] = hostname;
            counter++;
            hostname = "";
			}
			else {
            hostname += (char)data[i];
         }
				
		}
		return hostnames;
	}
	
   // Gets the IP from the hostnames
	public static byte[] getIPAddress(String hostname) {
		try {
			return InetAddress.getByName(hostname).getAddress();
		}
		catch (UnknownHostException e)
		{
			byte[] tmp = new byte[4];
         tmp[0] = (byte) (255 & 0xFF);
         tmp[1] = (byte) (255 & 0xFF);
         tmp[2] = (byte) (255 & 0xFF);
         tmp[3] = (byte) (255 & 0xFF);
         return tmp;
		}
	}
	
   // packs the length into the send buffer
	public static void packLength(byte[] data, short length) {
		int tmp = ((length >> 8) & 0x00ff) & 0xFF;
		data[0] = (byte) tmp;
		tmp = (length & 0x00ff) & 0xFF;
		data[1] = (byte) tmp;
	}
	
   // packs the rest of the data into the buffer
	public static void packIP(byte[] data, String[] Host) {
		int p = 5;
		for (int i = 0;  i < Host.length; i++) {
			byte[] tmp = getIPAddress(Host[i]);
			for (int k = 0; k < tmp.length; k++) {
				data[p] = tmp[k];
				p++;
			}
		}
	}
}
