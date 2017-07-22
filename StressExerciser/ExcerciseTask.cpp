#include "ExcerciseTask.h"
#include "Unmanaged.h"
#include "ExcerciseTask.h"

namespace StressExerciser {
    ExcerciseTask::ExcerciseTask(Control^ uiControl, TaskUiUpdater^ uiUpdater) {
        mUiControl = uiControl;
        mUiUpdater = uiUpdater;
        UpdateStatusToUi = true;
    }
    ExcerciseTask::~ExcerciseTask() {}

    SseFloatSqrtXPerX::SseFloatSqrtXPerX(Control^ uiControl, TaskUiUpdater^ uiUpdater) : ExcerciseTask(uiControl, uiUpdater) {
        mCancelToken = nullptr;
    }
    SseFloatSqrtXPerX::~SseFloatSqrtXPerX() {}
    void SseFloatSqrtXPerX::DoInitialize() {
        mLength = 64000;
        mResult = (float *)aligned_malloc(mLength * sizeof(float), 16);
    }
    void SseFloatSqrtXPerX::Run(Object^ cancelToken) {
        CancellationToken^ ct = (CancellationToken^)cancelToken;
        while (!ct->IsCancellationRequested) {
            sse_float_sqrt_x_per_x(mResult, mLength / 4, 100000);
        }
    }
    void SseFloatSqrtXPerX::DoDeinitialize() {
        _aligned_free(mResult);
    }

    FpuFloatSqrtXPerX::FpuFloatSqrtXPerX(Control^ uiControl, TaskUiUpdater^ uiUpdater) : ExcerciseTask(uiControl, uiUpdater) {
        mCancelToken = nullptr;
    }
    FpuFloatSqrtXPerX::~FpuFloatSqrtXPerX() {}
    void FpuFloatSqrtXPerX::DoInitialize() {
        mLength = 64000;
        mResult = (float *)malloc(mLength * sizeof(float));
    }
    void FpuFloatSqrtXPerX::Run(Object^ cancelToken) {
        CancellationToken^ ct = (CancellationToken^)cancelToken;
        while (!ct->IsCancellationRequested) {
            fpu_float_sqrt_x_per_x(mResult, mLength, 1000);
        }
    }
    void FpuFloatSqrtXPerX::DoDeinitialize() {
        free(mResult);
    }

    FpuDoubleSqrtXPerX::FpuDoubleSqrtXPerX(Control^ uiControl, TaskUiUpdater^ uiUpdater) : ExcerciseTask(uiControl, uiUpdater) {
        mCancelToken = nullptr;
    }
    FpuDoubleSqrtXPerX::~FpuDoubleSqrtXPerX() {}
    void FpuDoubleSqrtXPerX::DoInitialize() {
        mLength = 64000;
        mResult = (double *)malloc(mLength * sizeof(double));
    }
    void FpuDoubleSqrtXPerX::Run(Object^ cancelToken) {
        CancellationToken^ ct = (CancellationToken^)cancelToken;
        while (!ct->IsCancellationRequested) {
            fpu_double_sqrt_x_per_x(mResult, mLength, 1000);
        }
    }
    void FpuDoubleSqrtXPerX::DoDeinitialize() {
        free(mResult);
    }
}
