#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include<unistd.h>

void PSH_READ();
char **PSH_TOKENIZER(char *line);
int main(int argc,char **argv) 
{
    printf("Welcome to psh!\n");
    printf("%i\n",argc);
    PSH_READ();
    return 0;
}
void PSH_READ()
{
    size_t n=0;
    char *delim=" ";
    int quit=1;
    while(quit)
    {    
        char *inputline;
        printf("PSH $ ");
        if(getline(&inputline,&n,stdin)==-1)
        {
            exit(1);
        }
        if(strcmp(inputline,"exit\n")==0)
        {
            exit(0);
        }
        printf("%s\n",inputline);
        PSH_TOKENIZER(inputline);

    }
}
char **PSH_TOKENIZER(char *line)
{
    int i=0;
//     sudo -S -y
//     ["sudo       S","-y"]
    char **token_arr=malloc(1024*sizeof(char *));
    char *token;
    token=strtok(line," ");
    while(token!=NULL)
    {
        token_arr[i]=token;
        token=strtok(NULL," ");
        i++;
    }
    i=0;
    while(token_arr[i]!=NULL)
    {
        printf("%s ",token_arr[i]);
        i++;
    }
    return token_arr;
}
