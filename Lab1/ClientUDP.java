import java.net.DatagramPacket;
import java.net.DatagramSocket;
import java.net.InetAddress;
import java.util.Arrays;

public class ClientUDP {
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
         System.out.println("Usage: ClientUDP hostname op message");
      }
      
      addr = InetAddress.getByName(args[1]);
    
      if (args[3].length() < 1024) 
         str = args[3];
      else 
         str = args[3].substring(0, 1024); 
              
      op = Byte.parseByte(args[2]);
      length = (short)str.length();
      length += 5;
      sendData = new byte[length];
      sendData[0] = (byte)((length >> 8) & 0xff);
      sendData[1] = (byte)(length);
      sendData[2] = (byte)((id >> 8) & 0xff);
      sendData[3] = (byte)(id & 0xff);
      sendData[4] = op;
      
      for (char ch: str.toCharArray()){
         
         sendData[i] = (byte)ch;
         i++;
      }
      
      sendPacket = new DatagramPacket(sendData, (int)length, addr, portNum);
      clientSocket.send(sendPacket);
      DatagramPacket receivePacket = new DatagramPacket(receiveData, receiveData.length);
      clientSocket.receive(receivePacket);
      
      if (op == 0x55){
         int numVowel = receiveData[4];
         numVowel = numVowel << 8;
         numVowel = numVowel | receiveData[5];
         
         System.out.println(str + " contains " + numVowel + "vowels.");
      }
      else if (op == 0xAA) {
         String dv = "";
         for(i = 4; i < receiveData.length; i++)
         {
            dv += (char)receiveData[i];
         }
         System.out.println(str + " converts to " + dv);
      }    
   }
}
