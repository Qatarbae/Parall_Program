//#define _CRT_SECURE_NO_WARNINGS
#include <stdlib.h>
#include <stdio.h>
#include "omp.h"
long long Task(long long x)
{
	long long count = 0;
	long long sum = 0, number = 0;
	sum = x;
	number = x;
	long long j;
	for (j = 0; sum >= 1; j++)
	{
		if (number % 2 == 0) {
			number = number / 2;
		}
		else {
			number = 3 * number + 1;
		}
		if (number == 1) {
			break;
		}
		if (number < 0)
			number = 0;
		sum = number;
		count++;
	}
	return count;
}

int main()
{
#ifdef _OPENMP
	printf("OPENMP ON\n");
#else
	printf("OPENMP OFF\n");
#endif
	long long N;
	long long maxcount = 0;
	long long maxNumber = 0;
	printf("Threads: %d\n", omp_get_max_threads());
	printf("Enter number: ");
	scanf("%lld", &N);
	long* ResultArray = (long*)calloc(N, sizeof(long));
	long long i;
	long long y;
	double t1 = omp_get_wtime();
	for (i = 2; i < N; i++)
	{
		y = Task(i);
		ResultArray[i] = y;
		//printf("%ld\n", ResultArray[i]);

		if (ResultArray[i] > ResultArray[maxNumber]) {
			maxNumber = i;
		}
		maxcount = ResultArray[maxNumber];
	}
	double t2 = omp_get_wtime();
	printf("Posledov");
	printf("\n%lf\n", t2 - t1);
	printf("Max count\n");
	printf("%lld\n", maxcount);
	printf("Number\n");
	printf("%lld\n", maxNumber);
	free(ResultArray);
	long long* ResultArray1 = (long long*)calloc(N, sizeof(long long));
	//omp_set_num_threads(6);
	// PARALLel ------------------------------------------------------------
	t1 = omp_get_wtime();
#pragma omp parallel for private(y) shared(maxcount)
	for (i = 2; i < N; i++)
	{
		y = Task(i);
		#pragma omp critical
		{
			ResultArray1[i] = y;
			if (ResultArray1[i] > ResultArray1[maxNumber]) {
				maxNumber = i;
			}
			maxcount = ResultArray1[maxNumber];
		}
	}
	t2 = omp_get_wtime();
	printf("\nPARALL");
	printf("\n%lf\n", t2 - t1);
	printf("Max count\n");
	printf("%lld\n", maxcount);
	printf("Number\n");
	printf("%lld\n", maxNumber);
	free(ResultArray1);
	return 0;
}
