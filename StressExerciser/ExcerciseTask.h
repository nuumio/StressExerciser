#pragma once
#include "Unmanaged.h"

namespace StressExerciser {
    using namespace System;
    using namespace System::Threading;
    using namespace System::Windows::Forms;

    public enum class TaskStatus {
        Stopped,
        Initializing,
        Initialized,
        Running,
        Stopping,
        Error
    };

    ref class RunParams {
    private:
        CancellationToken^ mCancelToken;
        Semaphore^ mTaskRunSemaphore;
    public:
        RunParams(CancellationToken^ cancelToken, Semaphore^ taskRunSemaphore) {
            mCancelToken = cancelToken;
            mTaskRunSemaphore = taskRunSemaphore;
        }

        property CancellationToken^ CancelToken {
            CancellationToken^ get() {
                return mCancelToken;
            }
        }

        property Semaphore^ TaskRunSemaphore {
            Semaphore^ get() {
                return mTaskRunSemaphore;
            }
        }
    };

    delegate void TaskUiUpdater(Object^ task);

    ref class ExcerciseTask {
    private:
        String^ mName = nullptr;
        Thread^ mTaskThead = nullptr;
        TaskStatus mStatus = TaskStatus::Stopped;
        Control^ mUiControl = nullptr;
        TaskUiUpdater^ mUiUpdater = nullptr;
        Semaphore^ mTaskRunSemaphore = nullptr;
        int mUpdateUi = 0;
        void SetStatus(TaskStatus status) {
            mStatus = status;
            if (Interlocked::CompareExchange(mUpdateUi, 1, 2) == 0) {
                mUiControl->Invoke(mUiUpdater, this);
            }
        }
    protected:
        CancellationTokenSource^ mCancelTokenSource = nullptr;
        virtual void DoInitialize() = 0;
        virtual void Run(Object^ cancelToken) = 0;
        virtual void DoDeinitialize() = 0;
    public:
        ExcerciseTask(Control^ uiControl, TaskUiUpdater^ uiUpdater);
        virtual ~ExcerciseTask();
        property String^ Name {
            String^ get() {
                return mName;
            }
            void set(String^ name) {
                mName = name;
            }
        }
        property TaskStatus Status {
            TaskStatus get() {
                return mStatus;
            }
        }
        property bool Running {
            bool get() {
                return mCancelTokenSource != nullptr && mTaskThead != nullptr && mTaskThead->IsAlive && !mCancelTokenSource->IsCancellationRequested;
            }
        }
        property bool UpdateStatusToUi {
            bool get() {
                return Interlocked::CompareExchange(mUpdateUi, 0, 0) == 0;
            }
            void set(bool update) {
                if (update) {
                    Interlocked::Exchange(mUpdateUi, 0);
                } else {
                    Interlocked::Exchange(mUpdateUi, 1);
                }
            }
        }
        void TaskRunnerJob(Object^ params) {
            RunParams^ p = (RunParams^)params;
            CancellationToken^ ct = p->CancelToken;
            Semaphore^ sem = p->TaskRunSemaphore;
            SetStatus(TaskStatus::Initializing);
            DoInitialize();
            SetStatus(TaskStatus::Initialized);
            sem->WaitOne();
            SetStatus(TaskStatus::Running);
            Run(ct);
            sem->Release();
            DoDeinitialize();
        }
        void Initialize(Semaphore^ taskRunSemaphore) {
            mTaskRunSemaphore = taskRunSemaphore;
            mCancelTokenSource = gcnew CancellationTokenSource();
            mTaskThead = gcnew Thread(gcnew ParameterizedThreadStart(this, &ExcerciseTask::TaskRunnerJob));
            mTaskThead->Start(gcnew RunParams(mCancelTokenSource->Token, taskRunSemaphore));
            SetStatus(TaskStatus::Running);
        }
        void StopJob() {
            SetStatus(TaskStatus::Stopping);
            mCancelTokenSource->Cancel();
            mTaskThead->Join();
            SetStatus(TaskStatus::Stopped);
            mCancelTokenSource = nullptr;
            mTaskThead = nullptr;
        }
        void Stop() {
            Thread^ stopThread = gcnew Thread(gcnew ThreadStart(this, &ExcerciseTask::StopJob));
            stopThread->Start();
        }
    };

