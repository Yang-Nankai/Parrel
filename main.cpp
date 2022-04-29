//此代码是稀疏矩阵的平凡算法
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

typedef float mat_type;   //进行类型重命名

#define NUM_THREADS 4   //定义线程数

int const SIZE_M = 2048;   //设置矩阵的大小 +1为了方便
int const RANDOM = 20;
int const Width = 4;   //设置显示位宽
int const DATA = 8000000;

long long head, freq;   //timers

void printMatrix(mat_type m[][SIZE_M]);  //函数声明

typedef struct {
	mat_type *data;   //非零元三元组
	int *rpos;   //各行第一个非零元的位置表
	int mu, nu, tu;  //矩阵的行数、列数和非零元的个数
	int *index;   //元素所在的列数
}RLSMatrix;

//定义两个矩阵（二维数组）
mat_type **A,**B,**C,**D;
RLSMatrix M, N, Q; //M, N, Q为为矩阵的CSR压缩格式

typedef struct{
    int threadId;
}threadParm_t;    //结构变量

pthread_barrier_t barrier;   //barrier同步
pthread_mutex_t mutex=PTHREAD_MUTEX_INITIALIZER;

//线程函数
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
    int rpos[SIZE_M];//暂存
    int my_first = (threadId)*(SIZE_M/NUM_THREADS);
    int my_last = (threadId+1)*(SIZE_M/NUM_THREADS);  //计算每个线程需要算到的大小
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


//动态分配二维数组，方便内存释放
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

//通过随机化来进行矩阵的初始化(为了实现稀疏化，这里将矩阵设置为90%的空余，且值为1)
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

//对稀疏矩阵进行CSR压缩
void CSRMartix(mat_type ** m,RLSMatrix& re) {
	int num = 1;   //记录每行第一个非零元素的位置
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
//将CSR格式转换为矩阵格式
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
	//打印矩阵
	//printMatrix(m);
}

//打印矩阵
void printMatrix(mat_type ** m) {
	cout << "The Matrix is:" << endl;
	for (int i = 0; i < SIZE_M; i++){
		for (int j = 0; j < SIZE_M; j++){
			cout << setw(Width) << m[i][j];
		}
		cout << endl;
	}
}

//打印CSR格式
void printCSR(RLSMatrix m) {
	cout << "data:";
	for (int i = 0; i < m.tu; i++){
		cout<<setw(Width)<<m.data[i];
	}
	cout << endl;
	cout << "col_index:";  //保存列的数组
	for (int i = 0; i < m.tu; i++){
		cout << setw(Width) << m.index[i];
	}
	cout << endl;
	cout << "row_index:";//第i个元素记录了前i-1行包含的非零元素的数量
	for (int i = 0; i < m.mu+1; i++){
		cout << setw(Width) << m.rpos[i];
	}
	cout << endl;
	cout << endl;
	cout << endl;
}



int main() {

	//分别对稀疏矩阵进行分配内存
	A = mallocMatrix(A);
	B = mallocMatrix(B);
	mallocRLSMatrix(M);
	mallocRLSMatrix(N);
	mallocRLSMatrix(Q);
	//分别进行稀疏矩阵的初始化
	InitialMatrix(A);
	InitialMatrix(B);

	//打印矩阵
	//printMatrix(A);
	//printMatrix(B);


    long long tail;
	QueryPerformanceFrequency((LARGE_INTEGER*)&freq);

	//start time
	QueryPerformanceCounter((LARGE_INTEGER*)&head);

	//分别进行CSR三元组的转换
	CSRMartix(A, M);
	CSRMartix(B, N);

    //打印
	//printCSR(M);
	//printCSR(N);

	pthread_mutex_init(&mutex, NULL);  //初始化互斥锁
	pthread_barrier_init(&barrier,NULL,NUM_THREADS);   //初始化barrier
	pthread_t handles[NUM_THREADS];  //创建对应的句柄
	threadParm_t parm[NUM_THREADS];  //创建对应的线程数据结构

	//CSR形式进行矩阵乘法运算，结果保存在Q
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
        Q.rpos[SIZE_M] = Q.tu + 1; //最后一个赋值
        pthread_mutex_destroy(&mutex);   //释放互斥量
        pthread_barrier_destroy(&barrier);
	}
	//end time
	QueryPerformanceCounter((LARGE_INTEGER*)&tail);
	cout << "CSR压缩矩阵乘法的时间消耗为:" << (tail - head) * 1000.0 / freq<<"ms"<<endl;

    //打印结果
    //printCSR(Q);

    //打印结果矩阵
    //C=mallocMatrix(C);
    //MatrixCSR(C,Q);
    //printMatrix(C);

    //对稀疏矩阵占用空间进行释放
    freeMatrix(A);
    freeMatrix(B);
    //freeMatrix(C);
    freeRLSMatrix(M);
    freeRLSMatrix(N);
    freeRLSMatrix(Q);




	return 0;
}
