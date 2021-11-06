#include<iostream>
#include<mpi.h>
#include<iomanip>

using namespace std;
#define Matrix_Size_Max 1000

double Matrix[Matrix_Size_Max][Matrix_Size_Max];

double det_cal(double mmatrix[][20], int size){
	if (size == 1){
		return mmatrix[0][0];
	}
	else{
		double res = 0.0;
		double matrix[20][20];
		for (int i = 0; i < size; i++){
			for (int j = 1; j < size; j++){
				for (int k = 0; k < size; k++){
					if (k < i){
						matrix[j - 1][k] = mmatrix[j][k];
					}
					if (k > i){
						matrix[j - 1][k - 1] = mmatrix[j][k];
					}
				}
			}
			if (i % 2 == 0){
				res += mmatrix[0][i] * det_cal(matrix, size - 1);
			}
			else{
				res -= mmatrix[0][i] * det_cal(matrix, size - 1);
			}
		}
		return res;
	}
}

int main(int argc,char *argv[]){
	int num_procs,my_rank;
	double Matrix_size = 0.0;
	double start = 0.0, stop = 0.0;
	double per_procs = 0.0;
	double result = 0.0;
	double matrix[20][20];

	MPI_Init(&argc, &argv);
	MPI_Comm_size(MPI_COMM_WORLD, &num_procs);
	MPI_Comm_rank(MPI_COMM_WORLD, &my_rank);
	
	if (my_rank == 0){
		char input[200];
		cout << "Matrix Size:";
		cin >> Matrix_size;
		cout << "Input File:";
		cin >> input;
		freopen(input, "r", stdin);
		for (int i = 0; i < Matrix_size; i++){
			for (int j = 0; j < Matrix_size; j++){
				cin >> Matrix[i][j];
			}
		}
	}

	MPI_Bcast(&Matrix_size, 1, MPI_DOUBLE, 0, MPI_COMM_WORLD);    //将矩阵大小广播给所有进程

	for (int i = 0; i < Matrix_size; i++){
		MPI_Bcast(Matrix[i], Matrix_size, MPI_DOUBLE, 0, MPI_COMM_WORLD);
	}			//将矩阵广播给所有进程

	start = MPI_Wtime();
	for (int i = my_rank; i < Matrix_size; i += num_procs){
		double sum = 0.0;
		for (int k = 1; k < Matrix_size; k++){
			for (int l = 0; l < Matrix_size; l++){
				if (l < i){
					matrix[k - 1][l] = Matrix[k][l];
				}
				if (l > i){
					matrix[k - 1][l - 1] = Matrix[k][l];
				}
			}
		}
		if (i % 2 == 0){
			sum += Matrix[0][i] * det_cal(matrix, Matrix_size - 1);
		}
		if (i % 2 != 0){
			sum -= Matrix[0][i] * det_cal(matrix, Matrix_size - 1);
		}
		per_procs += sum;
	}

	MPI_Reduce(&per_procs, &result, 1, MPI_DOUBLE, MPI_SUM, 0, MPI_COMM_WORLD);

	if (my_rank == 0){
		cout <<fixed<<setprecision(2)<<result << endl;
		stop = MPI_Wtime();
		printf("Time: %f\n", stop - start);
		fflush(stdout);
	}
	MPI_Finalize();
	return 0;
}