import java.net.*;

public class UDPServer {
	public static void main(String args[]) throws Exception {
		int portNum;
		byte[] receiveData = new byte[1024];
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

			DatagramPacket receivePacket = new DatagramPacket(receiveData, receiveData.length);
			serverSocket.receive(receivePacket);
			InetAddress IPAddress = receivePacket.getAddress();
			System.out.println("Received packet from " + IPAddress);
			
			length = getLength(receiveData);
			if (!testLength(receiveData, length)){
            sendData = new byte[5];
				sendData[0] = 0x01;
				sendData[1] = 127;
				sendData[2] = 127;
				sendData[3] = 0x00;
				sendData[4] = 0x00;
				DatagramPacket sendPacket = new DatagramPacket(sendData, sendData.length, IPAddress, portNum);
				serverSocket.send(sendPacket);
            
         } else if(!testChecksum(receiveData)) {
            GID = getGID(receiveData);
				requestID = getRequestID(receiveData);
            sendData = new byte[5];
            sendData[1] = GID;
            sendData[2] = requestID;
            sendData[3] = 0x00;
            sendData[4] = 0x00;
            sendData[0] = (byte)calcChecksum(sendData);
            DatagramPacket sendPacket = new DatagramPacket(sendData, sendData.length, IPAddress, portNum);
				serverSocket.send(sendPacket);
            
         }else {
				GID = getGID(receiveData);
				requestID = getRequestID(receiveData);
				numipAddresses = getNumIP(receiveData);
				length = (short) (5 + numipAddresses * 4);
				sendData = new byte[length];
				packLength(sendData, length);
				sendData[3] = GID;
				sendData[4] = requestID;
				
				hostnames = getHostnames(numipAddresses, receiveData);
				packIP(sendData, hostnames);
            
            sendData[2] = (byte) (calcChecksum(sendData) & 0xFF);
				DatagramPacket sendPacket = new DatagramPacket(sendData, sendData.length, IPAddress, portNum);
				serverSocket.send(sendPacket);
			}
		}
	}
	
	public static short getLength(byte[] data) {
		 return (short) ( ((data[0] << 8) | (data[1]))& 0xFF);
	}
	
	public static int getChecksum(byte[] data) {
		 return (int) data[2];
	}
	
	public static byte getGID(byte[] data) {
		 return data[3];
	}
	
	public static byte getRequestID(byte[] data) {
		 return data[4];
	}
	
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
   
	public static boolean testChecksum(byte[] data) {
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
		if (sum == 0xFF)
			return true;
		
		return false; 
	}
	
	public static boolean testLength(byte[] data, short length) {
		if (data.length == length)
			return true;
		else 
			return false;
	}
	
	public static boolean valid(byte[] data, short length, int checksum) {
		if (testLength(data, length) && testChecksum(data, checksum))
			return true;
		else
			return false;
	}
	
	public static short getNumIP(byte[] data) {
		short num = 0;
		for (int i = 0; i < data.length; i++) {
			if (data[i] == 127)
				num++;
		}
		return (short) (num - 1);
	}
	
	public static String[] getHostnames(int num, byte[] data) {
		String hostname = "";
		String hostnames[] = new String[num];
		
		int counter  = 0;
		
		for(int i = 0; i < data.length; i++) {
			if (data[i] == 127) {
				counter++;
			}
			else
				hostname += (char)data[i];
		}
		return hostnames;
	}
	
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
	
	public static void packLength(byte[] data, short length) {
		int tmp = ((length >> 8) & 0x00ff) & 0xFF;
		data[0] = (byte) tmp;
		tmp = (length & 0x00ff) & 0xFF;
		data[1] = (byte) tmp;
	}
	
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
