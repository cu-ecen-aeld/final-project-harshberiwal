/*
 * filename: server.c
 * Created by: Shashank Chandrasekaran 
 * Description: Server side C code to capture image from camera and transmit image to client
 * Date: 21-Apr-2023
 * Reference: AESD Assignment code server.c
 */
 
#include <stdio.h>
#include <stdlib.h>
#include <stdbool.h>
#include <string.h>
#include <sys/types.h>
#include <sys/socket.h>
#include <syslog.h>
#include <netdb.h>
#include <unistd.h>
#include <netinet/in.h>
#include <arpa/inet.h>
#include <fcntl.h>
#include <signal.h>
#include <pigpio.h>

#define GPIO_PIR 25

int socket_fd,accept_return; //Socket fd and client connection

//Structure for bind and accept
struct addrinfo hints;
struct addrinfo *servinfo;
struct sockaddr_in client_addr;

//Signal handler
void signal_handler(int sig)
{
	if(sig==SIGINT)
	{
		syslog(LOG_INFO,"Caught SIGINT, leaving");
	}
	else if(sig==SIGTERM)
	{
		syslog(LOG_INFO,"Caught SIGTERM, leaving");
	}
	
	//Close socket and client connection
	close(socket_fd);
	close(accept_return);
	syslog(LOG_ERR,"Closed connection with %s",inet_ntoa(client_addr.sin_addr));
	printf("Closed connection with %s\n",inet_ntoa(client_addr.sin_addr));
	
	exit(0); //Exit success 
}

int main(int argc, char *argv[])
{
	int add =1;
	int getaddr,sockopt_status,bind_status,listen_status; //Variables to get address for bind, return value of bind and listen
	socklen_t size=sizeof(struct sockaddr); 
	int fd_status;
	int rd_status; //Read status from .JPG file
	int send_status; //Return of send function
	char read_arr[1]; //Create a temporary buffer to read file's contents
	int bytes_send=0; //Bytes send from socket
	
	openlog(NULL,LOG_PID, LOG_USER); //To setup logging with LOG_USER

	//GPIO iniitializing code
	if(gpioInitialise() < 0)
		exit(1);
	
	//Initialize signal handlers
	if(signal(SIGINT,signal_handler)==SIG_ERR)
	{
		syslog(LOG_ERR,"SIGINT failed");
		exit(2);
	}
	if(signal(SIGTERM,signal_handler)==SIG_ERR)
	{
		syslog(LOG_ERR,"SIGTERM failed");
		exit(3);
	}
	
	//create socket
	socket_fd=socket(AF_INET, SOCK_STREAM, 0);
	if(socket_fd==-1)
	{
		syslog(LOG_ERR, "Couldn't create a socket");
		exit(4);
	}
	
	//Get server address
	hints.ai_flags=AI_PASSIVE; //Set this flag before passing to function
	getaddr=getaddrinfo(NULL,"9000",&hints,&servinfo);
	if(getaddr !=0)
	{
		syslog(LOG_ERR, "Couldn't get the server's address");
		exit(5);
	}
	
	//Reuse address to avoid bind errors
	sockopt_status=setsockopt(socket_fd,SOL_SOCKET,SO_REUSEADDR,&add,sizeof(add));
	if(sockopt_status==-1)
	{
		syslog(LOG_ERR, "Couldn't get the socket server's address");
		exit(6);
	}
	
	//Bind
	bind_status=bind(socket_fd,servinfo->ai_addr,sizeof(struct sockaddr));
	if(bind_status==-1)
	{
		freeaddrinfo(servinfo); //Free the memory before exiting
		syslog(LOG_ERR, "Binding failed");
		exit(7);
	}
	
	freeaddrinfo(servinfo); //Free this memory before next operations
	
	//Listen
	listen_status=listen(socket_fd,1); //Max queue of connections set to 20
	if(listen_status==-1)
	{
		syslog(LOG_ERR, "Listening to the connections failed");
		exit(8);
	}
	
	//accept connection
	accept_return=accept(socket_fd,(struct sockaddr *)&client_addr,&size);
	if(accept_return==-1)
	{
		syslog(LOG_ERR, "Connection could not be accepted");
		exit(9);
	}
	else
	{
		syslog(LOG_INFO,"Accepts connection from %s",inet_ntoa(client_addr.sin_addr));
		printf("Accepts connection from %s\n",inet_ntoa(client_addr.sin_addr));
	}
	
	//Infinite loop for server-client connections
	while(1)
	{
		if(gpioRead(GPIO_PIR)==0) //if PIR detects object/person at gpio pin 24
		{
			system("/home/capture"); //Create a fork process to capture an image if PIR detects motion
			fd_status=open("/usr/bin/cap.png",O_RDONLY, S_IRWXU | S_IRWXG | S_IRWXO); //Open file to read only
			if(fd_status==-1)
			{
				syslog(LOG_ERR, "Could not open the file to read");
				exit(10);
			}
			int img_size = lseek(fd_status, 0, SEEK_END); //To find total bytes in the file (image size)
			lseek(fd_status,0,SEEK_SET);
			if(send(accept_return, &img_size, sizeof(img_size),0) == -1)  //Send the image size first through socket
			{
				syslog(LOG_ERR, "The image size couldn't be send to the client");
				exit(11);
			}
			printf("\nReading one byte at a time from the image file and sending to the client\n");
			while((rd_status=read(fd_status,&read_arr,1))!=0) //Read the file and store contents in the buffer
			{
				if(rd_status==-1)
				{
					syslog(LOG_ERR, "Could not read bytes from the file");
					exit(12);
				}
		
				//Send data packet to the client 
				send_status=send(accept_return,&read_arr,1,0);
				if(send_status==-1)
				{
					syslog(LOG_ERR, "The data packets couldn't be send to the client");
					exit(13);
				}
				bytes_send++; 
			}
			printf("%d bytes transferred to the client\n", bytes_send);
			memset(read_arr,0,1);
			bytes_send=0;
			close(fd_status); //Close file after reading
			sleep(80); //Sleep after an image is send for the client to process
		}
	}
	gpioTerminate();
	closelog(); //Close syslog
	return 0;
}
