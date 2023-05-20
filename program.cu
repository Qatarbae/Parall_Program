/* 10. Вычисление обратной матрицы */

#include <cuda.h>
#include <cuda_runtime.h>
#include "device_launch_parameters.h"
#include <math.h>
#include <stdlib.h>
#include <stdio.h>
#include <time.h>

__device__ int max_element_index;

void print(double* matrix, int n) {
    for (int i = 0; i < n; i++) {
        for (int j = 0; j < n; j++) {
            printf("%lf \t", matrix[i * n + j]);
        }
        printf("\n");
    }
}


__global__ void swap_lines(double* matrix, double* identity, int n, int row) {
    if (row == max_element_index) {
        return;
    }

    int idx = blockIdx.x * blockDim.x + threadIdx.x;
    int offset = gridDim.x * blockDim.x;

    double temp;
    for (; idx < n; idx += offset) {
	//__syncthreads(); 
        temp = matrix[row * n + idx];
        matrix[row * n + idx] = matrix[max_element_index * n + idx];
        matrix[max_element_index * n + idx] = temp;

        temp = identity[row * n + idx];
        identity[row * n + idx] = identity[max_element_index * n + idx];
        identity[max_element_index * n + idx] = temp;
    }
}

__global__ void max_in_column(double* matrix, int n, int x) {
    int max_index = x;
    double max_value = fabs(matrix[x * n + x]);
    double current_value;

    for (int i = x + 1; i < n; i++) {
	//__syncthreads(); 
        current_value = fabs(matrix[i * n + x]);
        if (current_value > max_value) {
            max_index = i;
            max_value = current_value;
        }
    }

    max_element_index = max_index;
}

__global__ void devide_identity(double* matrix, double* identity, int n) {
    int idx = blockIdx.x * blockDim.x + threadIdx.x;
    int idy = blockIdx.y * blockDim.y + threadIdx.y;
    int offsetx = gridDim.x * blockDim.x;
    int offsety = gridDim.y * blockDim.y;

    for (int i = idx; i < n; i += offsetx) {
        for (int j = idy; j < n; j += offsety) {
	   // __syncthreads();
            identity[i * n + j] /= matrix[i * n + i];
        }
    }
}

__global__ void devide_matrix(double* matrix, int n) {
    int idx = blockIdx.x * blockDim.x + threadIdx.x;
    int offsetx = gridDim.x * blockDim.x;
    for (int i = idx; i < n; i += offsetx) {
        matrix[i * n + i] = 1.0;
    }
}

__global__ void subtract_below(double* matrix, double* identity, int n, int x) {
    int idx = blockIdx.x * blockDim.x + threadIdx.x;
    int idy = blockIdx.y * blockDim.y + threadIdx.y;
    int offsetx = gridDim.x * blockDim.x;
    int offsety = gridDim.y * blockDim.y;

    int i, j;
    double coeff;
    for (i = x + 1 + idx; i < n; i += offsetx) {
        coeff = matrix[i * n + x] / matrix[x * n + x];
        for (j = x + 1 + idy; j < n; j += offsety) {
		//__syncthreads();
            matrix[i * n + j] -= coeff * matrix[x * n + j];
        }
        for (j = idy; j < n; j += offsety) {
		//__syncthreads();
            identity[i * n + j] -= coeff * identity[x * n + j];
        }
    }
}

__global__ void nullify_below(double* matrix, int n, int x) {
    int idx = blockIdx.x * blockDim.x + threadIdx.x;
    int offsetx = gridDim.x * blockDim.x;
    for (int i = x + 1 + idx; i < n; i += offsetx) {
        matrix[i * n + x] = 0.0;
    }
}

__global__ void subtract_above(double* matrix, double* identity, int n, int x) {
    int idx = threadIdx.x + blockIdx.x * blockDim.x;
    int idy = threadIdx.y + blockIdx.y * blockDim.y;
    int offsetx = gridDim.x * blockDim.x;
    int offsety = gridDim.y * blockDim.y;

    int i, j;
    double coeff;
    for (i = x - 1 - idx; i >= 0; i -= offsetx) {
        coeff = matrix[i * n + x] / matrix[x * n + x];
        for (j = x - 1 - idy; j >= 0; j -= offsety) {
		//__syncthreads();
            matrix[i * n + j] -= coeff * matrix[x * n + j];
        }
        for (j = idy; j < n; j += offsety) {
		//__syncthreads();
            identity[i * n + j] -= coeff * identity[x * n + j];
        }
    }
}

