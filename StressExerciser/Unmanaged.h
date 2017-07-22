#pragma once
#include <xmmintrin.h>

public class SimpleSseVars {
private:
    int mLength;
    float *mResult;
public:
    SimpleSseVars(int length);
    float * GetResultArray();
};

void * aligned_malloc(size_t size, size_t alignment);
void sse_float_sqrt_x_per_x(float * pResult, int SSELength, int repeats);
void fpu_float_sqrt_x_per_x(float * pResult, int length, int repeats);
void fpu_double_sqrt_x_per_x(double * pResult, int length, int repeats);
