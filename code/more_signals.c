#include <unistd.h>
#include <stdio.h>
#include <stdlib.h>
#include <sys/types.h>
#include <errno.h>
#include <pthread.h>
#include <fcntl.h>
#include <semaphore.h>
#include <sys/mman.h>
#include <signal.h>
#include <stdbool.h>
#include <string.h>
#include <asm/signal.h>

int WAKEUP_SIGNAL = SIGUSR1;
int END_SIGNAL = SIGUSR2; // for producer to tell consumer to end
sigset_t sigSet;

pid_t otherPid;

bool proceed = true; // changes to fales when producer is finished

// Shared Circular Buffer. Each element holds an int 
struct CIRCULAR_BUFFER
{
    int count; // Number of items in the buffer
    int lower; // Next slot to read in the buffer
    int upper; // Next slot to write in the buffer
    char buffer[100]; 
};
struct CIRCULAR_BUFFER *buffer = NULL;

// Sleep function
void sleepUntilWoken()
{
    int nSig;

    //put to sleep until notified to wake up
    printf("sleeping...\n");
    sigwait(&sigSet, &nSig);
    printf("Awoken\n");
}

// Signal handler for end signal
void endSignalHandler(int signal)
{
    proceed = false;
    printf("signalHandler proceed = %s\n", proceed ? "true" : "false");
}

// Responsible for creating numbers and passing them to 
// the Consumer via a shared circular buffer
void producer(int length, char *message[])
{
    for(int i = 1; i < length; i++)
    {
        // Go to sleep if the buffer is full
        if (buffer->count == sizeof(buffer->buffer)) 
        {
            printf("Buffer full, Producer going to sleep...\n");
            sleepUntilWoken();
        }


        char *arg = message[i];
        while(*arg != '\0')
        {
            putValue(*arg);
            printf("Produced: %c\n", *arg);
            arg++;
        }

        // Signal the consumer
        printf("Waking Consumer...\n");
        kill(otherPid, WAKEUP_SIGNAL); 

        sleep(1); // Simulate processing time
    }

    printf("Exiting Producer process...\n");
    proceed = false;
    printf("producer proceed = %s\n", proceed ? "true" : "false");
    kill(otherPid, END_SIGNAL); // send the end signal to consumer

    // waking the consumer 1 last time and should end program
    printf("Waking Consumer...\n");
    kill(otherPid, WAKEUP_SIGNAL); 
    _exit(1);
}

// Consumer will always be behind the producer
// and will not have to wait for the producer
void consumer()
{  
    // Set up a Signal set
    sigemptyset(&sigSet);
    sigaddset(&sigSet, WAKEUP_SIGNAL);
    sigprocmask(SIG_BLOCK, &sigSet, NULL);


    printf("Running Consumer process...\n");

    while(proceed)
    {    
        // Go to sleep if there is no data in buffer
        if (buffer->count == 0) 
        {
            printf("Buffer empty, consumer going to sleep...\n");
            sleepUntilWoken();
        }
        
        signal(END_SIGNAL, endSignalHandler); // register signal handler
        printf("consumer proceed = %s\n", proceed ? "true" : "false");

        if(buffer->count == 0 && !proceed) break; // end if everything is finished

        // Get the next int element in the buffer
        char data = getValue();
        printf("Consumed: %c\n", data);


        if(buffer->count == sizeof(buffer->buffer))
        {    
            // Signal the producer
            printf("Waking Producer...\n");
            kill(otherPid, WAKEUP_SIGNAL);
        }
    }

    printf("Exiting Consumer process...\n");
    _exit(1);
}


// putValue function to write to the buffer
void putValue(char character) 
{
    if(buffer->count < sizeof(buffer->buffer))
    {
        buffer->buffer[buffer->upper] = character; // assign the character to the buffer's next upper array location
        ++buffer->upper; // increment the upper(write) location slot and wrape around if needed
        if(buffer->upper >= 100) buffer->upper = 0; // reset buffer upper position to the beginning
        ++buffer->count; // increment the buffer count
    }
    else printf("Buffer is full, did not add: %c\n", character);
}

// getValue function to read from the buffer
int getValue()
{
    if(buffer->count > 0)
    {
        // return the number that is in the buffer's lower slot location
        char returnCharacter = buffer->buffer[buffer->lower]; 
        printf("Returning: %c from location: %d\n", returnCharacter, buffer->lower);
        ++buffer->lower; // increment the lower(read) location and wrap around if needed
        if(buffer->lower >= 100) buffer->lower = 0; // reset buffer lower position to the beginning 
        --buffer->count; // decrement the buffer count
        return returnCharacter;
    }
    else printf("Buffer is empty, nothing to get\n");
    return -1;
}


// Takes an argument to run the program, creates 2 threads that run simultaniously.
// 1 thread (producer) adds the chars to the buffer and the other thread (consumer)
// consumes them from the buffer and prints them to the console
int main(int argc, char* argv[])
{
    pid_t pid;

    // Create shared memory for the Circular Buffer to be shared between
    // the Parent and Child Processes
    buffer = (struct CIRCULAR_BUFFER*)mmap(0, sizeof(struct CIRCULAR_BUFFER), 
        PROT_READ|PROT_WRITE,MAP_SHARED|MAP_ANONYMOUS, -1, 0);
    buffer->count = 0;
    buffer->lower = 0;
    buffer->upper = 0;

    // Use fork()
    pid = fork();
    if (pid == -1)
    {
        // Error: if fork() returns -1, then error
        printf("Error in fork(), error %d\n", errno);
        exit(EXIT_FAILURE);
    }

    // OK if fork() returns 0
    if (pid == 0)
    {
        // Run Producer process logic as a child process
        otherPid = getppid();

        // send length and array of arguments to the producer
        producer(argc, argv);
    }
    else
    {
        // Run Consumer process logic at Parent process
        // result pointer points to array that consumer returns
        otherPid = pid;
        consumer();
    }
    
    pthread_exit(NULL);
    return 0;
}