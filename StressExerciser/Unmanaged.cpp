#include "Unmanaged.h"
#include <xmmintrin.h>
#include <math.h>

SimpleSseVars::SimpleSseVars(int length) {
    mLength = length;
    mResult = (float*)_aligned_malloc(mLength * sizeof(float), 16);
}
float * SimpleSseVars::GetResultArray() {
    return mResult;
}

void * aligned_malloc(size_t size, size_t alignment) {
    return _aligned_malloc(size, alignment);
}

void sse_float_sqrt_x_per_x(float * pResult, int SSELength, int repeats) {
    __m128 x;
    __m128 xDelta = _mm_set1_ps(4.0f);
    __m128 *pResultSSE = (__m128*) pResult;
    for (int n = 0; n < repeats; n++) {
        x = _mm_set_ps(4.0f, 3.0f, 2.0f, 1.0f);
        for (int i = 0; i < SSELength; i++) {
            __m128 xSqrt = _mm_sqrt_ps(x);
            pResultSSE[i] = _mm_div_ps(xSqrt, x);
            x = _mm_add_ps(x, xDelta);
        }
    }
}

void fpu_float_sqrt_x_per_x(float * pResult, int length, int repeats) {
    for (int n = 0; n < repeats; n++) {
        float x = 1.0f;
        for (int i = 0; i < length; i++) {
            pResult[i] = sqrt(x) / x;
            x += 1.0f;
        }
    }
}

void fpu_double_sqrt_x_per_x(double * pResult, int length, int repeats) {
    for (int n = 0; n < repeats; n++) {
        double x = 1.0;
        for (int i = 0; i < length; i++) {
            pResult[i] = sqrt(x) / x;
            x += 1.0;
        }
    }
}