__global__ void nullify_above(double* matrix, int n, int x) {
    int idx = blockIdx.x * blockDim.x + threadIdx.x;
    int offsetx = gridDim.x * blockDim.x;
    for (int i = x - idx - 1; i >= 0; i -= offsetx) {
        matrix[i * n + x] = 0.0;
    }
}

void inverse_gpu(double* matrix, double* identity, int n) {
    dim3 BLOCKS_1D(16);
    dim3 THREADS_1D(32);
    dim3 BLOCKS_2D(16, 16);
    dim3 THREADS_2D(32, 32);

    double* dev_matrix;
    double* dev_identity;
    cudaMalloc(&dev_matrix, sizeof(double) * n * n);
    cudaMalloc(&dev_identity, sizeof(double) * n * n);
    cudaMemcpy(dev_matrix, matrix, sizeof(double) * n * n,
        cudaMemcpyHostToDevice);
    cudaMemcpy(dev_identity, identity, sizeof(double) * n * n,
        cudaMemcpyHostToDevice);



    for (int i = 0; i < n; i++) {
        max_in_column << <1, 1 >> > (dev_matrix, n, i);
        swap_lines << <BLOCKS_1D, THREADS_1D >> > (dev_matrix, dev_identity, n, i);
        subtract_below << <BLOCKS_2D, THREADS_2D >> > (dev_matrix, dev_identity, n, i);
        nullify_below << <BLOCKS_1D, THREADS_1D >> > (dev_matrix, n, i);
    }

    for (int i = n - 1; i >= 0; i--) {
        subtract_above << <BLOCKS_2D, THREADS_2D >> > (dev_matrix, dev_identity, n, i);
        nullify_above << <BLOCKS_1D, THREADS_1D >> > (dev_matrix, n, i);
    }

    devide_identity << <BLOCKS_2D, THREADS_2D >> > (dev_matrix, dev_identity, n);
    devide_matrix << <BLOCKS_1D, THREADS_1D >> > (dev_matrix, n);


    cudaMemcpy(matrix, dev_matrix, sizeof(double) * n * n,
        cudaMemcpyDeviceToHost);
    cudaMemcpy(identity, dev_identity, sizeof(double) * n * n,
        cudaMemcpyDeviceToHost);
    cudaFree(dev_matrix);
    cudaFree(dev_identity);
}


void swap_lines(double* matrix, double* identity, int n, int i, int j) {
    double temp;
    for (int k = 0; k < n; k++) {
        temp = matrix[i * n + k];
        matrix[i * n + k] = matrix[j * n + k];
        matrix[j * n + k] = temp;

        temp = identity[i * n + k];
        identity[i * n + k] = identity[j * n + k];
        identity[j * n + k] = temp;
    }
}

void pivotize(double* matrix, double* identity, int n, int row) {
    int max_index = row;
    double max_value = fabs(matrix[row * n + row]);
    double current_value;
    for (int i = row + 1; i < n; i++) {
        current_value = fabs(matrix[i * n + row]);
        if (current_value > max_value) {
            max_index = i;
            max_value = current_value;
        }
    }

    if (row != max_index) {
        swap_lines(matrix, identity, n, row, max_index);
    }
}

void devide(double* matrix, int n, int i, double denominator) {
    for (int j = 0; j < n; j++) {
        matrix[i * n + j] /= denominator;
    }
}

void subtract_below1(double* matrix, double* identity, int n, int x) {
    double coeff;
    for (int i = x + 1; i < n; i++) {
        coeff = matrix[i * n + x] / matrix[x * n + x];
        for (int j = x; j < n; j++) {
            matrix[i * n + j] -= coeff * matrix[x * n + j];
        }
        for (int j = 0; j < n; j++) {
            identity[i * n + j] -= coeff * identity[x * n + j];
        }
    }
}

void subtract_above1(double* matrix, double* identity, int n, int x) {
    double coeff;
    for (int i = x - 1; i >= 0; i--) {
        coeff = matrix[i * n + x] / matrix[x * n + x];
        for (int j = x; j >= 0; j--) {
            matrix[i * n + j] -= coeff * matrix[x * n + j];
        }
        for (int j = 0; j < n; j++) {
            identity[i * n + j] -= coeff * identity[x * n + j];
        }
    }
}

void inverse1(double* matrix, double* identity, int n) {
    for (int i = 0; i < n - 1; i++) {
        pivotize(matrix, identity, n, i);

        subtract_below1(matrix, identity, n, i);
    }

    for (int i = n - 1; i > 0; i--) {
        subtract_above1(matrix, identity, n, i);
    }

    for (int i = 0; i < n; i++) {
        devide(identity, n, i, matrix[i * n + i]);
        devide(matrix, n, i, matrix[i * n + i]);
    }
}

