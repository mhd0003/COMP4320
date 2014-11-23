import java.io.*;
import java.net.*;
import java.util.Arrays;
import java.util.Scanner;

public class Client {
   public static void main(String args[]) throws Exception {
      int myPort;
      int serverPort;
      DatagramPacket sendPacket;
      DatagramPacket receivePacket;
      byte[] sendData = new byte[1028];
      byte[] receiveData = new byte[1028];
      InetAddress addr = null;
      boolean myMove = false;
      
      int GID = 28;
      DatagramSocket clientSocket = new DatagramSocket();
      
      if(args.length != 3)
      {
         System.out.println("Usage: hostname ServerPort MyPort");
      	System.exit(1);
		}
      
      addr = InetAddress.getByName(args[0]);
      serverPort = Integer.parseInt(args[1]);
		myPort = Integer.parseInt(args[2]);
      
      if(myPort < (10010 + (5*GID)) || myPort > (10014 + (5* GID))) {
         System.out.println("Invalid myPort");
         System.exit(1);
      }
      
      sendData[0] = 0x12;
      sendData[1] = 0x34;
      sendData[2] = (byte) GID;
      sendData[3] = (byte) (myPort >> 8);
      sendData[4] = (byte) (myPort & 0x000000FF);
      
      sendPacket = new DatagramPacket(sendData, (int) 1028, addr, serverPort);
      receivePacket = new DatagramPacket(receiveData, receiveData.length);
      
      clientSocket.send(sendPacket);
      clientSocket.receive(receivePacket);
      
      clientSocket.close();
      
      if(getClientType(receiveData) == 1) {
         ServerSocket socket = new ServerSocket(myPort);
         sendData = new byte[1028];
         
         int[] nim = {1,3,5,7};
         int row = 0;
         int token = 0;
         
         System.out.println("\nRow#\t:Number of tokens");
         System.out.println("1\t:" + nim[0]);
         System.out.println("2\t:" + nim[1]);
         System.out.println("3\t:" + nim[2]);
         System.out.println("4\t:" + nim[3]);
         System.out.println("Waiting  for a player to connect...\n");
         
         Socket connectionSocket = socket.accept();
         
         System.out.println("Player connected.. Waiting for turn...");
         System.out.println("-----------------------------------------------------\n");
         
         DataInputStream inFromClient = new DataInputStream(connectionSocket.getInputStream());
         DataOutputStream outToClient = new DataOutputStream(connectionSocket.getOutputStream());
         
         while(true) {
            receiveData = new byte[1028];
            sendData = new byte[1028];
            
            inFromClient.read(receiveData);
            myMove = true;
            if (isInvalidMove(receiveData)) {
               playTurn(sendData, nim);
               outToClient.write(sendData, 0, 1028);
               myMove = false;
            }
            else if(!setBoard(receiveData, nim)) {
               sendData[0] = 0x12;
               sendData[1] = 0x34;
               sendData[2] = (byte)GID;
               sendData[3] = -127;
               sendData[4] = -127;
               outToClient.write(sendData, 0, 1028);
               myMove = false;
            }
            else if ((nim[0] + nim[1] + nim[2] + nim[3]) == 1 && myMove == true) {
               System.out.println("\nRow#\t:Number of tokens");
               System.out.println("1\t:" + nim[0]);
               System.out.println("2\t:" + nim[1]);
               System.out.println("3\t:" + nim[2]);
               System.out.println("4\t:" + nim[3]);
               System.out.println("Game Over! You lost!\n");
               System.exit(0);
            }
            else {
               playTurn(sendData, nim);
               outToClient.write(sendData, 0, 1028);
               myMove = false;
              
            }
            
            if((nim[0] + nim[1] + nim[2] + nim[3]) == 1 && myMove == false) {
               System.out.println("\nRow#\t:Number of tokens");
               System.out.println("1\t:" + nim[0]);
               System.out.println("2\t:" + nim[1]);
               System.out.println("3\t:" + nim[2]);
               System.out.println("4\t:" + nim[3]);
               System.out.println("Game Over! You Won!\n");
               System.exit(0);
            }
         }
      }
      else if (getClientType(receiveData) == 2)
      {
         sendData = new byte[1028];
         myPort = (receiveData[7] & 0xFF) << 8;
         myPort = (receiveData[8] & 0xFF) | myPort;
         System.out.println(myPort);
         int[] nim = {1,3,5,7};
         int row = 0;
         int token = 0;
         
         Socket gameClientSocket = new Socket(getIP(receiveData), myPort);
         
         DataInputStream inFromClient = new DataInputStream(gameClientSocket.getInputStream());
         DataOutputStream outToClient = new DataOutputStream(gameClientSocket.getOutputStream());
         
         playTurn(sendData, nim);
         outToClient.write(sendData, 0, 1028);
         
         while(true) {
            myMove = true;
            receiveData = new byte[1028];
            sendData = new byte[1028];
            
            inFromClient.read(receiveData);
            
            if (isInvalidMove(receiveData)) {
               playTurn(sendData, nim);
               outToClient.write(sendData, 0, 1028);
               myMove = false;
            }
            else if(!setBoard(receiveData, nim)) {
               sendData[0] = 0x12;
               sendData[1] = 0x34;
               sendData[2] = (byte)GID;
               sendData[3] = -127;
               sendData[4] = -127;
               outToClient.write(sendData, 0, 1028);
               myMove = false;
            }
            else if ((nim[0] + nim[1] + nim[2] + nim[3]) == 1 && myMove == true) {
               System.out.println("\nRow#\t:Number of tokens");
               System.out.println("1\t:" + nim[0]);
               System.out.println("2\t:" + nim[1]);
               System.out.println("3\t:" + nim[2]);
               System.out.println("4\t:" + nim[3]);
               System.out.println("Game Over! You lost!\n");
               System.exit(0);
            }
            else {
               playTurn(sendData, nim);
               outToClient.write(sendData, 0, 1028);
               myMove = false;
               
            }
            
            if((nim[0] + nim[1] + nim[2] + nim[3]) == 1 && myMove == false) {
               System.out.println("\nRow#\t:Number of tokens");
               System.out.println("1\t:" + nim[0]);
               System.out.println("2\t:" + nim[1]);
               System.out.println("3\t:" + nim[2]);
               System.out.println("4\t:" + nim[3]);
               System.out.println("Game Over! You Won!\n");
               System.exit(0);
            }

         }
      }
      else  {
         getError(receiveData);
         System.exit(1);
      }
   }
   