    ref class SseFloatSqrtXPerX : public ExcerciseTask {
    public:
        SseFloatSqrtXPerX(Control^ uiControl, TaskUiUpdater^ uiUpdater);
        virtual ~SseFloatSqrtXPerX();
        static ExcerciseTask^ Create(Control^ uiControl, TaskUiUpdater^ uiUpdater) {
            return gcnew SseFloatSqrtXPerX(uiControl, uiUpdater);
        };
    protected:
        void DoInitialize() override;
        void Run(Object^ cancelToken) override;
        void DoDeinitialize() override;
    private:
        int mLength;
        float *mResult;
        CancellationToken^ mCancelToken;
    };

    ref class FpuFloatSqrtXPerX : public ExcerciseTask {
    public:
        FpuFloatSqrtXPerX(Control^ uiControl, TaskUiUpdater^ uiUpdater);
        virtual ~FpuFloatSqrtXPerX();
        static ExcerciseTask^ Create(Control^ uiControl, TaskUiUpdater^ uiUpdater) {
            return gcnew FpuFloatSqrtXPerX(uiControl, uiUpdater);
        };
    protected:
        void DoInitialize() override;
        void Run(Object^ cancelToken) override;
        void DoDeinitialize() override;
    private:
        int mLength;
        float *mResult;
        CancellationToken^ mCancelToken;
    };

    ref class FpuDoubleSqrtXPerX : public ExcerciseTask {
    public:
        FpuDoubleSqrtXPerX(Control^ uiControl, TaskUiUpdater^ uiUpdater);
        virtual ~FpuDoubleSqrtXPerX();
        static ExcerciseTask^ Create(Control^ uiControl, TaskUiUpdater^ uiUpdater) {
            return gcnew FpuDoubleSqrtXPerX(uiControl, uiUpdater);
        };
    protected:
        void DoInitialize() override;
        void Run(Object^ cancelToken) override;
        void DoDeinitialize() override;
    private:
        int mLength;
        double *mResult;
        CancellationToken^ mCancelToken;
    };

    delegate ExcerciseTask^ TaskCreator(Control^ uiControl, TaskUiUpdater^ uiUpdater);

    public ref class ExerciseTaskDescription {
    private:
        String^ mName;
        TaskCreator^ mCreator;
    public:
        static array<ExerciseTaskDescription^>^ sDescriptions = gcnew array<ExerciseTaskDescription^> {
            gcnew ExerciseTaskDescription("FPU float: Sqrt(x) / x", gcnew TaskCreator(&FpuFloatSqrtXPerX::Create)),
                gcnew ExerciseTaskDescription("FPU double: Sqrt(x) / x", gcnew TaskCreator(&FpuDoubleSqrtXPerX::Create)),
                gcnew ExerciseTaskDescription("SSE float: Sqrt(x) / x", gcnew TaskCreator(&SseFloatSqrtXPerX::Create)),
        };
        ExerciseTaskDescription(String^ name, TaskCreator^ creator) {
            mName = name;
            mCreator = creator;
        }
        static array<ExerciseTaskDescription^>^ GetDescriptions() {
            return sDescriptions;
        }
        String^ GetName() {
            return mName;
        }
        property String^ Name {
            String^ get() {
                return mName;
            }
        }
        ExcerciseTask^ CreateTask(Control^ uiControl, TaskUiUpdater^ uiUpdater) {
            ExcerciseTask^ task = mCreator->Invoke(uiControl, uiUpdater);
            task->Name = mName;
            return task;
        }
    };
}
