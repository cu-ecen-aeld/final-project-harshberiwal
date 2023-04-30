/*
 * filename: client.c
 * Created by: Shashank Chandrasekaran
 * Modified by: Harsh Beriwal (Added GPIO logic and integrated face recognition python code)
 * Description: Client code to receive image, perform face recognition and actuate gpio
 * Date: 21-Apr-2023
 * Reference: https://www.geeksforgeeks.org/socket-programming-cc/
 */
 
#include <arpa/inet.h>
#include <stdio.h>
#include <string.h>
#include <sys/socket.h>
#include <unistd.h>
#include <stdlib.h>
#include <stdbool.h>
#include <sys/types.h>
#include <syslog.h>
#include <netdb.h>
#include <netinet/in.h>
#include <arpa/inet.h>
#include <fcntl.h>
#include <signal.h>
#include <pigpio.h>

#define PORT 9000
#define GPIO_RELAY 23
#define GPIO_LED 24

int client_fd; //Socket connection

//Signal handler
void signal_handler(int sig)
{
	printf("Entering isnide handler\n");
	if(sig==SIGINT)
	{
		syslog(LOG_INFO,"Caught SIGINT, leaving");
		printf("Caught SIGINT, leaving\n");
	}
	else if(sig==SIGTERM)
	{
		syslog(LOG_INFO,"Caught SIGTERM, leaving");
		printf("Caught SIGTERM, leaving\n");
	}
	else if (sig==SIGTSTP)
	{
		syslog(LOG_INFO,"Caught SIGTSTP, leaving");
		printf("Caught SIGTSTP, leaving\n");
	}

	syslog(LOG_ERR,"Closed connection with 10.0.0.120");
	printf("Closed connection with 10.0.0.120\n");

	//Close socket connection
	close(client_fd);
	
	exit(0); //Exit success 
}

int main(int argc, char const* argv[])
{
	int status, fd_status, read_status, write_status;
	struct sockaddr_in serv_addr;
	char buffer[1];
	int bytes_read=0;

	openlog(NULL,LOG_PID, LOG_USER); //To setup logging with LOG_USER

	//Initialize signal handlers
	if(signal(SIGINT,signal_handler)==SIG_ERR)
	{
		syslog(LOG_ERR,"SIGINT failed");
		exit(9);
	}
	if(signal(SIGTERM,signal_handler)==SIG_ERR)
	{
		syslog(LOG_ERR,"SIGTERM failed");
		exit(10);
	}
	if(signal(SIGTSTP,signal_handler)==SIG_ERR)
	{
		syslog(LOG_ERR,"SIGTSTP failed");
		exit(9);
	}

	//GPIO iniitializing code
	if(gpioInitialise() < 0)
		exit(7);
	gpioWrite(GPIO_RELAY, 1);
	gpioWrite(GPIO_LED, 0);
 	
	if ((client_fd = socket(AF_INET, SOCK_STREAM, 0)) < 0) 
	{
		syslog(LOG_ERR,"Socket creation error");
		exit(1);
	}

	serv_addr.sin_family = AF_INET;
	serv_addr.sin_port = htons(PORT);

	//Convert IPv4 and IPv6 addresses from text to binary
	if (inet_pton(AF_INET, "10.0.0.120", &serv_addr.sin_addr)<= 0) 
	{
		syslog(LOG_ERR,"Invalid address: Address not supported");
		exit(2);
	}
	
	if ((status=connect(client_fd, (struct sockaddr*)&serv_addr,sizeof(serv_addr)))< 0) 
	{
		syslog(LOG_ERR,"Connection Failed");
		exit(3);
	}
	
	while(1)
	{
		fd_status=open("/home/capture_recreate.png", O_WRONLY | O_CREAT, S_IRWXU | S_IRWXG | S_IRWXO);
		if (fd_status==-1)
		{
			syslog(LOG_ERR,"The file could not be created/found");
			exit(4);
		}
		int img_size; 
		if(recv(client_fd, &img_size, sizeof(img_size), 0) == -1) //Read image size from server
		{
			syslog(LOG_ERR,"Could not read image size from server");
			exit(8);
		}
		printf("Reading byte by byte from socket and writing to .png file\n"); 
		while(((read_status=recv(client_fd, &buffer,1,0))!=0))
		{
			if(read_status==-1)
			{
				syslog(LOG_ERR,"Could not read from server");
				exit(5);
			}
			write_status= write(fd_status,&buffer,1);
			if(write_status!=1)
			{
				syslog(LOG_ERR,"Could not write total bytes to the file");
				exit(6);
			}	
			bytes_read++;
			if(bytes_read == img_size)  //If total bytes read is equal to image size, exit from loop
				break; 
		}
		printf("%d Bytes are read from socket\n",bytes_read);
		bytes_read=0;
		close(fd_status);
		int sys_status=system("python3 /etc/face-rec-sample/face_recog.py");  //Call the face recognition python code
		if(WIFEXITED(sys_status)) 
		{
			int exit_code = WEXITSTATUS(sys_status);
			if(exit_code==1) //If face matches
			{
				gpioWrite(GPIO_LED, 0);
				gpioWrite(GPIO_RELAY, 0); //Turn on relay (active low)
				sleep(5);
				gpioWrite(GPIO_RELAY, 1); //Turn off relay
				
			}
			else if(exit_code==0)
			{
				gpioWrite(GPIO_RELAY, 1); 
				gpioWrite(GPIO_LED, 1); //Turn on intruder LED
				sleep(5);
				gpioWrite(GPIO_LED, 0); //Turn off intruder LED
			}
		}
	}
	
	closelog(); //Close syslog
	return 0;
}