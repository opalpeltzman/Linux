Instructions to run the program -

COMPILE:

	gcc -g -finstrument-functions myFileSystemMonitor.c -o myFileSystemMonitor -lcli -lpthread

RUN:
	sudo ./myFileSystemMonitor -d /.. -i 127.0.0.1

NETCAT:

	netcat -lukp 10000
	
TELNET:
	
	telnet localhost 2468
	
	UserName:  user
	PassWord: 123
	
	backtrace
	
	exit
