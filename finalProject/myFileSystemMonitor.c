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
#include <linux/limits.h>				// for PATH_MAX define

int main(int argc, char const *argv[]) {

	int opt = 0;
	extern char *optarg;
	char dir_name[200];			 		// holds the file name
	char IP_ADDR[21];			 		// hold the IP address
	char* curr_dir[PATH_MAX];	 		        // holds the current working directory

	getcwd(curr_dir, sizeof(curr_dir));	                // get the current working directory

	// handle the receiving arguments
	while( (opt = getopt(argc, argv, "d:i:")) != -1) {
	        switch (opt) {
	            case 'i':{
	            	strcpy(IP_ADDR, optarg);
	            }
	            case 'd': {                                             
	                if(strcmp(optarg, curr_dir) != 0)
	                    strcpy(dir_name, optarg);
	                else
	                    printf("directory Error!\n");
	                break;
	            }
	            default:
	                printf("Bad arguments\n");
	                break;
	        }
	    }
}

