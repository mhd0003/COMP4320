import java.io.*;
import java.net.*;

class ServerTCP
{
   public static void main(String argv[]) throws Exception
   {
         String clientSentence;
         String capitalizedSentence;
			
			if (argv.length != 1) {
				System.out.println("Usage: ServerTCP portNum");
				System.exit(0);
			}
			
         ServerSocket socket = new ServerSocket(argv[0]);

         while(true)
         {
            Socket connectionSocket = welcomeSocket.accept();
            BufferedReader inFromClient =
               new BufferedReader(new InputStreamReader(connectionSocket.getInputStream()));
            DataOutputStream outToClient = new DataOutputStream(connectionSocket.getOutputStream());
            
				clientSentence = inFromClient.readLine();
            
				System.out.println("Received: " + clientSentence);
            
            outToClient.writeBytes(capitalizedSentence);
         }
    }
}
