#include <stdio.h>
#include <signal.h>
#include <unistd.h>
#include <stdlib.h>

void alarm_handler(int signo){
  if (signo == SIGALRM){
    printf("ALARM\n");
  }else if(signo == SIGINT){
    printf("CTRL+C PRESSED!\n");
  }else if(signo ==SIGTSTP){
    printf("CTRL+Z PRESSED!\n");
    exit(1);
  }
}

int main(void){
  if (signal(SIGALRM, alarm_handler) == SIG_ERR){
    printf("failed to register alarm handler.");
    exit(1);
  }else if(signal(SIGINT, alarm_handler) == SIG_ERR){
    printf("failed to register alarm handler.");
  }else if(signal(SIGTSTP, alarm_handler) == SIG_ERR){
    printf("failed to register alarm handler.");
  }
  while(1){
    sleep(2);
    raise(SIGALRM);
  }
}
