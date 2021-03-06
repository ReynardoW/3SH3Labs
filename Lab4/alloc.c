#include <stdlib.h>
#include <sys/types.h>
#include <stdio.h>
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

//This function reduces the amount of memory requested by the user from the resource type
//it locates the resource type, reduces the amount the user requested it by, and then 
//syncs the updated amount to the memory map.
void reduceResources(char *memory, int size, int resourceType, int resourceAmount)
{
    int x;
    for(int i = 0; i < size; i = i + 4)
    {
        //- '0' removes the ascii value from the character. Converting it to an integer
        if(memory[i] - '0' == resourceType)
        {
            x = (memory[i+2] - '0') - resourceAmount;
            if(x < 0) 
            {
                printf("You have tried to allocate too many resources to a specific resource type. Setting resource to minimum allowable (0)");
                x = 0;
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
        sem = semget(key, 1, IPC_CREAT | S_IRUSR | S_IWUSR);
    }else
    {
        printf("Using an older semaphore\n");
        printf("sem: %d\n", sem);
    }

    //Memory Map Initialization
    int fd = open(argv[1], O_RDWR, S_IRUSR | S_IWUSR);
    struct stat sb;
    if(fstat(fd, &sb) == -1)
    {
        perror("Unable to get file size\n");
    }
    //MAP_SHARED allows other processes to see updates
    char *memoryMap = mmap(NULL, sb.st_size, PROT_READ | PROT_WRITE, MAP_SHARED, fd, 0);
    //Once done, increment semaphore so other process can access memorymap
    //After creating or grabbing an old semaphore set it to 1
    if(semctl(sem, 0, SETVAL, 1) == -1)
    {
        perror("Unable to set semaphore value\n");
        exit(1);
    }
    printf("Semaphore value = %d\n", semctl(sem, 0, GETVAL, 0));
    
    int resourceType;
    int resourceAmount;

    while(1)
    {
        //Accessing memory. Wait for other process to finish before accessing
        semop(sem, &sem_wait, 1);
        displayResources(memoryMap, (int)sb.st_size);
        semop(sem, &sem_signal, 1);

        printf("\nWhat resource type would you like to use? (Enter -1 to break)\n");
        scanf("%d", &resourceType);
        if(resourceType == -1) break;
        printf("How many of specified type is required?\n");
        scanf("%d", &resourceAmount);
        semop(sem, &sem_wait, 1);
        reduceResources(memoryMap, (int)sb.st_size, resourceType, resourceAmount);
        semop(sem, &sem_signal, 1);
    }
    //Exit program when user decides to break while loop
    //exit(0);
}

