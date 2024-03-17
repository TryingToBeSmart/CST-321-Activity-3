#include <unistd.h>
#include <stdio.h>
#include <errno.h>
#include <pthread.h>

int end_time;
pthread_mutex_t mutex;
int counter; // Global variable for 2 threads to share and increment
pid_t pid;
pid_t otherPid;

void *counter_thread (void *arg)
{
    int status;

    // For the specified time duration, increment the counter, 
    // sleep for 1 second with mutex locked
    while (time (NULL) < end_time)
    {
        /* Enter Critical Region */
        status = pthread_mutex_lock (&mutex);
        if (status == 0) printf("counter_thread: Locking mutex to increment counter\n");
        ++counter;

        sleep(1); // sleep to hold onto mutex for a greater chance of trylock failing

        status = pthread_mutex_unlock (&mutex);
        if (status == 0) printf("counter_thread: Finished, unlock mutex\n");
        /* Exit Critical Region */
    }
    
    // Print final value of counter
    printf("Final counter: %d\n", counter);
    return NULL;
}

// Every 3 seconds, attempt to use trylock and read the counter
void *monitor_thread (void *arg)
{
    int status;
    int misses = 0; // Increment if it fails to lock the mutex

    while (time (NULL) < end_time)
    {
        // sleep(3); // sleep 3 seconds 

        /* Enter Critical Region, lock mutex*/
        status = pthread_mutex_trylock (&mutex);
        if (status != EBUSY) 
        {
            printf("monitor_thread: Locking mutex, counter = %d\n", counter);

            // Unlock mutex
            status = pthread_mutex_unlock (&mutex);

            if (status == 0) printf("monito_thread: Unlocked mutex\n");
        }
        else
        {
            misses++;
            printf("monitor_thread: Missed = %d\n", misses);
        }

    }
    printf("monitor_thread: Final Missed = %d\n", misses);
    return NULL;
}

// Entry to demo the mutex trywait API
int main(int argc, char *argv[])
{
    int status;
    pthread_t counter_thread_id;
    pthread_t monitor_thread_id;

    // initialize mutex
    pthread_mutex_init(&mutex, 0);

    // set end time in seconds
    end_time = time (NULL) + 60;

    // create counter_thread and monitor_thread
    if(pthread_create (&counter_thread_id, NULL, counter_thread, NULL))
        printf("Create counter_thread failed");
    if(pthread_create (&monitor_thread_id, NULL, monitor_thread, NULL))
        printf("Create monitor_thread failed");
    
    if(pthread_join(counter_thread_id, NULL))
        printf("Join counter_thread failed");
    if(pthread_join(monitor_thread_id, NULL))
        printf("Join monitor_thread failed");

    pthread_mutex_destroy(&mutex);
    return 0;
}