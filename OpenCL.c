#define _CRT_SECURE_NO_WARNINGS
#include <stdio.h>
#include <time.h>
#include <malloc.h>
#include <stdlib.h>
#include <CL/cl_platform.h>
#include <CL/cl.h>
#define BLOCK_SIZE 32
#define TRUE (1 == 1)
#define FALSE (!TRUE)

char* errCodeToString(int err) {
	switch (err) {
	case CL_SUCCESS:                            return "Success!";
	case CL_DEVICE_NOT_FOUND:                   return "Device not found.";
	case CL_DEVICE_NOT_AVAILABLE:               return "Device not available";
	case CL_COMPILER_NOT_AVAILABLE:             return "Compiler not available";
	case CL_MEM_OBJECT_ALLOCATION_FAILURE:      return "Memory object allocation failure";
	case CL_OUT_OF_RESOURCES:                   return "Out of resources";
	case CL_OUT_OF_HOST_MEMORY:                 return "Out of host memory";
	case CL_PROFILING_INFO_NOT_AVAILABLE:       return "Profiling information not available";
	case CL_MEM_COPY_OVERLAP:                   return "Memory copy overlap";
	case CL_IMAGE_FORMAT_MISMATCH:              return "Image format mismatch";
	case CL_IMAGE_FORMAT_NOT_SUPPORTED:         return "Image format not supported";
	case CL_BUILD_PROGRAM_FAILURE:              return "Program build failure";
	case CL_MAP_FAILURE:                        return "Map failure";
	case CL_INVALID_VALUE:                      return "Invalid value";
	case CL_INVALID_DEVICE_TYPE:                return "Invalid device type";
	case CL_INVALID_PLATFORM:                   return "Invalid platform";
	case CL_INVALID_DEVICE:                     return "Invalid device";
	case CL_INVALID_CONTEXT:                    return "Invalid context";
	case CL_INVALID_QUEUE_PROPERTIES:           return "Invalid queue properties";
	case CL_INVALID_COMMAND_QUEUE:              return "Invalid command queue";
	case CL_INVALID_HOST_PTR:                   return "Invalid host pointer";
	case CL_INVALID_MEM_OBJECT:                 return "Invalid memory object";
	case CL_INVALID_IMAGE_FORMAT_DESCRIPTOR:    return "Invalid image format descriptor";
	case CL_INVALID_IMAGE_SIZE:                 return "Invalid image size";
	case CL_INVALID_SAMPLER:                    return "Invalid sampler";
	case CL_INVALID_BINARY:                     return "Invalid binary";
	case CL_INVALID_BUILD_OPTIONS:              return "Invalid build options";
	case CL_INVALID_PROGRAM:                    return "Invalid program";
	case CL_INVALID_PROGRAM_EXECUTABLE:         return "Invalid program executable";
	case CL_INVALID_KERNEL_NAME:                return "Invalid kernel name";
	case CL_INVALID_KERNEL_DEFINITION:          return "Invalid kernel definition";
	case CL_INVALID_KERNEL:                     return "Invalid kernel";
	case CL_INVALID_ARG_INDEX:                  return "Invalid argument index";
	case CL_INVALID_ARG_VALUE:                  return "Invalid argument value";
	case CL_INVALID_ARG_SIZE:                   return "Invalid argument size";
	case CL_INVALID_KERNEL_ARGS:                return "Invalid kernel arguments";
	case CL_INVALID_WORK_DIMENSION:             return "Invalid work dimension";
	case CL_INVALID_WORK_GROUP_SIZE:            return "Invalid work group size";
	case CL_INVALID_WORK_ITEM_SIZE:             return "Invalid work item size";
	case CL_INVALID_GLOBAL_OFFSET:              return "Invalid global offset";
	case CL_INVALID_EVENT_WAIT_LIST:            return "Invalid event wait list";
	case CL_INVALID_EVENT:                      return "Invalid event";
	case CL_INVALID_OPERATION:                  return "Invalid operation";
	case CL_INVALID_GL_OBJECT:                  return "Invalid OpenGL object";
	case CL_INVALID_BUFFER_SIZE:                return "Invalid buffer size";
	case CL_INVALID_MIP_LEVEL:                  return "Invalid mip-map level";
	default: return "Unknown";
	}
}
int parallel(unsigned long n, long int* tStart, long int* tEnd, struct timespec * tStart2, struct timespec * tEnd2, uint* maxnum ) {
	cl_int clerr;
	cl_uint qty_platforms = 0;
	cl_platform_id* platforms;
	cl_uint ui;
	cl_uint* qty_devices;
	cl_device_id** devices;
	cl_context cntx;
	cl_event kEvent;
	cl_command_queue cq;
	cl_device_id* ds;
	cl_program p;
	cl_kernel k;
	uint* res = (uint*)malloc(n*10 * sizeof(uint));
	char* program_source = "__kernel void find_len (unsigned long number, __global unsigned long * result)" 
	"{"
		"unsigned long i = get_global_id(0);"
		"unsigned long q = number+i;"
		"int x = 0;"
  	"for (unsigned long k = 1; k < q; k++)"
    	"{"
        	"if (q % k == 0) {"
            		"unsigned long check_n = k + (q / k);"
            		"int isPrime = 0;"
            		"unsigned long j;"
    
            	"for (j = 2; j < check_n; j++)"
            	"{"
                	"if (check_n % j == 0) {"
				"isPrime = 1;"
                    		"break;"
                	"}"
            	"}"
            	"if (isPrime == 0) {"
                	"continue;"
            	"}"
            	"else {"
			"x = 1;"
			"break;"
            	"}"
        	"}"
    	"}"
	"if(x == 0){"
	"result[i] = q;"
	"return;"
	"}"
	"else{result[i] = 0;}"
	"}";
	size_t global_work_size[1] = { n };
	size_t cb;
	int i = -1;
	clerr = clGetPlatformIDs(0, NULL, &qty_platforms);
	if (clerr != CL_SUCCESS) {
		fprintf(stderr, "Error, code = %d.\n", clerr);
		return 1;
	}
	platforms = (cl_platform_id*)malloc(sizeof(cl_platform_id) * qty_platforms);
	devices = (cl_device_id**)malloc(sizeof(cl_device_id*) * qty_platforms);
	qty_devices = (cl_uint*)malloc(sizeof(cl_uint) * qty_platforms);
	clerr = clGetPlatformIDs(qty_platforms, platforms, NULL);
	for (ui = 0; ui < qty_platforms; ui++) {
		clerr = clGetDeviceIDs(platforms[ui], CL_DEVICE_TYPE_ALL, 0, NULL, &qty_devices[ui]);
		if (qty_devices[ui]) {
			devices[ui] = (cl_device_id*)malloc(qty_devices[ui] * sizeof(cl_device_id));
			clerr = clGetDeviceIDs(platforms[ui], CL_DEVICE_TYPE_ALL, qty_devices[ui], devices[ui], NULL);
		}
	}
	clerr = CL_SUCCESS;
	cntx = clCreateContext(0, qty_devices[0], devices[0], NULL, NULL, &clerr);
	cq = clCreateCommandQueue(cntx, devices[0][0], CL_QUEUE_PROFILING_ENABLE, &clerr);
	p = clCreateProgramWithSource(cntx, 1, (const char**)&program_source, NULL, &clerr);

	clerr = clBuildProgram(p, 0, NULL, NULL /*clcompileflags*/, NULL, NULL);
	if (clerr == CL_BUILD_PROGRAM_FAILURE) {

		// Determine the size of the log
		size_t log_size;
		clGetProgramBuildInfo(p, devices[0][0], CL_PROGRAM_BUILD_LOG, 0, NULL, &log_size);

		// Allocate memory for the log
		char* log = (char*)malloc(log_size);

		// Get the log
		clGetProgramBuildInfo(p, devices[0][0], CL_PROGRAM_BUILD_LOG, log_size, log, NULL);

		// Print the log
		printf("\nerror\n");
		FILE* flog = fopen("errorLog.txt", "w");
		fprintf(flog, "%s\n", log);
		fclose(flog);
		return 1;
	}
	clock_gettime(CLOCK_REALTIME, tStart2);
	k = clCreateKernel(p, "find_len", &clerr);
	cl_mem result = clCreateBuffer(cntx, CL_MEM_READ_WRITE, n * sizeof(unsigned long), NULL, NULL);
	i = -1;
	clSetKernelArg(k, i += 1, sizeof(unsigned long), &n);
	clSetKernelArg(k, i += 1, sizeof(cl_mem), &result);
	kEvent = clCreateUserEvent(cntx, &clerr);
	clerr = clEnqueueNDRangeKernel(cq, k, 1, NULL,
		global_work_size, NULL, 0, NULL, &kEvent);
	if (clerr != 0)
		printf("\n%s\n", errCodeToString(clerr));
	clerr = clEnqueueReadBuffer(cq, result, CL_TRUE, 0, n * sizeof(uint), res, 0, NULL, NULL);//???
	clerr = clGetEventProfilingInfo(kEvent, CL_PROFILING_COMMAND_START, sizeof(*tStart), tStart, NULL);
	clerr = clGetEventProfilingInfo(kEvent, CL_PROFILING_COMMAND_END, sizeof(*tEnd), tEnd, NULL);
	int ii = 0;
	int gg = 0;
	clock_gettime(CLOCK_REALTIME, tEnd2);
	while(gg == 0){
		ii++;
		if(res[ii] != 0){
		printf("\nFound number parall: %lld", res[ii]); gg=1;}
	}
	clReleaseMemObject(result);
	clReleaseKernel(k);
	clReleaseProgram(p);
	clReleaseCommandQueue(cq);
	clReleaseContext(cntx);
	free(res);
        return 0;
}
long long chk_num(long long number) {
    long long res = 0;
    int x = 0;
    long long q = number;
    while (q>= number)
    {
        q++;
        x = 0;
        if (q % 2 == 1) {
            x = 1;
            continue;
        }

        long long i = 0;
        for (i = 1; i < q; i++)
        {
            if (q % i == 0) {
                long long check_n = i + (q / i);
                int isPrime = TRUE;
                long long j;

                for (j = 2; j < check_n; j++)
                {
                    if (check_n % j == 0) {
                        isPrime = FALSE;
                        x = 1;
                        break;
                    }
                }

                if (isPrime) {
                    continue;
                }
                else {
                    x = 1;
                    break;
                }
            }
        }

        if (x == 0) {
            res = q;
            FILE* out = fopen("res.txt", "w");
            long long i = 0;
            for (i = 1; i < q; i++)
            {
                if (q % i == 0) {
                    long long checking_number = i + (q / i);
                    int isPrime = TRUE;
                    long long j;
                    fprintf(out, " %lld", i);
                    for (j = 2; j < checking_number; j++)
                    {
                        if (checking_number % j == 0) {
                            isPrime = FALSE;
                            x = 1;
                            break;
                        }
                    }

                    if (isPrime) {
                        continue;
                    }
                    else {
                        x = 1;
                        break;
                    }
                }
            }
            fclose(out);
            q = -1;
            return res;
        }
    }
}
int main()
{
    printf("Enter N:\n");
    unsigned long N;
    scanf("%lld", &N);
	long int tStart = 0l;
	long int tEnd = 0l;
	struct timespec tStart2;
	struct timespec tEnd2;
	struct timespec tStart3;
	struct timespec tEnd3;

	uint degree= 0;
	uint nump = 0;
	uint nums = 0;
    //ПОСЛЕДОВАТЕЛЬНАЯ 
    clock_gettime(CLOCK_REALTIME, &tStart3);
    long long q = 0;
    long long seq_number_result = 0;
    seq_number_result = chk_num(N);
    clock_gettime(CLOCK_REALTIME, &tEnd3);
	printf("...");

    //ПАРАЛЛЕЛЬНАЯ 
	parallel(N, &tStart, &tEnd, &tStart2, &tEnd2, &nump);
    //результаты
    long long tt = 1000000000 * (tEnd2.tv_sec - tStart2.tv_sec) + (tEnd2.tv_nsec - tStart2.tv_nsec);
    long long tk = 1000000000 * (tEnd3.tv_sec - tStart3.tv_sec) + (tEnd3.tv_nsec - tStart3.tv_nsec);

    printf("\nFound number posled: %lld", seq_number_result);
    printf("\nTime parall: %lld ns", tt);
    printf("\nTime posled: %lld ns\n", tk);
    return 0;
}
