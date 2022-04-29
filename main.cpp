//�˴�����ϡ������ƽ���㷨
#include<iostream>
#include<windows.h>
#include<stdlib.h>
#include<stack>
#include<cstring>
#include<iomanip>
#include<pthread.h>
#include<stdio.h>
#include<immintrin.h>
#include<algorithm>
using namespace std;

typedef float mat_type;   //��������������

#define NUM_THREADS 4   //�����߳���

int const SIZE_M = 2048;   //���þ���Ĵ�С +1Ϊ�˷���
int const RANDOM = 20;
int const Width = 4;   //������ʾλ��
int const DATA = 8000000;

long long head, freq;   //timers

void printMatrix(mat_type m[][SIZE_M]);  //��������

typedef struct {
	mat_type *data;   //����Ԫ��Ԫ��
	int *rpos;   //���е�һ������Ԫ��λ�ñ�
	int mu, nu, tu;  //����������������ͷ���Ԫ�ĸ���
	int *index;   //Ԫ�����ڵ�����
}RLSMatrix;

//�����������󣨶�ά���飩
mat_type **A,**B,**C,**D;
RLSMatrix M, N, Q; //M, N, QΪΪ�����CSRѹ����ʽ

typedef struct{
    int threadId;
}threadParm_t;    //�ṹ����

pthread_barrier_t barrier;   //barrierͬ��
pthread_mutex_t mutex=PTHREAD_MUTEX_INITIALIZER;

//�̺߳���
void *threadFunc(void *parm){

    int arow = 0, brow = 0, tp = 0;
	int ccol = 0;
	int p = 0, t = 0, q = 0;

    threadParm_t* threadp=(threadParm_t*) parm;
    int threadId=threadp->threadId;
    fprintf(stdout,"Thread %d has entered.\n",threadp->threadId);
    pthread_barrier_wait(&barrier);
    //fprintf(stdout,"Thread %d next.\n",threadp->threadId);

    long long tail;
    int rpos[SIZE_M];//�ݴ�
    int my_first = (threadId)*(SIZE_M/NUM_THREADS);
    int my_last = (threadId+1)*(SIZE_M/NUM_THREADS);  //����ÿ���߳���Ҫ�㵽�Ĵ�С
    for (arow = my_first; arow < my_last; arow++) {
			memset(rpos, 0, sizeof(rpos));
			pthread_mutex_lock(&mutex);
			Q.rpos[arow] = Q.tu + 1;
			//printf("lock q.rpos %d \n",threadp->threadId);
			pthread_mutex_unlock(&mutex);

			if (arow < M.mu - 1)
				tp = M.rpos[arow + 1];
			else
				tp = M.tu + 1;
			for (p = M.rpos[arow]-1; p < tp-1; p++) {
				brow = M.index[p];
				if (brow < N.mu - 1)
					t = N.rpos[brow + 1];
				else
					t = N.tu + 1;
				for ( q= N.rpos[brow]-1; q < t-1; q++){
					ccol = N.index[q];
					rpos[ccol] += M.data[p] * N.data[q];
				}
			}

			pthread_mutex_lock(&mutex);
			for (ccol = 0; ccol < Q.nu; ccol++) {
				if (rpos[ccol]) {
					Q.index[Q.tu] = ccol;
					Q.data[Q.tu] = rpos[ccol];
					Q.tu++;
				}
			}
			pthread_mutex_unlock(&mutex);
		}
		QueryPerformanceCounter((LARGE_INTEGER*)&tail);
		fprintf(stdout,"Thread %d exit! and cost %lf ms.\n;",threadp->threadId,(tail-head)*1000.0/freq);
		pthread_exit(nullptr);
		return nullptr;
}


//��̬�����ά���飬�����ڴ��ͷ�
mat_type** mallocMatrix(mat_type **p){
    p = (mat_type**)malloc(sizeof(mat_type*)*SIZE_M);
    for(int i = 0; i < SIZE_M;i++){
        *(p+i)=(mat_type*)malloc(sizeof(mat_type)*SIZE_M);
    }
    return p;
}

void freeMatrix(mat_type **p){
    for(int i=0;i<SIZE_M;i++){
        free(*(p+i));
    }
    free(p);
}

void mallocRLSMatrix(RLSMatrix& re){
    re.data = (mat_type*)malloc(sizeof(mat_type)*(DATA+1));
    re.index= (int*)malloc(sizeof(int)*(DATA+1));
    re.rpos=(int*)malloc(sizeof(mat_type)*(SIZE_M+2));
}

void freeRLSMatrix(RLSMatrix& re){
    free(re.data);
    free(re.index);
    free(re.rpos);
}

//ͨ������������о���ĳ�ʼ��(Ϊ��ʵ��ϡ�軯�����ｫ��������Ϊ90%�Ŀ��࣬��ֵΪ1)
void InitialMatrix(mat_type **m) {
	mat_type a;
	int r;
	for (int i = 0; i < SIZE_M; i++){
		for (int j = 0; j < SIZE_M; j++){
			r = rand() % RANDOM + 1;
			if (r == 1) {
				a = rand() % 2 + 1;
				m[i][j] = a;
			}
			else
                m[i][j]=0;
		}
	}
}

