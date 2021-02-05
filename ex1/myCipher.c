#include <stdio.h>
#include <stdlib.h>
#include <getopt.h>
#include <unistd.h>
#include <stdbool.h>
#define SIZE 36
#define MAX 200

int main(int argc, char *argv[])
{

    // ----------------------------
    int opt;
    FILE *keys;
    FILE *newkeys;
    FILE *decryptKeys;
    FILE* message;
    FILE* decMessage;
    FILE* encMessage;
    extern char *optarg;
    char userMessage[MAX];
    char UserK;
    char UserEnK;
    int i = 0;
    char str[5]; 
    char key[SIZE];
    char encKey[SIZE];
    bool encFlag = false;
    bool decFlag = false;

    while ((opt = getopt(argc, argv, "e:i:d:")) != -1)
    {
        switch (opt)
        {   
            // encrypt key
            case 'e': 
                keys = fopen(optarg, "r");

                if (keys == NULL)
                {
                    printf("There is an error with the encryption key file. Please try again.\n");
                    return 0;
                }
                else
                {
                    encFlag = true;
                    while (fgets(str, 5, keys) != NULL)
                    {
                        key[i] = str[0];
                        encKey[i] = str[2];
                        
                        i++;
                    }
                   
                    fclose(keys);

                    newkeys = fopen("new_keys.txt", "w");
                    printf("Please enter 3 keys you would like to change :\n");
                    for (int in = 1; in < 4; in++)
                    {
                        fflush(stdin);
							printf("which key would you like to change? [insert key]\n");
							scanf(" %c",&UserK);
							printf("Which value you to change? [insert value]\n");
							scanf(" %c", &UserEnK);

                        for (int j = 0; j < SIZE; j++)
                        {
                            if (key[j] == UserK)
                            {
                                encKey[j] = UserEnK;
                                break;
                            }
                        }
                    }
                    for (int j = 0; j < SIZE; j++)
                        fprintf(newkeys,"%c %c\n", key[j], encKey[j]);
		     
                    fclose(newkeys);
                }
                break;

            // decryption file
            case 'd': 
                decryptKeys = fopen(optarg, "r");
                if (decryptKeys == NULL)
                {
                    printf("There is an error opening decryption file\n");
                    return 0;
                }
                else
                {
                    decFlag = true;
                     while (fgets(str, 5, decryptKeys) != NULL)
                    {
                        key[i] = str[2];
                        encKey[i] = str[0];
                        i++;
                    }
                    key[i] = '\0';
                    encKey[i] = '\0';
                    fclose(decryptKeys);
                }
                break;
            
            // message file
            case 'i':
                message = fopen(optarg, "r");
                if (message == NULL)
                {
                    printf("There is an error opening user message file\n");
                    return 0;
                }
                else{
                    fgets(userMessage, 200, message);
                    fclose(message);
                }
                break;
            default:
                printf("Error with files, please check the files:%s, %s , you send\n", argv[2], argv[4]);
                exit(EXIT_FAILURE);
        } // switch

    }

    //Eecrypting the file
    if(encFlag){
		encMessage = fopen("encryption_file.txt","w");
		encFlag = false;
		for(int i = 0; i < MAX; i++)
	{
			for(int j = 0; j < SIZE; j++)
			{
				if(key[j] == userMessage[i])
				{
					userMessage[i] = encKey[j];
					break;
				}
				
				
			}
			fprintf(encMessage,"%c", userMessage[i]);

	}
	fclose(encMessage);
		}
	else if(decFlag){ 
		decMessage = fopen("decrypted_text.txt", "w");
		decFlag = false;
		for(int i = 0; i < MAX; i++)
	{
			for(int j = 0; j < SIZE; j++)
			{
				if(key[j] == userMessage[i])
				{
					userMessage[i] = encKey[j];
					break;
				}
				
				
			}
			fprintf(decMessage,"%c", userMessage[i]);

	}
	fclose(decMessage);	
	}
		
		
    

    return 1;
}
