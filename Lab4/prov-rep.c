#include <stdio.h>
#include <stdlib.h>
#include <signal.h>
#include <unistd.h>
#include <sys/types.h>
#include <sys/sem.h>
#include <sys/ipc.h>
#include <ctype.h>
#include <fcntl.h>
#include <sys/stat.h>
#include <sys/mman.h>

int sem;
key_t key;

//Semaphore allocation
struct sembuf sem_signal = {0, 1, 0}; //Increases sem by 1
struct sembuf sem_wait = {0, -1, 0}; //Decreases sem by 1

//This function adds the amount of memory requested by the user from the resource type
//it locates the resource type, reduces the amount the user requested it by, and then 
//syncs the updated amount to the memory map.
void addResources(char *memory, int size, int resourceType, int resourceAmount)
{
    int x;
    for(int i = 0; i < size; i = i + 4)
    {
        //- '0' removes the ascii value from the character. Converting it to an integer
        if(memory[i] - '0' == resourceType)
        {
            x = (memory[i+2] - '0') + resourceAmount;
            if(x > 10) 
            {
                printf("You have tried to exceed the maximum amount of resources. Setting amount to the maximum allowable amount (10)");
                x = 10;
            }
            memory[i+2] = x + '0';
        }
    }
    msync((void *)memory, (long)size, MS_SYNC);
}

//This function runs through the memory map and prints out each element onto the console
void displayResources(char *memory, int size)
{
    for(int i = 0; i < size; i++)
    {
        printf("%c", memory[i]);
    }
}

int main(int argc, char *argv[])
{   
        //SEMAPHORE INITIALIZATION
    //Initializing a new semaphore or getting a pre-existing semaphore
    key = ftok("./alloc.c", 1);
    sem = semget(key, 1, 0);
    if(sem == -1)
    {
        printf("Creating a new semaphore\n");
        sem = semget(key, 1, IPC_CREAT);
    }else
    {
        printf("Using an older semaphore\n");
        printf("sem: %d\n", sem);
    }
    //Wait before accessing file
    semop(sem, &sem_wait, 1);

    //Memory Map Initialization
    int fd = open(argv[1], O_RDWR, S_IRUSR | S_IWUSR);
    struct stat sb;
    if(fstat(fd, &sb) == -1)
    {
        perror("Unable to get file size\n");
    }
    char *memoryMap = mmap(NULL, sb.st_size, PROT_READ | PROT_WRITE, MAP_SHARED, fd, 0);
    //Once memory map has been initialized release semaphore for other processes
    semop(sem, &sem_signal, 1);
    
    int pid = fork();

    
    if(pid == 0)
    {//Child Process
        while(1)
        {
            printf("Page size: %d\n", getpagesize());
            semop(sem, &sem_wait, 1);
            displayResources(memoryMap, (int)sb.st_size);
            //Mincores
            char *ptr = malloc(sizeof(char));
            printf("mincore returned: %d\n", mincore((void *)memoryMap, sb.st_size, ptr));
            printf("mincore: %d\n", *ptr & 1);
            semop(sem, &sem_signal, 1);
            sleep(10);
        }
    }else
    {//Parent Process
        int resourceType;
        int resourceAmount;
        while(1)
        {
            printf("\nWhich type of resource would you like to add to? (Enter -1 to exit)\n");
            scanf("%d", &resourceType);
            if(resourceType == -1) 
            {
                kill(pid, SIGKILL);
                break;
            }
            printf("How many would you like to add to specified resource?\n");
            scanf("%d", &resourceAmount);

            //Add specified amount to memorymap
            semop(sem, &sem_wait, 1);
            addResources(memoryMap, (int)sb.st_size, resourceType, resourceAmount);
            semop(sem, &sem_signal, 1);
        }
    }
}