#include <stdlib.h>     // standard stuff
#include <sys/mman.h>   // mmap()
#include <stdio.h>      // io stuff
#include <unistd.h>     // sleep()
#include <semaphore.h>  // semaphore()
#include <time.h>       // time()

//sem_wait : giảm giá trị đi 1
//sem_post : tăng giá trị lên 1

#define BUFFER_SIZE 10

typedef struct Buffer{
    char **Tuples;
    int inSlotIndex;
    int outSlotIndex;
} Buffer;

int main()
{
    
    int *buffer = mmap(NULL, sizeof(int)*BUFFER_SIZE, PROT_READ | PROT_WRITE, MAP_SHARED | MAP_ANONYMOUS, -1, 0);

    
    sem_t *mutex = (sem_t*)mmap(NULL, sizeof(sem_t*), PROT_READ | PROT_WRITE, MAP_SHARED | MAP_ANONYMOUS, -1, 0);
    sem_t *full = (sem_t*)mmap(NULL, sizeof(sem_t*), PROT_READ | PROT_WRITE, MAP_SHARED | MAP_ANONYMOUS, -1, 0);
    sem_t *empty = (sem_t*)mmap(NULL, sizeof(sem_t*), PROT_READ | PROT_WRITE, MAP_SHARED | MAP_ANONYMOUS, -1, 0);

    sem_init(mutex, 1, 1); //khóa mutex
    sem_init(empty, 1, BUFFER_SIZE); //--> đầy

    sem_init(full, 1, 0);//-> trống

    
    int *buffer1 = mmap(NULL, sizeof(int)*BUFFER_SIZE, PROT_READ | PROT_WRITE, MAP_SHARED | MAP_ANONYMOUS, -1, 0);

    sem_t *mutex1 = (sem_t*)mmap(NULL, sizeof(sem_t*), PROT_READ | PROT_WRITE, MAP_SHARED | MAP_ANONYMOUS, -1, 0);
    sem_t *full1 = (sem_t*)mmap(NULL, sizeof(sem_t*), PROT_READ | PROT_WRITE, MAP_SHARED | MAP_ANONYMOUS, -1, 0);
    sem_t *empty1 = (sem_t*)mmap(NULL, sizeof(sem_t*), PROT_READ | PROT_WRITE, MAP_SHARED | MAP_ANONYMOUS, -1, 0);

    sem_init(mutex1, 1, 1); 
    sem_init(empty1, 1, BUFFER_SIZE); 

    sem_init(full1, 1, 0);
    
    //hệ số 2 = 1 != 0 --> share giữa các process
    
    pid_t chef1, chef2;
    const char* name1 = "Donatello";
    const char* name2 = "Portecelli"; 

    pid_t customer1, customer2, customer3;
    //non-vegan
    const char* dish1 = "Fettuccine Chicken Alfredo";
    const char* dish2 = "Garlic Sirloin Steak";
    //vegan
    const char* dish3 = "Pistachio Pesto Pasta";
    const char* dish4 = "Avocado Fruit Salad";

    srand(time(NULL));
    const char* nonvegan() {
        int a = rand() % 2;
        if (a == 1) {
            return dish1;
        } else {
            return dish2;
        }
    }

    const char* vegan() {
        int a = rand() % 2;
        if (a == 1) {
            return dish3;
        } else {
            return dish4;
        }
    }

    const char* all() {
        int a = rand() % 2;
        if (a == 1) {
            return nonvegan();
        } else {
            return vegan();
        }
    }

    int num1,num2;
    if((chef1 = fork()) == 0)
    {
        while(1)
        {
            
            sem_wait(empty); // wait until empty > 0 and then decrement 'empty'
            sem_wait(mutex); //Yêu cầu khóa
           
			/* YOUR TASK: IMPLEMENT IN/OUT POINTERS TO ADD/TAKE FROM THE BUFFERS IN FIFO FASHION */
            printf("%s creats non-vegan dish: %s\n",name1,nonvegan());
           
            sem_post(full); //increment "full"
            sem_post(mutex); //Nhả khóa

            /* Sleep between 1 and 5 seconds */
            srand(time(NULL));
            sleep(1 + rand()% 5);
        }
    }

    /* Child chef vegan process */
    if((chef2 = fork()) == 0)
    {
        while(1)
        {
            sem_wait(empty1); // wait until empty > 0 and then decrement 'empty'
            sem_wait(mutex1); //Yêu cầu khóa
           
			/* YOUR TASK: IMPLEMENT IN/OUT POINTERS TO ADD/TAKE FROM THE BUFFERS IN FIFO FASHION */
            printf("%s creats vegan dish: %s",name2,nonvegan());
           
            sem_post(full1); //increment "full"
            sem_post(mutex1); //Nhả khóa

            /* Sleep between 1 and 5 seconds */
            srand(time(NULL));
            sleep(1 + rand()% 5);
        }
    }
            /* Child customer non-vegan process */
    if((customer1 = fork()) == 0)
    {
        while(1)
        {
            sem_wait(full); //wait until full > 0 and then decrement 'full'
            sem_wait(mutex); //yêu cầu khóa
            
			/* YOUR TASK: IMPLEMENT IN/OUT POINTERS TO ADD/TAKE FROM THE BUFFERS IN FIFO FASHION */
            printf("Consumer 1 takes %s\n",nonvegan());

            sem_post(mutex); //Nhả khóa
            sem_post(empty); // increment 'empty'

            /* Sleep between 10 and 15 seconds */
            srand(time(NULL));
            sleep(10 + rand()% 6);
        }
    }


    if((customer2 = fork()) == 0)
    {
        while(1)
        {
            sem_wait(full1); //wait until full > 0 and then decrement 'full'
            sem_wait(mutex1); //yêu cầu khóa
            
			/* YOUR TASK: IMPLEMENT IN/OUT POINTERS TO ADD/TAKE FROM THE BUFFERS IN FIFO FASHION */
            printf("Consumer 2 takes %s\n",vegan());

            sem_post(mutex1); //Nhả khóa
            sem_post(empty1); // increment 'empty'

            /* Sleep between 10 and 15 seconds */
            srand(time(NULL));
            sleep(10 + rand()% 6);
        }
    }

    if((customer3 = fork()) == 0)
    {
        while(1)
        {
            int a = rand() % 2;
            if (a == 1) {
                sem_wait(full); //wait until full > 0 and then decrement 'full'
                sem_wait(mutex); //yêu cầu khóa
            
			    /* YOUR TASK: IMPLEMENT IN/OUT POINTERS TO ADD/TAKE FROM THE BUFFERS IN FIFO FASHION */
                printf("Consumer 3 takes %s\n",nonvegan());

                sem_post(mutex); //Nhả khóa
                sem_post(empty); // increment 'empty'

                /* Sleep between 10 and 15 seconds */
                srand(time(NULL));
                sleep(10 + rand()% 6);
            } else {
                sem_wait(full1); //wait until full > 0 and then decrement 'full'
                sem_wait(mutex1); //yêu cầu khóa
            
			    /* YOUR TASK: IMPLEMENT IN/OUT POINTERS TO ADD/TAKE FROM THE BUFFERS IN FIFO FASHION */
                printf("Consumer 3 takes %s\n",vegan());

                sem_post(mutex1); //Nhả khóa
                sem_post(empty1); // increment 'empty'

                /* Sleep between 10 and 15 seconds */
                srand(time(NULL));
                sleep(10 + rand()% 6);
            }
            
        }
    }
    /* Parent */
    if (chef1 > 0 && chef2 > 0 && customer1 > 0 && customer2 > 0 && customer3 > 0)
    {
        while (1)
        {
            sleep(10);
            int takenSlots1, takenSlots2;
            sem_getvalue(full, &takenSlots1);
            sem_getvalue(full1, &takenSlots2);
            printf("\nDish in non-vegan tray: %d/%d, Dish in vegan tray: %d/%d\n\n", takenSlots1, BUFFER_SIZE, takenSlots2, BUFFER_SIZE);
        }
    }

    
}
