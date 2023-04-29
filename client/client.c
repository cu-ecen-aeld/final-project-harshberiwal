/*
 * filename: client.c
 * Created by: Shashank Chandrasekaran
 * Description: Client side C code to receive image from server
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

int main(int argc, char const* argv[])
{
	int status, client_fd, fd_status, read_status, write_status;
	struct sockaddr_in serv_addr;
	char buffer[1];
	int bytes_read=0;
	unsigned int GPIO=23;

	//GPIO iniitializing code
	if(gpioInitialise() < 0)
		exit(7);
	gpioWrite(GPIO, 0);
 	
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
		if(recv(client_fd, &img_size, sizeof(img_size), 0) == -1) {
			syslog(LOG_ERR,"Could not read image size from server");
			exit(7);
		}
		printf("Reading byte by byte from socket and writing to .JPG file\n"); 
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
			//printf("%c",buffer[0]); //Debuggig
			bytes_read++;
			if(bytes_read == img_size) 
				break; 
		}
		printf("\n%d Bytes are read from socket\n",bytes_read);
		bytes_read=0;
		close(fd_status);
		gpioWrite(GPIO, 1);
	}
	
	//Closing the connected socket
	close(client_fd);
	return 0;
}