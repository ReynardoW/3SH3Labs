#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <unistd.h>
#include <sys/types.h>


int main(){
  int fdChild[2], fdParent[2], nbytes;
  pid_t childpid;  
  pipe(fdParent);
  pipe(fdChild);
  childpid = fork();
  int sum = 0;
  int input;
  if(childpid < 0){
    perror("fork failed");
    exit(0);
  }
  while(1){
    if (childpid == 0){
      // Child process
      int input;
      printf("Input a 1-byte integer\n");
      scanf("%d", &input);
      if(input == -1){
        // Send -1 to parent so it knows to send sum to child
        write(fdParent[1], &input, sizeof(input));
        close(fdChild[1]);        
        // Write parent (sum) into child process and print it
        read(fdChild[0], &input, sizeof(input)); 
        printf("sum = %d\n", input);
        exit(0);
      }else {
        //Write to parent
        close(fdParent[0]);   
        close(fdChild[1]);
        write(fdParent[1], &input, sizeof(input));
      }
    }
    if(childpid > 0){
      // Parent process
      // Read from child
      int childInput;
      close(fdParent[1]);
      read(fdParent[0], &childInput, sizeof(childInput));
      printf("Parent received: %d\n", childInput);
      if(childInput == -1){ // Read then write to child
        close(fdParent[0]);
        write(fdChild[1], &sum, sizeof(sum));
        // Wait for child to exit then exit parent
        sleep(2);
        exit(0);
      }else{
        sum += childInput;
        printf("new sum = %d\n", sum);
      }
    }
  }  
    return 0;
}
