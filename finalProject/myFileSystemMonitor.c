/*
 * myFileSystemMonitor.c
 *
 *  Created on: 6 Feb 2021
 *      Author: opvmlinux
 */

#include <errno.h>
#include <poll.h>
#include <stdio.h>
#include <stdlib.h>
#include <sys/inotify.h>
#include <unistd.h>
#include <time.h>
#include <fcntl.h>
#include <string.h>
#include <sys/socket.h> 
#include <pthread.h>
#include <sys/types.h> 
#include <netinet/in.h> 
#include <arpa/inet.h>
#include <libcli.h>
#include <getopt.h>
#include <execinfo.h>				// for PATH_MAX define

#define PORT 10000				//Netcat server port
#define SIZE 2048				

/*
*	sendInfoToUDP()-
	ends a message to the netcat server When theres a notify event.
*/
void sendInfoToUDP(char* name, char* access, char* time, char* ip){

	int nsent;
	char SendClient[SIZE];
	memset(SendClient, 0, sizeof(SendClient));

	//UDP client
	int sock;
	struct sockaddr_in  server_addr = {0};
	server_addr.sin_family = AF_INET;
	server_addr.sin_port = htons(PORT);
	
	sock = socket(AF_INET, SOCK_DGRAM, 0);
	if(sock < 0)
	{
		perror("\nsocket");
		exit(EXIT_FAILURE);
	}
	
	if(inet_pton(AF_INET, ip, &server_addr.sin_addr.s_addr)<=0)  
   	{ 
        	perror("\nAddress not supported"); 
        	exit(EXIT_FAILURE); 
   	} 
	
	
	if(connect(sock, (struct sockaddr*)&server_addr, sizeof(server_addr)) < 0)
	{
		perror("\nconnect");
		exit(EXIT_FAILURE);
	}
	
	strcpy(SendClient, "\nFILE ACCESSED: ");
	strcat(SendClient, name);	
	strcat(SendClient, "\nACCESS: ");
	strcat(SendClient, access);
	strcat(SendClient, "\nTIME OF ACCESS: ");
	strcat(SendClient, time);
	strcat(SendClient, "\n\0");
	
	if((nsent = send(sock, SendClient, strlen(SendClient), 0)) < 0)
	{
		perror("\nrecv");
		exit(EXIT_FAILURE);
	}
	
	close(sock);

	exit(0);
}

/*
 *	handle_events() -
 *	called When an event occurd in the listening directory.
 *	The function writes to apache html page and calls sendInfoToUDP().
 */

static void handle_events(int fd, int *wd, int htmlFd, char* directory, char* ip){

	char buf[4096]
	    __attribute__ ((aligned(__alignof__(struct inotify_event))));
	const struct inotify_event *event;
	int i;
	ssize_t len;
	char *ptr;
	
	char access[50];
	char eventTime[32];
	char eventName[1024];
	
	//netcat
	pid_t pid;
	
	/* Loop while events can be read from inotify file descriptor. */

	for (;;) {

		/* Read some events. */

		len = read(fd, buf, sizeof buf);
		if (len == -1 && errno != EAGAIN) {
			perror("read");
			exit(EXIT_FAILURE);
		}


		if (len <= 0)
			break;

		/* Loop over all events in the buffer */

		char* str = malloc(1024);
		for (ptr = buf; ptr < buf + len; ptr += sizeof(struct inotify_event) + event->len) 
		{	
			event = (const struct inotify_event *) ptr;

			/* event time */
			time_t t = time(NULL);
			struct tm* tm = localtime(&t);
			
		/*if (event->mask & IN_OPEN)
			printf("IN_OPEN: \n");*/
				
		if (!(event->mask & IN_OPEN))
		{	
			memset(eventName, 0, 1024);
			memset(access, 0, 50);
			memset(eventTime, 0, sizeof (eventTime));
			strftime(eventTime, 26, "%Y-%m-%d %H:%M:%S", tm);

			if (event->mask & IN_CLOSE_NOWRITE)
				strcpy(access, "NO_WRITE ");
			if (event->mask & IN_CLOSE_WRITE)
				strcpy(access, "WRITE ");	
			
	
			write(htmlFd, "FILE ACCESSED:  ", strlen("FILE ACCESSED:  "));
			
			if(event->mask & IN_ISDIR)
				write(htmlFd, " [directory] ", strlen(" [directory] "));
				
			else
				write(htmlFd, " [file] ", strlen(" [file] "));
				
			if (event->len){
				if(event->mask & IN_ISDIR){
					strcpy(eventName, directory);
					strcat(eventName, "/");
	 				strcat(eventName, event->name);
	 				write(htmlFd, eventName, strlen(eventName));
				}
				else{
	 				write(htmlFd, event->name, strlen(event->name));
	 				strcpy(eventName, event->name);
	 			}
	 		}
	 		else{
	 			write(htmlFd, directory, strlen(directory));
	 			strcpy(eventName, directory);
	 		}
	 
	 		write(htmlFd, ", &nbspACCESS:  ", strlen(", &nbspACCESS:  "));
	 		write(htmlFd, access, strlen(access));
	 		write(htmlFd, ", &nbspTIME OF ACCESS: ", strlen(", &nbspTIME OF ACCESS: "));
	 		write(htmlFd, eventTime, strlen(eventTime));
	 		write(htmlFd, "<br><br>\n\n", strlen("<br><br>\n\n"));
	 		 		
	 		
			pid = fork();
			if(pid == -1)
				perror("fork");
			if(pid == 0)
				sendInfoToUDP(eventName, access, eventTime, ip);
		}
	}
    }
}


