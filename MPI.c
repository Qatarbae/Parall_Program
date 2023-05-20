#define _CRT_SECURE_NO_WARNINGS
#include "mpi.h"
#include <stdio.h>
#include <stdlib.h>
#include<math.h>
#define epsilon 1e-5
#define  FILENAME "koeff_17_100.bin"
float* Zei(int max, float** mas,float*B);
void Gaus(int max, float** mas, float* B);
int myid;
int numprocs;
float* rez;
int main(int agrc, char* argv[])
{
	double eps = 1e-5;
  	int n = 4;
  	int i;
  	int rc;
  	long double drob,drobSum=0, sum;
  	double startwtime = 0.0;
  	double endwtime;
	char name[80];
	printf("Parall\n");
	printf("filename: ");
	scanf("%s",&name);
	//MPI_Init(&agrc, &argv);
  	if (rc= MPI_Init(&agrc, &argv)) 
  	{ 
    		printf("ERROR");
    		MPI_Abort(MPI_COMM_WORLD, rc);
  	} 

	MPI_Comm_size(MPI_COMM_WORLD,&numprocs);
	MPI_Comm_rank(MPI_COMM_WORLD,&myid); 
	MPI_Bcast(&n, 1, MPI_INT, 0, MPI_COMM_WORLD);
	MPI_Bcast(&eps, 1, MPI_DOUBLE, 0, MPI_COMM_WORLD);
	int max;
	float*itog = NULL,** mas = NULL,*B=NULL;
	FILE* filein = fopen(name, "rb");
	fread(&max, sizeof(int), 1, filein);
	mas = (float**)malloc(max * (sizeof(float*)));
	B = (float*)malloc(max * sizeof(float));
	for(int i=0;i<max;i++){
		mas[i] = (float*)malloc(max * (sizeof(float)));
	}
	for (int i = 0; i < max; i++) {
		for (int j = 0; j < max; j++) {
			fread(&mas[i][j], sizeof(float), 1, filein);
		}
		fread(&B[i], sizeof(float), 1, filein);
	}
	if (myid == 0)
 	{ 
  	
    	startwtime = MPI_Wtime();
  	}
	Gaus(max, mas, B);
	itog = Zei(max, mas,B);
  	if (myid == 0)
  	{   
    	endwtime = MPI_Wtime();
    	printf("%lf\n", endwtime-startwtime);    
  	}
	MPI_Finalize();
	FILE* fileout = fopen("otvet.txt", "wb");
	for (int i = 0; i < max; i++) {
		//fwrite(&itog[i], sizeof(float), 1, fileout);
		fprintf(fileout, " %lf ", itog[i]);
		//printf("%f\n", itog[i]);
	}
	free(mas);
	free(itog);
	free(B);

	fclose(filein);
	fclose(fileout);
}

float* Zei(int max, float** mas,float*B) {
	float* x = (float*)malloc(max * sizeof(float));
	float delta = 10;
	rez = (float*)malloc(max * sizeof(float));
	for (int i = 0; i < max; i++) {
		//x[i] =  1/mas[i][i];
		x[i] = B[i];
	}
	while (delta >= epsilon) {
		delta = 0;
		for (int i = 0; i < max; i++) {
			float a = B[i];
			for (int j = 0; j < max; j++)
				if (j!=i)
				a = a- x[j]*mas[i][j];
			a = a / mas[i][i];
			if (fabs(x[i] - a) > delta) delta = fabs(x[i] - a);
			x[i] = a;
			MPI_Reduce(&x[i], &rez[i], 1, MPI_DOUBLE, MPI_SUM, 0, MPI_COMM_WORLD);
		}
	}
	return rez;
}

void Gaus(int max, float** mas, float* B) {
	for (int i = 0; i < max; i++) {
		for (int j = 0; j < max; j++) {
			if (i != j) {
				float koef= -mas[j][i] / mas[i][i];
				for (int k = 0; k < max; k++) {
					mas[j][k] = mas[j][k] + koef * mas[i][k];
				}
				B[j] = B[j] + B[i] * koef;
			}
		}
	}
}
