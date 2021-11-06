#include<iostream>
#include<pthread.h>
#include<cstdlib>
#include<cstdio>
#include<sys/time.h>
#include<stdlib.h>
#include<string.h>
#include<iomanip>
using namespace std;

#define Max_Matrix_size 1000
#define Max_pthread_size 100

double Matrix[Max_Matrix_size][Max_Matrix_size];
int Matrix_size;
int Thread_num;
int Per_thread_do;
double Per_thread_cal[Max_pthread_size];
double result = 0.0;
pthread_mutex_t mut;

double det_cal(double **mmatrix, int size){
	if(size == 1){
		return mmatrix[0][0];
	}
	else{
		double res = 0.0;
		double **matrix = (double **)malloc(sizeof(double*) * (size));

		for(int i = 0; i < size - 1; i++){
			matrix[i] = (double *)malloc(sizeof(double) * (size));
		}

        // 按第一行展开
		for(int i = 0; i < size; i++){
			for(int j = 1; j < size; j++){ // 与该元素同行或同列的元素不被保留。matrix[j][]，所以此处表示的是忽略同行元素
				for(int k = 0; k < size; k++){
					if(k == i) // 与该元素同行或同列的元素不被保留。matrix[][k]，所以此处表示的是忽略同列元素
						continue; // 因此j的起始条件与k的判断条件就能实现将该行该列的元素划去
					if(k < i){
						matrix[j - 1][k] = mmatrix[j][k];
					}
					if(k > i){
						matrix[j - 1][k - 1] = mmatrix[j][k];
					}
				}
			}
			if(i % 2 == 0){
				res += mmatrix[0][i] * det_cal(matrix,size - 1);
			}
			else{
				res -= mmatrix[0][i] * det_cal(matrix,size - 1);
			}
		}

        // Free the temp matrix.
		for (int i = 0; i < size - 1; i++) {
			free(matrix[i]);
		}
		free(matrix);

		return res;
	}
}

void *t_hread(void *arg){
	int q = *(int*)arg; // 从0开始的数，代表第q列，每个线程处理的是按第一行第q列展开得到的结果

	// 申请同尺寸矩阵
	double **matrix = (double **)malloc(sizeof(double*) * (Matrix_size));   
	for(int i = 0; i < Matrix_size;i++){
		matrix[i] = (double *)malloc(sizeof(double) * (Matrix_size));
	}

	int end_index = ((q + 1) * Per_thread_do);
	if(q == Thread_num - 1)
		end_index = Matrix_size;

	for(int i = q * Per_thread_do; i < end_index; i++){
		for(int j = 1; j < Matrix_size; j++){
			for(int k = 0; k < Matrix_size; k++){
				if(k < i){
					matrix[j - 1][k] = Matrix[j][k];
				}
				if(k > i){
					matrix[j - 1][k - 1] = Matrix[j][k];
				}
			}
		}
		if(i % 2 == 0){
			Per_thread_cal[q] += Matrix[0][i] * det_cal(matrix,Matrix_size - 1);	
		}
		
		else{
			Per_thread_cal[q] -= Matrix[0][i] * det_cal(matrix,Matrix_size - 1);
		}
	}		
	

	pthread_mutex_lock(&mut);
	result += Per_thread_cal[q];
	pthread_mutex_unlock(&mut);

	for(int i = 0; i < Matrix_size - 1; i++){
		free(matrix[i]);
	}
	free(matrix);

	pthread_exit(0);
}

int main(int argc,char *argv[]){
	if(argc != 4){
		cout<<"Input format:MATRIX_SIZE,THREAD_NUM,INPUT_FILE"<<endl;
		return 0;
	}	

	memset(Per_thread_cal, 0.0, sizeof(Per_thread_cal));// 将该块内存全部设定为指定值

	Matrix_size = atoi(argv[1]);// 字符串转整型   
	Thread_num = atoi(argv[2]);	
	char *input = argv[3];

	Per_thread_do = Matrix_size / Thread_num; // int Per_thread_do

	pthread_mutex_init(&mut,NULL);
	freopen(input,"r",stdin);					
	for(int i = 0;i < Matrix_size;i++){
		for(int j = 0;j < Matrix_size;j++){
			cin>>Matrix[i][j];
		}
	}	


	pthread_t thread[Max_pthread_size];
	//int id[100];
	int *id = (int *)malloc(sizeof(int) * (Thread_num));
	struct timeval begin,end;
    gettimeofday(&begin,NULL);

	for(int i = 0;i < Thread_num;i++){
		id[i] = i;
		pthread_create(&thread[i],NULL,t_hread,(void*)&id[i]);
	}

	for(int i = 0;i < Thread_num;i++){
		pthread_join(thread[i],NULL);
	}
    
	free(id);
	cout<<fixed<<setprecision(2)<<"Det result is : "<<result<<endl;

	return 0;
}