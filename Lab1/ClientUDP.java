import java.net.DatagramPacket;
import java.net.DatagramSocket;
import java.net.InetAddress;
import java.util.Arrays;

public class ClientUDP {

   public static void putLength(byte[] data, short length) {
      data[0] = (byte) (length >> 8);
      data[1] = (byte) (length & 0x00ff);
   }

   public static void putID(byte[] data, short id) {
      data[2] = (byte) (id >> 8);
      data[3] = (byte) (id & 0x00ff);
   }

   public static short getLength(byte[] data) {
      return (short) ( (data[0] << 8) | (data[1]));
   }

   public static short getID(byte[] data) {
      return (short) ( (data[2] << 8) | (data[3]));
   }

   public static String getMessage(byte[] data) {
      return new String(Arrays.copyOfRange(data, 4, getLength(data)));
   }

   public static void main(String args[]) throws Exception {

      int portNum = 10038;
      DatagramPacket sendPacket;
      short id = 0;
      short length;
      String str = null;
      byte op = 0;
      byte[] sendData;
      byte[] receiveData = new byte[1029];
      InetAddress addr = null;
      int i = 5;
      DatagramSocket clientSocket = new DatagramSocket();

      if(args.length != 4)
      {
         System.out.println("Usage: hostname requestID op message");
      }

      // hostname
      addr = InetAddress.getByName(args[0]);

      // first 1024 chars of message
      if (args[3].length() < 1024)
         str = args[3];
      else
         str = args[3].substring(0, 1024);

      // requestID and op
      id = Short.parseShort(args[1]);
      op = (byte) Integer.parseInt(args[2]);

      // 1 byte for each char in message + 5 bytes for hostname,
      // requestID and op
      length = (short) str.length();
      length += 5;
      sendData = new byte[length];

      // Assuming big endian here
      putLength(sendData, length);
      putID(sendData, id);
      sendData[4] = op;

      for (char ch : str.toCharArray()){
         sendData[i++] = (byte) ch;
      }

      sendPacket = new DatagramPacket(sendData, (int) length, addr, portNum);
      DatagramPacket receivePacket = new DatagramPacket(receiveData, receiveData.length);
      long startTime = System.currentTimeMillis();

      clientSocket.send(sendPacket);
      clientSocket.receive(receivePacket);

      System.out.println("Time taken: " + (System.currentTimeMillis() - startTime) + " ms");
      System.out.println("Request ID: " + getID(receiveData));
      System.out.println("Response: " + getMessage(receiveData));

      /* I think this should be for the server:
      if (op == 0xAA) {
         int numVowel = receiveData[4];
         numVowel = numVowel << 8;
         numVowel = numVowel | receiveData[5];

         System.out.println(str + " contains " + numVowel + "vowels.");
      } else if (op == 0x55) {
         String dv = "";
         for(i = 4; i < receiveData.length; i++)
         {
            dv += (char)receiveData[i];
         }
         System.out.println(str + " converts to " + dv);
      }
      */
   }
}