int** CreateArray(int N)
{
    int i, j;
    int** arr = (int**)malloc(N * sizeof(int*));
    for (i = 0; i < N; i++)
        arr[i] = (int*)malloc(N * sizeof(int));

    for (i = 0; i < N; i++)
        for (j = 0; j < N; j++)
            arr[i][j] = 0;

    return arr;
}

int main() {
    int n;
    int** arr = 0;
	char text[30];
	printf("input file: ");
	scanf("%s", &text);
    FILE* f = fopen(text, "rb");
    if (f) {
        printf("\nПроизводится считывание матрицы из файла. Пожалуйста, подождите...\n");
        // Находим размерность матрицы
        int str, str2;
        fread(&str, sizeof(int), 1, f);
        fread(&str2, sizeof(int), 1, f);
        n = str;
	printf("%d\n", n);
        arr = CreateArray(str); // Создание матрицы по считанным размерам
                              // Запись в матрицу значений из файла
        for (int i = 0; i < str; i++) {
            for (int j = 0; j < str2; j++)
            {
                fread(&arr[i][j], sizeof(int), 1, f);
            }
        }
       /* for (int i = 0; i < str; i++) {
            for (int j = 0; j < str2; j++)
            {
                printf("%d ",arr[i][j]);
            }
            printf("\n");
        }*/
        fclose(f);
    }
    else if (!f) {
        printf("File error.");
        return 1;
    }

    double* matrix = (double*)malloc(n * n * sizeof(double));
    double* identity = (double*)malloc(n * n * sizeof(double));

    for (int i = 0; i < n; i++) {
        for (int j = 0; j < n; j++) {
            matrix[i * n + j] = (double)arr[i][j];
        }
    }

    for (int i = 0; i < n; i++) {
        for (int j = 0; j < n; j++) {
            identity[i * n + j] = i == j ? 1.0 : 0.0;
        }
    }

   //Структуры для сохранения определенного времени
   struct timespec mt1, mt2; 
   
   //Определяем текущее время
   clock_gettime (CLOCK_REALTIME, &mt1);

    inverse_gpu(matrix, identity, n);

    clock_gettime (CLOCK_REALTIME, &mt2);
    double t = (mt2.tv_sec-mt1.tv_sec)*1000+(mt2.tv_nsec-mt1.tv_nsec)/1000000;
    printf ("Parall mls: %lf\n", t);
	printf("Posled \n");
    double* matrix1 = (double*)malloc(n * n * sizeof(double));
    double* identity1 = (double*)malloc(n * n * sizeof(double));

    for (int i = 0; i < n; i++) {
        for (int j = 0; j < n; j++) {
            matrix1[i * n + j] = (double)arr[i][j];
        }
    }

    for (int i = 0; i < n; i++) {
        for (int j = 0; j < n; j++) {
            identity1[i * n + j] = i == j ? 1.0 : 0.0;
        }
    }
clock_gettime(CLOCK_REALTIME, &mt1);
	inverse1(matrix1, identity1, n);
	//print(identity1, n);
clock_gettime (CLOCK_REALTIME, &mt2);
 t  = (mt2.tv_sec-mt1.tv_sec)*1000+(mt2.tv_nsec-mt1.tv_nsec)/1000000;
    printf ("Parall mls: %lf\n", t);
double sum = 0;
    for (int i = 0; i < n; i++) {
	    for (int j = 0; j < n; j++) {
            sum+= abs(identity[i*n+j] - identity1[i*n+j]);
        }
    }
	printf("dif = %.16lf\n", sum);

 	FILE* out = fopen("Results_parall.txt", "w");
  for (int i = 0; i < n; i++) {
    for (int j = 0; j < n; j++) {
	//identity[i*n+j] = round(identity[i*n+j]*1000)/1000;
	//fwrite(&identity[i*n+j], sizeof(double), 1, out);
	fprintf(out, "%lf ",identity[i*n+j]); 
}
fputc('\n', out);
}

FILE* out1 = fopen("Results_posled.txt", "w");
  for (int i = 0; i < n; i++) {
    for (int j = 0; j < n; j++) {
	//identity1[i*n+j] = round(identity1[i*n+j]*1000)/1000;
        //fwrite(&identity1[i*n+j], sizeof(double), 1, out1);
	fprintf(out1, "%lf ",identity1[i*n+j]);
}
fputc('\n', out1);
}

	fclose(out);
	fclose(out1);
    free(matrix);
    free(identity);
    free(matrix1);
    free(identity1);

    return 0;
}