//��ϡ��������CSRѹ��
void CSRMartix(mat_type ** m,RLSMatrix& re) {
	int num = 1;   //��¼ÿ�е�һ������Ԫ�ص�λ��
	for (int i = 0; i < SIZE_M; i++){
		re.rpos[i] = num;
		for (int j = 0; j < SIZE_M; j++){
			if (m[i][j] != 0) {
				re.index[num-1] = j;
				re.data[num-1] = m[i][j];
				num++;
			}
		}
	}
	re.mu = SIZE_M;
	re.nu = SIZE_M;
	re.rpos[SIZE_M] = num;
	re.tu = num-1;
}
//��CSR��ʽת��Ϊ�����ʽ
void MatrixCSR(mat_type ** m, RLSMatrix re) {
	int num = 0;
	for (int i = 0; i < SIZE_M; i++){
		for (int j = 0; j < SIZE_M; j++){
			if (re.index[num] == j) {
				m[i][j] = re.data[num];
				num++;
			}
			else
				m[i][j] = 0;
		}
	}
	//��ӡ����
	//printMatrix(m);
}

//��ӡ����
void printMatrix(mat_type ** m) {
	cout << "The Matrix is:" << endl;
	for (int i = 0; i < SIZE_M; i++){
		for (int j = 0; j < SIZE_M; j++){
			cout << setw(Width) << m[i][j];
		}
		cout << endl;
	}
}

//��ӡCSR��ʽ
void printCSR(RLSMatrix m) {
	cout << "data:";
	for (int i = 0; i < m.tu; i++){
		cout<<setw(Width)<<m.data[i];
	}
	cout << endl;
	cout << "col_index:";  //�����е�����
	for (int i = 0; i < m.tu; i++){
		cout << setw(Width) << m.index[i];
	}
	cout << endl;
	cout << "row_index:";//��i��Ԫ�ؼ�¼��ǰi-1�а����ķ���Ԫ�ص�����
	for (int i = 0; i < m.mu+1; i++){
		cout << setw(Width) << m.rpos[i];
	}
	cout << endl;
	cout << endl;
	cout << endl;
}



int main() {

	//�ֱ��ϡ�������з����ڴ�
	A = mallocMatrix(A);
	B = mallocMatrix(B);
	mallocRLSMatrix(M);
	mallocRLSMatrix(N);
	mallocRLSMatrix(Q);
	//�ֱ����ϡ�����ĳ�ʼ��
	InitialMatrix(A);
	InitialMatrix(B);

	//��ӡ����
	//printMatrix(A);
	//printMatrix(B);


    long long tail;
	QueryPerformanceFrequency((LARGE_INTEGER*)&freq);

	//start time
	QueryPerformanceCounter((LARGE_INTEGER*)&head);

	//�ֱ����CSR��Ԫ���ת��
	CSRMartix(A, M);
	CSRMartix(B, N);

    //��ӡ
	//printCSR(M);
	//printCSR(N);

	pthread_mutex_init(&mutex, NULL);  //��ʼ��������
	pthread_barrier_init(&barrier,NULL,NUM_THREADS);   //��ʼ��barrier
	pthread_t handles[NUM_THREADS];  //������Ӧ�ľ��
	threadParm_t parm[NUM_THREADS];  //������Ӧ���߳����ݽṹ

	//CSR��ʽ���о���˷����㣬���������Q
	Q.mu = M.mu;
	Q.nu = N.nu;
	Q.tu = 0;
	if (M.tu * N.tu != 0) {
        for(int t_id = 0;t_id <NUM_THREADS;t_id++){
            parm[t_id].threadId=t_id;
            pthread_create(&handles[t_id],nullptr,threadFunc,(void*)&parm[t_id]);
        }
        for(int t_id =0; t_id<NUM_THREADS;t_id++){
            pthread_join(handles[t_id],NULL);
        }
        Q.rpos[SIZE_M] = Q.tu + 1; //���һ����ֵ
        pthread_mutex_destroy(&mutex);   //�ͷŻ�����
        pthread_barrier_destroy(&barrier);
	}
	//end time
	QueryPerformanceCounter((LARGE_INTEGER*)&tail);
	cout << "CSRѹ������˷���ʱ������Ϊ:" << (tail - head) * 1000.0 / freq<<"ms"<<endl;

    //��ӡ���
    //printCSR(Q);

    //��ӡ�������
    //C=mallocMatrix(C);
    //MatrixCSR(C,Q);
    //printMatrix(C);

    //��ϡ�����ռ�ÿռ�����ͷ�
    freeMatrix(A);
    freeMatrix(B);
    //freeMatrix(C);
    freeRLSMatrix(M);
    freeRLSMatrix(N);
    freeRLSMatrix(Q);




	return 0;
}
