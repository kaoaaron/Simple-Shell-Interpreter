#include <stdio.h>
#include <unistd.h>
#include <readline/readline.h>
#include <readline/history.h>
#include <string.h>
#include <stdlib.h>
#include <errno.h>
//gcc a1.c -lreadline -lhistory -o a1

struct node{ 
    pid_t pid; 
    char command[1024]; 
    struct node* next; 
}node;

char carat[4] = " > ";
int bgProcess = 0;
struct node* head = NULL;

void bglist(){
    if (bgProcess > 0){
        if(head->next == NULL){
	    printf("%s %d\n",head->command, head->pid);
        }
        else{
	    struct node* current = head;
	    printf("%s %d\n",head->command, head->pid);
	    while (current->next != NULL){
	        current = current->next;
	        printf("%s %d\n",current->command, current->pid);
    	    }
        }
    printf("Background Processes: %d\n", bgProcess);
    }else{
	printf("No processes in the background\n");
    }
}

void bgCheck(){
    if(bgProcess > 0){
	pid_t ter = waitpid(0, NULL, WNOHANG);

	while(ter > 0){
	    if(head->pid == ter){
	        printf("%d %s has terminated\n", head->pid, head->command);
		bgProcess--;
		head = head->next;
	    }
	    else{
		struct node* current = head;
	        while (current->next->pid != ter){
		    current = current->next;
	        }
		bgProcess--;
		printf("%d %s has terminated\n", current->next->pid, current->next->command);
		current->next = current -> next -> next;
	    }
	ter = waitpid(0, NULL, WNOHANG);
    	}
    }
    
}

void background(char *tok[],int count){
    int i;
    char **newTok = malloc(sizeof(char*)*5);

    for(i = 0; i < count -1; i++){
	newTok[i] = malloc(sizeof(char)*(strlen(tok[i+1])+5));
   	strcpy(newTok[i], tok[i+1]); 
    }

    newTok[count-1] = NULL;

    pid_t parent = getpid();
    pid_t pid = fork();
        
    if(pid == -1){
        printf("Fork Failed\n");
    }else if(pid > 0){
//	printf("dad PID: %d\n", getpid());
//	printf("child PID: %d\n", pid);
        if(bgProcess == 0){
	    bgProcess++;
	    head = malloc(sizeof(node)*3);
	    head->pid = pid;
	    head->next = NULL;
	    strcat(head->command, newTok[0]);
	    for(i = 1; i < count-1; i++){
		strcat(head->command, " ");
		strcat(head->command, newTok[i]);
	    }
	    //strcpy(head->command,newTok[1]);
	}
	else{
	    bgProcess++;
	    
	    struct node* current = head;
	    while (current->next != NULL){
		current = current->next;
	    }
	    current->next = malloc(sizeof(node));
	    current->next->pid = pid;
	    strcat(current->next->command, newTok[0]);
	    for(i = 1; i < count-1; i++){
		strcat(current->next->command, " ");
		strcat(current->next->command, newTok[i]);
	    }
	   // strcpy(current->next->command,newTok[1]);
	    current->next->next = NULL;

	}
	waitpid(pid, NULL, WNOHANG);
    }

    else{
        if(execvp(newTok[0],newTok)==-1){
	    //printf("count: %d tok %s", bgProcess,newTok[1]);
            printf("command not found\n");
	    exit(1);
        }
    }

}

void basicExec(char *tok[], int count){
    pid_t parent = getpid();
    pid_t pid = fork();
        
    if(pid == -1){
        printf("Fork Failed");
    }else if(pid > 0){
        wait();
    }
    else{
        if(execvp(tok[0],tok)==-1){
            printf("command not found\n");
	    exit(1);
        }
    }
}

void changeDir(char *tok[],int count){


    if(count  == 1){
        chdir(getenv("HOME"));
    }
    else if(count  == 2){
        if(strcmp(tok[1],"~")==0){
            chdir(getenv("HOME"));
        }
        else if(strcmp(tok[1],"..")==0){
            chdir("..");
        }
        else if(chdir(tok[1])==-1){
            printf("%s\n",strerror(errno));
        }
        else{ 
            chdir(tok[1]);
        }
    }
    else{
        printf("Too many input arguments\n");
    }
}
  
//chdir("/home/kaoaaron/CSC 360");*/


void parse(char *userinput ){
   char *token = strtok(userinput, " ") ;
   int count = 0;
   int i;
   char **tok = malloc(sizeof(char*));
   
   while(token != NULL){
        tok[count] = malloc(sizeof(char)*(strlen(token)+1));
        strcpy(tok[count], token);
        token = strtok(NULL, " ");
	count++;

        if(token != NULL){
            char **newTok = realloc(tok, sizeof(char*)*(count+1));

            if(newTok != NULL){
                tok = newTok;
            }
        }
   }

   tok[count] = NULL;

   if(tok[0] != NULL){
	   if(!strcmp(tok[0], "cd\0")){
	       changeDir(tok,count);
	   }else if(!strcmp(tok[0], "bg\0")){
	       background(tok, count);
	   }else if(!strcmp(tok[0], "bglist\0")){
	       bglist();
	   }else{
	       basicExec(tok,count);
	   }
   }
   
   //for(i = 1; i < count; i++){
//	free(tok[i]);
   //}
   //free(tok);
}

void init(){
   char *cwd;
   char cwdval[1024];
   char ssi[1024] = "SSI: ";
   getcwd(cwdval, sizeof(cwdval));
   strcat(cwdval, carat);
   strcat(ssi, cwdval);
   cwd = readline(ssi);
   if (strlen(ssi) > 0){
       add_history(cwd);
   }
   parse(cwd);
   //free(cwd);
}

int main(){
    while(1){
        init();
	bgCheck();
    }
    return 0;
}

