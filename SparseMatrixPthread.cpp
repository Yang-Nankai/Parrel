#include<stdlib.h>
#include<stdio.h>
#include<pthread.h>
#include<Windows.h>


struct threadParm_t
{
    int threadId;
    int n;
};

int const thread_count=4;

int flag =0;
double factor,sum=0;
int num = 9192;


void* pi_busywaiting(void* parm)
{

    threadParm_t* p = (threadParm_t*)parm;
    int r = p->threadId;   int n = p->n;   int my_n = n / thread_count;
    printf("I am the thread %ld.\n", r);
    int my_first = my_n * r;   int my_last = my_first + my_n;
    double my_sum = 0.0;
    if (my_first % 2 == 0) factor = 1.0;
    else factor = -1.0;
    //printf("my first is:%d,my last is:%d\n", my_first, my_last);
    for (int i = my_first; i < my_last; i++, factor = -factor) {
        my_sum += factor / (2 * i + 1);
    }

    while (flag != r) Sleep(0);
    sum += my_sum;
    flag++;
    pthread_exit(nullptr);
    return nullptr;
}

int main() {

    long long head, tail, freq;

    long thread;
    threadParm_t* thread_Parm[thread_count];

    for (thread = 0; thread < thread_count; thread++) {
        thread_Parm[thread] = static_cast<threadParm_t*> (malloc(thread_count * sizeof(thread_Parm)));
        thread_Parm[thread]->n = num;
        thread_Parm[thread]->threadId = thread;
    }
    //thread_Parm = static_cast<threadParm_t*> (malloc(thread_count * sizeof(thread_Parm)));
    pthread_t* thread_handles = static_cast<pthread_t*> (malloc(thread_count * sizeof(pthread_t)));

    QueryPerformanceFrequency((LARGE_INTEGER*)&freq);

    QueryPerformanceCounter((LARGE_INTEGER*)&head);
    for (thread = 0; thread < thread_count; thread++) {
        pthread_create(&thread_handles[thread], NULL, pi_busywaiting, (void*)thread_Parm[thread]);
    }
    for (thread = 0; thread < thread_count; thread++) {
        pthread_join(thread_handles[thread], NULL);
    }
    printf("the final sum is: %f\n", 4 * sum);
    QueryPerformanceCounter((LARGE_INTEGER*)&tail);
    printf("the time is %f ms.\n",(float)((tail - head) * 1000 / freq));
    free(thread_handles);
    for (thread = 0; thread < thread_count; thread++)
        free(thread_Parm[thread]);
    return 0;


}
