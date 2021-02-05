#include <stdio.h>
#include <sys/types.h>
#include <unistd.h>
#include <sys/wait.h>
#include <stdlib.h>

//This program runs the bash commands that were in myScript.sh File

int main()
{
	pid_t pid;
	int status;
	char * encrypt[] = {"myCipher", "-e","keys","-i","some_text", NULL};
	char * decrypt[] = {"myCipher", "-d","new_keys.txt","-i","encryption_file.txt", NULL};
	char * comp[] = {"cmp", "some_text", "decrypted_text.txt", NULL};
	char * rm[] = {"rm","-f" ,"decrypted_text.txt","new_keys.txt", NULL};
	
	//first fork - running the first mycipher program to encrypt the password with new_key file (-e) and the  password file with (-i)
	pid = fork();
	if(pid == -1)
	{
		printf("Error Forking the program.\n");
	}

	else if(pid == 0)
	{
      		execvp("./myCipher", encrypt);
		exit(0);
	}

	else
	{
		if (waitpid(pid, &status, 0) > 0)
		{
			if(!(WIFEXITED(status) && WEXITSTATUS(status)))
				printf("Error encrypt\n");
		}
	}

	//second fork - running the decrypt password program with new_key file (-d) and the ecrypted password file with (-i)
	pid = fork();
	if(pid == -1)
	{
		printf("Error Forking the program.\n");
	}

	else if(pid == 0)
	{
      		execvp("./myCipher", decrypt);
		exit(0);
	}

	else
	{
		if (waitpid(pid, &status, 0) > 0)
		{
			if(!(WIFEXITED(status) && WEXITSTATUS(status)))
				printf("Error decipher\n");
		}
	}

	
	//third fork - running the cmp program to compare the password and the decrypted password 
	pid = fork();
	if(pid == -1)
	{
		printf("Error Forking the program.\n");
	}

	else if(pid == 0)
	{
      		execvp("cmp", comp);
		exit(0);
	}

	else
	{
		if (waitpid(pid, &status, 0) > 0)
		{
			if(!(WIFEXITED(status) && !WEXITSTATUS(status)))
				printf("Error compare passwords\n");
			else
				printf("Success");
		}
	}

	
	
	return 0;
}