   public static InetAddress getIP(byte[] receiveData) throws UnknownHostException{
      byte[] ip = new byte[4];
      ip[0] = receiveData[3];
      ip[1] = receiveData[4];
      ip[2] = receiveData[5];
      ip[3] = receiveData[6];
      InetAddress addr = InetAddress.getByAddress(ip);
      return addr;
   }
   
   public static int getClientType(byte[] data) {
      int clientOrServer = -1;
      
      if (checkMagic(data)) {
         
         if (isInvalid(data))
            clientOrServer = 0;
         
         else if (getLength(data) == 9)
            clientOrServer = 2;
         
         else if (getLength(data) == 5)
            clientOrServer = 1;
      }
      return clientOrServer;
   }
   
   public static int getLength(byte[] data) {
		int length = 0;
      int i = data.length-1;
      boolean start = false;
      for(int j = i; i >= 0; i--) {
         if(start){
            length++; 
         }
         else if (data[i] != 0){
            start = true;
            length++;
         }
      }
      
      return length;
	}
   
   public static void getError(byte[] data) {
      int tmp = (data[4] & 0xFF);
      System.out.println(tmp);
      if (tmp == 1)
         System.out.println("No magic number");
      else if (tmp == 2)
         System.out.println("Incorrect length");
      else if (tmp == 4)
         System.out.println("Port out of range");
      else 
         System.out.println("Invalid magic number");
   }
   
   public static boolean checkMagic(byte[] data) {
      boolean invalid = false;
      int tmp = (data[0] & 0xFF);
      tmp = tmp << 8;
      tmp = tmp | (data[1] & 0xFF);
      
      if (tmp == 4660)
         invalid = true;
         
      return invalid;
   }
   
   public static boolean setBoard(byte[] data, int[] nim) {
      int row = (data[3] & 0xFF);
      int token = (data[4] & 0xFF);
      boolean set = true;
      
      if(checkMagic(data)) {
         if (!validMove(row, token, nim)) {
            set = false;
         }
         else {
            nim[row-1] = nim[row-1] - token;
         }
      }
      return set;
   }
   
   public static boolean isInvalid(byte[] data) {
      boolean invalid = false;
      int tmp = (data[3] & 0xFF);
      if (tmp == 0)
         invalid = true;
      
      return invalid;
   }
   
   public static boolean isInvalidMove(byte[] data) {
      boolean invalid = false;
      
      if(data[3] == -127 && data[4] == -127)
         invalid = true;
      
      return invalid;
   }
   
   public static boolean validMove(int row, int token, int[] nim) {
      int[] tmp = nim.clone();
      
      if(row < 1 || row > 4) {
         return false;
      }
      
      if ((nim[row-1] - token) < 0) {
         return false;
      }
      if (token < 1 ) {
         return false;
      }
      
      tmp[row-1] = tmp[row-1] - token;
      if ((tmp[0] + tmp[1] + tmp[2] + tmp[3]) < 1) {
         return false;
      }
      
      return true;
   }
   
   public static void playTurn(byte[] data, int[] nim) {
      int row = 0;
      int token = 0;
      
      Scanner in = new Scanner(System.in);
      
      System.out.println("\nRow#\t:Number of tokens");
      System.out.println("1\t:" + nim[0]);
      System.out.println("2\t:" + nim[1]);
      System.out.println("3\t:" + nim[2]);
      System.out.println("4\t:" + nim[3]);
      System.out.println("Choose Row: ");
      row = Integer.parseInt(in.nextLine());
      System.out.println("Choose amount of tokens: ");
      token = Integer.parseInt(in.nextLine());
      
      if (validMove(row, token, nim)) {
         nim[row-1] = nim[row-1] - token;
      }
      
      data[0] = 0x12;
      data[1] = 0x34;
      data[2] = 28;
      data[3] = (byte) row;
      data[4] = (byte) token;
      System.out.println("Sending move... Waiting for Opponent's move...");
      System.out.println("-----------------------------------------------------");
   }
}
