import java.io.*;
import java.net.*;
import java.util.Arrays;

class ServerTCP {
   public static void main(String argv[]) throws Exception {
      byte[] request = new byte[1029];
      byte[] response = new byte[1029];
      char[] outputStr = new char[1024];
   		
      if (argv.length != 1) {
         System.out.println("Usage: ServerTCP portNum");
         System.exit(0);
      }
   		
      ServerSocket socket = new ServerSocket(Integer.parseInt(argv[0]));
   
      while(true){
         System.out.println("\n\nWaiting to receive...");
         Socket connectionSocket = socket.accept();
			
         DataInputStream inFromClient =
               new DataInputStream(connectionSocket.getInputStream());
         DataOutputStream outToClient = new DataOutputStream(connectionSocket.getOutputStream());
			
         inFromClient.read(request);
   		
         System.out.println("Got input stream");
         
         fillResponse(request, response);
			
      	outToClient.write(response, 0, 1029);
			response = new byte[1029];
			
         
      }
   }
   public static void fillResponse(byte[] request, byte[] response) {
      short length;
      short id;
      int op;
      String message;
   	
      length = putLength(request);
      id = putID(request);
      op = putOP(request);
      message = putMessage(request, length);
   	
   	
      if (op == 85) {
         response[0] = 0;
         response[1] = 6;
			response[2] = request[2];
      	response[3] = request[3];
         short nv = numVowels(message);
         response[4] = (byte) ((nv >> 8)& 0x00ff);
         response[5] = (byte) (nv & 0x00ff);
         System.out.println("Length: " + 6 + " ID: " + id + " Num vowels: " + nv);
      }
      else if (op == 170) {
         String dv = "";
         byte[] bMessage;
         int rLength;
         dv = disemvowel(message, length-5);
         bMessage = dv.getBytes();
         rLength = dv.length() + 4;
         response[0] = (byte) ((rLength >> 8)& 0x00ff);
         response[1] = (byte) (rLength & 0x00ff);
			response[2] = request[2];
      	response[3] = request[3];
         for (int i = 4; i < rLength; i++) {
            response[i] = bMessage[i-4];
         }
         System.out.println("Length: " + rLength + " ID: " + id + " Message: " + dv);
      }
   }
	
   public static short putLength(byte[] data) {
      return (short) (((data[0] << 8) | (data[1])) & 0xff);
   }

   public static short putID(byte[] data) {
      return (short) (((data[2] << 8) | (data[3])) & 0xff);
   }
	
   public static int putOP(byte[] data) {
      return (int)(data[4] &0xFF);
   }
	
   public static String putMessage(byte[] data, int length) {
      return new String(Arrays.copyOfRange(data, 5, length));
   }
	
   private static int isVowel(char c)
   {
      switch(c) {
         case 'a':
         case 'A':
         case 'e':
         case 'E':
         case 'i':
         case 'I':
         case 'o':
         case 'O':
         case 'u':
         case 'U':
            return 1;
         default:
            return 0;
      }
   }

   private static short numVowels(String str) {
      short count = 0;
      char[] tmp = str.toCharArray();
   	
      for (char ch : tmp) {
         if(isVowel(ch) == 1) {
            count++;
         }
      }
   
      return count;
   }

   private static String disemvowel(String str,  int length) {
      String dv = "";
		char[] tmp = str.toCharArray();
      for (char ch : tmp) {
         if(isVowel(ch) != 1) {
            dv += ch + "";
         }
      }
      dv += '\0';
      return dv;
   }
	 
}
