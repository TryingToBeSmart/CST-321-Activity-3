# CST-321-Activity-3

This code implements a producer-consumer scenario using shared memory and signals for synchronization.

Signals:
- Two custom signals are defined: WAKEUP_SIGNAL (SIGUSR1) and END_SIGNAL (SIGUSR2).
- Signal handling functions are implemented for these signals: endSignalHandler for END_SIGNAL.

Circular Buffer:
- A shared circular buffer structure CIRCULAR_BUFFER is defined.
- It contains fields for count, lower index, upper index, and a buffer array to hold data.
- Functions putValue and getValue manipulate the buffer for adding and retrieving data respectively.
- The buffer is shared between the producer and consumer processes using shared memory (mmap).

Producer Function (producer):
- Takes command-line arguments as input.
- Produces characters from the input arguments and adds them to the circular buffer.
- Signals the consumer to wake up after adding data to the buffer.
- Terminates when all input arguments are processed and sends an END_SIGNAL to the consumer.

Consumer Function (consumer):
- Sets up signal handling for WAKEUP_SIGNAL.
- Consumes characters from the circular buffer and prints them.
- Signals the producer to wake up if the buffer is full.
- Terminates when signaled by the producer (proceed becomes false).

Main Function:
- Creates shared memory for the circular buffer.
- Forks a child process for the producer and runs the consumer logic in the parent process.
- Passes command-line arguments to the producer.

This setup ensures synchronization between the producer and consumer processes using signals and a shared circular buffer for inter-process communication.

Video:
[![More Signals program video](./screenshots/more_signals_video_screenshot.png)](https://www.loom.com/share/8609dc1b5eb849b5a5a3b159204d29d9)
