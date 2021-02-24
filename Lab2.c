#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <unistd.h>
#include <sys/types.h>

int main(){
  int fd[2], nbytes;
  pid_t childpid;  
  pipe(fd);
  childpid = fork();
  int sum = 0;
  int input;
  if(childpid < 0){
    perror("fork failed");
    exit(0);
  }
  if (childpid == 0){
    // Child process
    int input;
    printf("Input a 1-byte integer\n");
    scanf("%d", &input);
    if(input == -1){
      // Send -1 to parent so it knows to send sum to child
      write(fd[1], &input, sizeof(input));
      open(fd[0]);
      close(fd[1]);        
      // Write parent (sum) into child process and print it
      read(fd[0], &input, sizeof(input)); 
      printf("sum = %d\n", input);
      close(fd[0]);
      exit(0);
    }else {
      //Write to parent
      open(fd[1]);
      close(fd[0]);
      write(fd[1], &input, sizeof(input));
    }
  }
  if(childpid > 0){
    // Parent process
    // Read from child
    open(fd[0]);
    close(fd[1]);
    int childInput;
    read(fd[0], &childInput, sizeof(childInput));
    if(childInput != -1){ // Read then write to child
      open(fd[1]);
      close(fd[0]);
      write(fd[1], &sum, sizeof(sum));
      close(fd[1]);
      // Wait for child to exit then exit parent
    }else{
      sum += childInput;
    }
  }
    
  return 0;
}
