#include <fstream>
#include <iostream>
#include <pthread.h>
#include <math.h>

#define NORMAL 0
#define REVERSE 1

using namespace std;

// global variables
const int n = 4;                            // n x n elements
const int numPhases = log2 (n*n) + 1;       // maximum number of phases is log2(N) + 1
pthread_mutex_t phaseMutex;
pthread_cond_t nextPhase;

struct thread_data {
   int thread_id;
   int *phase;
   int *count;
   int (*table)[n]; 
};

void swap(int *xp, int *yp)  
{  
    int temp = *xp;  
    *xp = *yp;  
    *yp = temp;  
}

void bubbleSortRow(int input[], int n, int direction)
{
    int i, j; 
    bool swapped; 
    for (i = 0; i < n-1; i++) 
    { 
        swapped = false; 
        for (j = 0; j < n-i-1; j++) { 
            if (direction == NORMAL) {
                if (input[j] > input[j+1]) { 
                    swap(&input[j], &input[j+1]); 
                    swapped = true; 
                }
            }
            else if (direction == REVERSE) {
                if (input[j] < input[j+1]) { 
                    swap(&input[j], &input[j+1]); 
                    swapped = true; 
                }
            }
        } 

        // If no elements swapped, break 
        if (swapped == false) 
        break; 
    } 
}

void bubbleSortCol(int input[][n], int n, int col)
{
    int i, j; 
    bool swapped;
    for (i = 0; i < n-1; i++) 
    { 
        swapped = false; 
        for (j = 0; j < n-i-1; j++) { 
            if (input[j][col] > input[j+1][col]) { 
                swap(&input[j][col], &input[j+1][col]); 
                swapped = true; 
            }
        } 

        // If no elements swapped, break  
        if (swapped == false) 
        break; 
    } 
}

void *shearSort(void *arg)
{
    struct thread_data *data;
    data = (struct thread_data *) arg;
    int tid, *phase, *count, (*table)[n];
    tid = data->thread_id;
    phase = data->phase;
    count = data->count;
    table = data->table;
    
    while (*phase < (numPhases+1)) {
        // row sort in odd-numbered phases
        if (*phase%2 == 1) {
            if ((tid+1)%2 == 1) 
                bubbleSortRow(table[tid], n, NORMAL);         // normal sort for odd-numbered rows
            else
                bubbleSortRow(table[tid], n, REVERSE);        // reverse sort for even-numbered rows
        }
        // column sort in even-numbered phases
        else {
            bubbleSortCol(table, n, tid);
        }

        pthread_mutex_lock(&phaseMutex);
        (*count)++;
        if (*count == n) {
            *count = 0;
            cout << "\n" << "Phase "<< *phase << " complete. " << endl;

            // print result
            cout << "\n";
            for (unsigned int i = 0; i < n; i++) {
                for (unsigned int j = 0; j < n; j++) {
                    cout << table[i][j] << " "; 
                }
                cout << "\n";
            }

            (*phase)++;
            pthread_cond_broadcast(&nextPhase);
        }
        else {
            pthread_cond_wait(&nextPhase, &phaseMutex);
        }

        pthread_mutex_unlock(&phaseMutex);
    }

    pthread_exit(NULL);
}

int main()
{
    int table[n][n];
    int phase = 1;
    int count = 0;
    int retCode;
    pthread_t threads[n];
    long t;
    
    // make threads explicitly joinable
    pthread_attr_t attr;
    pthread_attr_init(&attr);
    pthread_attr_setdetachstate(&attr, PTHREAD_CREATE_JOINABLE);

    // initialize mutex and condition variable
    pthread_mutex_init(&phaseMutex, NULL);
    pthread_cond_init(&nextPhase, NULL);
    struct thread_data thread_data_array[n];

    cout << "Original input:" << "\n" << endl;
    // read from text file
    ifstream infile("input");
    if (infile.is_open()) {
        for (unsigned int i = 0; i < n; i++) {
            for (unsigned int j = 0; j < n; j++) {
                infile >> table[i][j];
                cout << table[i][j] << " "; 
            }
            cout << "\n";
        }
    }
    else {
        cout << "Input file could not be opened." << endl;
        return 1;
    }
    infile.close();
    
    cout << "\n";
    
    // spawn threads for shearsort
    for (t = 0; t < n; t++) {
        //Fill struct with data
        thread_data_array[t].thread_id = t;
        thread_data_array[t].phase = &phase;
        thread_data_array[t].count = &count;
        thread_data_array[t].table = table;
        retCode = pthread_create(&threads[t], &attr, shearSort, &thread_data_array[t]);
        if (retCode) {
            cout << "Unable to create thread." << endl;
            exit(-1);
        }
    }

    pthread_attr_destroy(&attr);
    for(t=0; t < n; t++) {
       retCode = pthread_join(threads[t], NULL);
       if (retCode) {
          printf("ERROR; return code from pthread_join() is %d\n", retCode);
          exit(-1);
          }
    }

    
    cout << "\n" << "Program complete." << endl;
    pthread_mutex_destroy(&phaseMutex);
    pthread_cond_destroy(&nextPhase);
    pthread_exit(NULL);
}