#include<iostream>
#include<chrono>
#include<cstdlib>
#include<pthread.h>
#include<ctime>
#include "../skiplist.h"

using namespace std;

#define NUM_THREADS 1
#define TEST_COUNT 200000

SkipList<int, string> skiplist(18);

void* insert_element(void* threadid){
    long tid;
    tid = (long)threadid;
    cout << tid << endl;
    int temp = TEST_COUNT / NUM_THREADS;
    for(int i = temp*tid, count = 0;count<temp;i++){
        count++;
        skiplist.insert_element(rand()%TEST_COUNT, "a");
    }
    pthread_exit(NULL);
}

void* getElement(void* threadid){
    long tid;
    tid = (long)threadid;
    cout << tid << endl;
    int temp = TEST_COUNT / NUM_THREADS;
    for(int i = temp*tid, count=0;count<temp;i++){
        count++;
        skiplist.search_element(rand()%TEST_COUNT);
    }
    pthread_exit(NULL);
}


int main(){
    srand((unsigned int)time(NULL));
    {
        pthread_t threads[NUM_THREADS];
        int rc;
        int i;
        auto start = chrono::high_resolution_clock::now();

        for(int i = 0;i<NUM_THREADS;i++){
            cout << "main(): create thread, " << endl;
            rc = pthread_create(&threads[i], NULL, getElement, (void*)i);
            if(rc!=0){
                cout << "ERROR: unable to create thread, " << rc << endl;
                exit(-1);
            }
        }

        void* ret;
        for(int i = 0;i<NUM_THREADS;i++){
            if(pthread_join(threads[i], &ret) != 0){
                perror("pthread_create() error");
                exit(3);
            }
        }

        auto finish = chrono::high_resolution_clock::now();
        chrono::duration<double> elapsed  = finish - start;
        cout << "insert elapsed: " << elapsed.count() << endl;
    }
    pthread_exit(NULL);
    return 0;
}