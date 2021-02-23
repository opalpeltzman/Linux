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
#include <sys/types.h> 
#include <netinet/in.h> 
#include <arpa/inet.h>
#include <libcli.h>
#include <getopt.h>
#include <execinfo.h>				// for PATH_MAX define

/*
 *	Function: handle_events()
 *	Description: , handle_event is called When an event occurd in the listening directory.
 *	The function writes to apache html page and calls sendToUDP().
 */

static void handle_events(int fd, int *wd, int htmlFd){
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
	//printf("%s", dic_input);
	//printf("%d", argc);
	
	//webserver section -
	char buf;
	int fd, i, poll_num;
	int wd;
	nfds_t nfds;
	struct pollfd fds[2];
	
	int htmlFd = open("/var/www/html/index.html", O_WRONLY | O_TRUNC);
	if(htmlFd == -1)
		perror("open");
	
	if (argc != 5) 
    {
		printf("bad arguments!\n");
		exit(EXIT_FAILURE);
	}
	
	// UDP client
	int sockfd;
	struct sockaddr_in  server_addr;
	if((sockfd = socket(AF_INET , SOCK_DGRAM, 0)) < 0)
	{
		perror("socket");
		exit(EXIT_FAILURE);
	}
	server_addr.sin_family = AF_INET;
	if(!inet_aton(ip_input , &server_addr.sin_addr))
	{
		perror("inet_aton");
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

				handle_events(fd, &wd, htmlFd);
			}
		}
	}

	printf("Listening for events stopped.\n");

	/* Close inotify file descriptor */

	close(fd);
	exit(EXIT_SUCCESS);
}

