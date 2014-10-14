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
		String[] ipAddresses;
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
			checksum = getChecksum(receiveData);
			if (valid(receiveData, length, checksum)){
				GID = getGID(receiveData);
				requestID = getRequestID(receiveData);
				numipAddresses = getNumIP(receiveData);
				length = (short) (5 + numipAddresses * 4);
				sendData = new byte[length];
				packLength(sendData, length);
				sendData[2] = (byte) checksum;
				sendData[3] = GID;
				sendData[4] = requestID;
				
				ipAddresses = getHostnames(numipAddresses, receiveData);
				for (int i = 0; i < ipAddresses.length; i++) {
					ipAddresses[i] = getIPAddress(ipAddresses[i]);
				}
				packIP(sendData, ipAddresses);
				DatagramPacket sendPacket = new DatagramPacket(sendData, sendData.length, IPAddress, portNum);
				serverSocket.send(sendPacket);
			}
			else
			{
				sendData = new byte[5];
				sendData[0] = 0x01;
				sendData[1] = 127;
				sendData[2] = 127;
				sendData[3] = 0x00;
				sendData[4] = 0x00;
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
	
	public static boolean testChecksum(byte[] data, int checksum) {
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
		
		if (sum == checksum)
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
				hostnames[counter] = hostname.replace(".",  "");
				counter++;
			}
			else
				hostname += (char)data[i];
		}
		return hostnames;
	}
	
	public static String getIPAddress(String hostname) {
		try {
			return InetAddress.getByName(hostname).toString();
		}
		catch (UnknownHostException e)
		{
			return "255.255.255.255";
		}
	}
	
	public static void packLength(byte[] data, short length) {
		int tmp = ((length >> 8) & 0x00ff) & 0xFF;
		data[0] = (byte) tmp;
		tmp = (length & 0x00ff) & 0xFF;
		data[1] = (byte) tmp;
	}
	
	public static void packIP(byte[] data, String[] IP) {
		int p = 5;
		for (int i = 0;  i < IP.length; i++) {
			byte[] tmp = IP[i].getBytes();
			for (int k = 0; k < tmp.length; k++) {
				data[p] = tmp[k];
				p++;
			}
		}
	}
}
