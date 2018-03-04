/*
 ----------------- COMP 310/ECSE 427 Winter 2018 -----------------
 Dimitri Gallos
 Assignment 2 skeleton
 
 -----------------------------------------------------------------
 I declare that the awesomeness below is a genuine piece of work
 and falls under the McGill code of conduct, to the best of my knowledge.
 -----------------------------------------------------------------
 */

//Please enter your name and McGill ID below
//Name: Wenjie Wei
//McGill ID: 260685967

#include <stdio.h>
#include <pthread.h>
#include <stdlib.h>
#include <unistd.h>
#include <limits.h>
#include <semaphore.h>

int BUFFER_SIZE = 100; //size of queue
int airplane_create_complete = 0; 

//declare mutex
sem_t mutex;

// A structure to represent a queue
struct Queue
{
    int front, rear, size;
    unsigned capacity;
    int *array;
};

// function to create a queue of given capacity.
// It initializes size of queue as 0
struct Queue *createQueue(unsigned capacity)
{
    struct Queue *queue = (struct Queue *)malloc(sizeof(struct Queue));
    queue->capacity = capacity;
    queue->front = queue->size = 0;
    queue->rear = capacity - 1; // This is important, see the enqueue
    queue->array = (int *)malloc(queue->capacity * sizeof(int));
    return queue;
}

// Queue is full when size becomes equal to the capacity
int isFull(struct Queue *queue)
{
    return ((queue->size) >= queue->capacity);
}

// Queue is empty when size is 0
int isEmpty(struct Queue *queue)
{
    return (queue->size == 0);
}

// Function to add an item to the queue.
// It changes rear and size
void enqueue(struct Queue *queue, int item)
{
    if (isFull(queue))
        return;
    queue->rear = (queue->rear + 1) % queue->capacity;
    queue->array[queue->rear] = item;
    queue->size = queue->size + 1;
}

// Function to remove an item from queue.
// It changes front and size
int dequeue(struct Queue *queue)
{
    if (isEmpty(queue))
        return INT_MIN;
    int item = queue->array[queue->front];
    queue->front = (queue->front + 1) % queue->capacity;
    queue->size = queue->size - 1;
    return item;
}

// Function to get front of queue
int front(struct Queue *queue)
{
    if (isEmpty(queue))
        return INT_MIN;
    return queue->array[queue->front];
}

// Function to get rear of queue
int rear(struct Queue *queue)
{
    if (isEmpty(queue))
        return INT_MIN;
    return queue->array[queue->rear];
}

void print(struct Queue *queue)
{
    if (queue->size == 0)
    {
        return;
    }

    for (int i = queue->front; i < queue->front + queue->size; i++)
    {

        printf(" Element at position %d is %d \n ", i % (queue->capacity), queue->array[i % (queue->capacity)]);
    }
}

struct Queue *queue;

/*Producer Function: Simulates an Airplane arriving and dumping 5-10 passengers to the taxi platform */
void *FnAirplane(void *cl_id)
{
    while(!airplane_create_complete){
    }

    int passengers_to_plat = (int)rand() % 6 + 5;
    printf("Airplane %d arrives with %d passengers\n", cl_id, passengers_to_plat);

    //the following few lines determines the passenger ID.
    int platform_id = 0;
    for (platform_id; platform_id < passengers_to_plat; platform_id++)
    {
        //there is a racing condition here?
        int passenger_id = 1000000;
        passenger_id += ((int)cl_id * 1000);
        passenger_id += platform_id;
        if (!isFull(queue))
        {
            sem_wait(&mutex);
            enqueue(queue, passenger_id);
            printf("Passenger %d of airplane %d arrives to platform.\n", passenger_id, cl_id);
            sem_post(&mutex);
        }
        else
        {
            printf("The platform queue is full. Rest of passengers of plane %d take the bus.\n", cl_id);
            break;
        }
    }
}

/* Consumer Function: simulates a taxi that takes n time to take a passenger home and come back to the airport */
void *FnTaxi(void *pr_id)
{
    int isWaiting = 0;

    while(1){
        if(!isWaiting)
            printf("Taxi driver %d arrives\n", pr_id);

        if(isEmpty(queue)){
            printf("Taxi driver %d waits for passengers to enter the platform.\n", pr_id);
        } else{
            sem_wait(&mutex);
            printf("Taxi driver %d picked up passenger %d from the platform\n", pr_id, dequeue(queue));
            int travelTime = rand()%167+333;  //in milli sec
            int minute = (int) (travelTime/60)*1000;
            printf("Taxi driver %d is going to travel %d minutes\n", pr_id, minute);
            sem_post(&mutex);
            isWaiting = 0;
            sleep(travelTime);
        }
    }
}

int main(int argc, char *argv[])
{
    int num_airplanes;
    int num_taxis;

    num_airplanes = atoi(argv[1]);
    num_taxis = atoi(argv[2]);

    printf("You entered: %d airplanes per hour\n", num_airplanes);
    printf("You entered: %d taxis\n", num_taxis);

    //initialize queue
    queue = createQueue(BUFFER_SIZE);

    //seed the rand
    srand(time(NULL));

    //declare arrays of threads and initialize semaphore(s)
    sem_init(&mutex, 0, 1);

    //create arrays of integer pointers to ids for taxi / airplane threads
    int *taxi_ids[num_taxis];
    int *airplane_ids[num_airplanes];

    //create threads for airplanes
    int i = 0;
    for (i; i < num_airplanes; i++)
    {
        pthread_t airplane_thread_id;
        if (pthread_create(&airplane_thread_id, NULL, FnAirplane, i))
        {
            fprintf(stderr, "Failed to create thread.\n");
            return 1;
        }
        else
        {
            printf("Creating airplane thread %d\n", i);
        }
        airplane_ids[i] = airplane_thread_id;
    }
    airplane_create_complete = 1;

    //create threads for taxis
    int j = 0;
    for (j; j < num_taxis; j++)
    {
        pthread_t taxi_thread_id;
        taxi_ids[j] = taxi_thread_id;
        if (pthread_create(&taxi_thread_id, NULL, FnTaxi, j))
        {
            fprintf(stderr, "Failed to create thread.\n");
            return 1;
        }
        else
        {
        }
    }

    pthread_exit(NULL);
}