int main(int argc, char *argv[]) {

	// getopt 
	int option;
	char *dic_input, *ip_input;
	while((option = getopt(argc, argv, "d:i:")) != -1){
    	switch (option) {
			case 'd':
				dic_input = optarg;
				break;
			case 'i':
				ip_input = optarg;
				break;
			default:
				return EXIT_FAILURE;
		}
	}
	printf("%s\n", dic_input);
	printf("%s\n", ip_input);
	
	//webserver section -
	char buf;
	int fd, i, poll_num;
	int wd;
	int htmlFd;
	nfds_t nfds;
	struct pollfd fds[2];
	
	/* Open 'index.html' to append events */
	htmlFd = open("/var/www/html/index.html", O_WRONLY | O_TRUNC);
	
	if(htmlFd == -1)
		perror("open");
		
	if (argc != 5) 
    	{
		printf("bad arguments!\n");
		exit(EXIT_FAILURE);
	}
    
	printf("Press 'ENTER' key to terminate.\n");

	/* Create the file descriptor for accessing the inotify API */

	fd = inotify_init1(IN_NONBLOCK);
	if (fd == -1) 
	{
		perror("inotify_init1");
		exit(EXIT_FAILURE); 
	}


	/* Mark directories for events
	   - file was opened
	   - file was closed */
	wd = inotify_add_watch(fd , dic_input , IN_OPEN | IN_CLOSE_WRITE | IN_CLOSE_NOWRITE);
	if(wd == -1)
	{
		fprintf(stderr, "Cannot watch '%s'\n", dic_input);
		perror("inotify_add_watch");
		exit(EXIT_FAILURE);
	}
	

	/* Prepare for polling */

	nfds = 2;

	/* Console input */

	fds[0].fd = STDIN_FILENO;
	fds[0].events = POLLIN;

	/* Inotify input */

	fds[1].fd = fd;
	fds[1].events = POLLIN;

	/* Wait for events and/or terminal input */
	write(htmlFd, "<html><head>  <meta http-equiv= 'refresh' content= '5'></head><body>", strlen("<html><head>  <meta http-equiv= 'refresh' content= '5'></head><body>"));

	printf("Listening for events.\n");
	
	while (1) {
        	
		poll_num = poll(fds, nfds, -1);
		if (poll_num == -1) {
			if (errno == EINTR)
				continue;
			perror("poll");
			exit(EXIT_FAILURE);
		}

		if (poll_num > 0) {

			if (fds[0].revents & POLLIN) {

				/* Console input is available. Empty stdin and quit */

				while (read(STDIN_FILENO, &buf, 1) > 0 && buf != '\n')
					continue;
				break;
			}

			if (fds[1].revents & POLLIN) {

				/* Inotify events are available */
				handle_events(fd, &wd, htmlFd, dic_input, ip_input);
			}
		}
	}

	printf("Listening for events stopped.\n");

	write(htmlFd, "</body></html>", strlen("</body></html>"));

	/* Close inotify file descriptor */
	close(htmlFd);
	close(fd);
	exit(EXIT_SUCCESS);
}

