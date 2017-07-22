#pragma once
#include "ExcerciseTask.h"

namespace StressExerciser {

    using namespace System;
    using namespace System::ComponentModel;
    using namespace System::Collections;
    using namespace System::Collections::Generic;
    using namespace System::Data;
    using namespace System::Diagnostics;
    using namespace System::Drawing;
    using namespace System::Reflection;
    using namespace System::Windows::Forms;

    // NOTE: Change version in Resource.rc too to update exe->properties->details.
    [assembly:AssemblyVersionAttribute("0.1")];

    /// <summary>
    /// Summary for StressExerciser
    /// </summary>
    public ref class StressExerciser : public System::Windows::Forms::Form {
    private:
        List<ExcerciseTask^>^ mTasks = gcnew List<ExcerciseTask^>();
        CancellationTokenSource^ mCancelSource = nullptr;
        Semaphore^ mRunSemaphore = nullptr;
        Int32 mErrorCount = 0;
        Int32 mInitializedCount = 0;
        Int32 mRunningCount = 0;
        Int32 mStoppedCount = 0;

        Stopwatch ^mStopwatch = gcnew Stopwatch();
        System::Windows::Forms::Timer^ mTimer = gcnew System::Windows::Forms::Timer();
        static StressExerciser^ sInstance = nullptr;

        String^ mVersion = "0.0";

        const int COL_NUMBER = 0;
        const int COL_NAME = 1;
        const int COL_STATUS = 2;
        const int COL_CONFIGURATION = 3; // TODO: Maybe later...

    private: System::Windows::Forms::Button^  btnAddWorkloadx8;
    private: System::Windows::Forms::Button^  btnAddWorkloadx4;
    private: System::Windows::Forms::Button^  btnRemoveWorkload;
    private: System::Windows::Forms::GroupBox^  groupBoxExercise;
    private: System::Windows::Forms::NumericUpDown^  numAutostopDelay;
    private: System::Windows::Forms::Label^  labelSeconds;
    private: System::Windows::Forms::CheckBox^  cbAutostop;

    public:
        StressExerciser(void) {
            InitializeComponent();

            // Timer related setup
            sInstance = this;
            mTimer->Interval = 1000;
            mTimer->Tick += gcnew EventHandler(StressExerciser::TimerEvent);

            // Set name and version to window title.
            // Major.Minor is used always. Build and revision are added if they're != 0.
            Version^ version = StressExerciser::typeid->Assembly->GetEntryAssembly()->GetName()->Version;
            mVersion = version->Major.ToString() + "." + version->Minor.ToString();
            if (version->Build > 0 || version->Revision > 0) {
                mVersion += "." + version->Build.ToString();
                if (version->Revision > 0) {
                    mVersion += "." + version->Revision.ToString();
                }
            }
            Text = Text + " " + mVersion;

            // Get excercise names
            array<ExerciseTaskDescription^>^ descriptions = ExerciseTaskDescription::GetDescriptions();
            for each(ExerciseTaskDescription^ desc in descriptions) {
                String^ name = desc->GetName();
                cbWorkloadName->Items->Add(desc);
            }
            cbWorkloadName->SelectedIndex = 0;

            // Setup list view
            lstWorkloads->Columns->Add("", 40, HorizontalAlignment::Left);
            lstWorkloads->Columns->Add("Workload", 192, HorizontalAlignment::Left);
            lstWorkloads->Columns->Add("Status", 96, HorizontalAlignment::Left);
            // TODO: Maybe later...
            //lstWorkloads->Columns->Add("Configuration", -2, HorizontalAlignment::Left);

            SetRemoveWorkloadButtonEnabled();
        }

        void UpdateTaskList(Object^ task) {
            int errorCount = 0;
            int initializedCount = 0;
            int runningCount = 0;
            int stoppedCount = 0;
            ExcerciseTask^ t = (ExcerciseTask^)task;
            for each(ListViewItem^ item in lstWorkloads->Items) {
                ExcerciseTask^ itemTask = (ExcerciseTask^)item->Tag;
                if (itemTask == t) {
                    switch (t->Status) {
                    case TaskStatus::Error:
                        item->SubItems[COL_STATUS]->Text = "Error";
                        errorCount = Interlocked::Increment(mErrorCount);
                        break;
                    case TaskStatus::Initialized:
                        item->SubItems[COL_STATUS]->Text = "Initialized";
                        initializedCount = Interlocked::Increment(mInitializedCount);
                        break;
                    case TaskStatus::Initializing:
                        item->SubItems[COL_STATUS]->Text = "Initializing";
                        break;
                    case TaskStatus::Running:
                        item->SubItems[COL_STATUS]->Text = "Running";
                        runningCount = Interlocked::Increment(mRunningCount);
                        break;
                    case TaskStatus::Stopped:
                        item->SubItems[COL_STATUS]->Text = "Stopped";
                        stoppedCount = Interlocked::Increment(mStoppedCount);
                        break;
                    case TaskStatus::Stopping:
                        item->SubItems[COL_STATUS]->Text = "Stopping";
                        break;
                    default:
                        item->SubItems[COL_STATUS]->Text = "N/A";
                        break;
                    }
                }
            }
            if (mRunSemaphore != nullptr) {
                if (!btnStartStopExercise->Enabled) {
                    if (initializedCount == lstWorkloads->Items->Count) {
                        mRunSemaphore->Release(Int32::MaxValue);
                    } else if (stoppedCount == lstWorkloads->Items->Count) {
                        mRunSemaphore->Close();
                        mRunSemaphore = nullptr;
                        btnStartStopExercise->Text = "Start the exercise";
                        SetRemoveWorkloadButtonEnabled();
                        SetStartStopButtonEnabled();
                        btnStartStopExercise->Focus();
                        cbWorkloadName->Enabled = true;
                        lstWorkloads->Enabled = true;
                        btnAddWorkload->Enabled = true;
                        btnAddWorkloadx4->Enabled = true;
                        btnAddWorkloadx8->Enabled = true;
                        cbAutostop->Enabled = true;
                        numAutostopDelay->Enabled = true;
                    } else if (runningCount == lstWorkloads->Items->Count) {
                        btnStartStopExercise->Enabled = true;
                        btnStartStopExercise->Focus();
                    }
                }
            }
        }

    protected:
        /// <summary>
        /// Clean up any resources being used.
        /// </summary>
        ~StressExerciser() {
            if (components) {
                delete components;
            }
        }
    private: System::Windows::Forms::ListView^  lstWorkloads;
    protected:

    protected:

    protected:
    private: System::Windows::Forms::GroupBox^  groupBoxWorkloads;
    private: System::Windows::Forms::ComboBox^  cbWorkloadName;
    private: System::Windows::Forms::Button^  btnAddWorkload;
    private: System::Windows::Forms::Button^  btnStartStopExercise;


    private:
        /// <summary>
        /// Required designer variable.
        /// </summary>
        System::ComponentModel::Container ^components;

#pragma region Windows Form Designer generated code
        /// <summary>
        /// Required method for Designer support - do not modify
        /// the contents of this method with the code editor.
        /// </summary>
        void InitializeComponent(void) {
            System::ComponentModel::ComponentResourceManager^  resources = (gcnew System::ComponentModel::ComponentResourceManager(StressExerciser::typeid));
            this->lstWorkloads = (gcnew System::Windows::Forms::ListView());
            this->groupBoxWorkloads = (gcnew System::Windows::Forms::GroupBox());
            this->btnRemoveWorkload = (gcnew System::Windows::Forms::Button());
            this->btnAddWorkloadx8 = (gcnew System::Windows::Forms::Button());
            this->btnAddWorkloadx4 = (gcnew System::Windows::Forms::Button());
            this->btnAddWorkload = (gcnew System::Windows::Forms::Button());
            this->cbWorkloadName = (gcnew System::Windows::Forms::ComboBox());
            this->btnStartStopExercise = (gcnew System::Windows::Forms::Button());
            this->groupBoxExercise = (gcnew System::Windows::Forms::GroupBox());
            this->labelSeconds = (gcnew System::Windows::Forms::Label());
            this->numAutostopDelay = (gcnew System::Windows::Forms::NumericUpDown());
            this->cbAutostop = (gcnew System::Windows::Forms::CheckBox());
            this->groupBoxWorkloads->SuspendLayout();
            this->groupBoxExercise->SuspendLayout();
            (cli::safe_cast<System::ComponentModel::ISupportInitialize^>(this->numAutostopDelay))->BeginInit();
            this->SuspendLayout();
            // 
            // lstWorkloads
            // 
            this->lstWorkloads->Anchor = static_cast<System::Windows::Forms::AnchorStyles>((((System::Windows::Forms::AnchorStyles::Top | System::Windows::Forms::AnchorStyles::Bottom)
                | System::Windows::Forms::AnchorStyles::Left)
                | System::Windows::Forms::AnchorStyles::Right));
            this->lstWorkloads->FullRowSelect = true;
            this->lstWorkloads->GridLines = true;
            this->lstWorkloads->Location = System::Drawing::Point(6, 93);
            this->lstWorkloads->Name = L"lstWorkloads";
            this->lstWorkloads->Size = System::Drawing::Size(537, 209);
            this->lstWorkloads->TabIndex = 5;
            this->lstWorkloads->UseCompatibleStateImageBehavior = false;
            this->lstWorkloads->View = System::Windows::Forms::View::Details;
            this->lstWorkloads->SelectedIndexChanged += gcnew System::EventHandler(this, &StressExerciser::lstWorkloads_SelectedIndexChanged);
            this->lstWorkloads->KeyDown += gcnew System::Windows::Forms::KeyEventHandler(this, &StressExerciser::lstWorkloads_KeyDown);
            // 
            // groupBoxWorkloads
            // 
            this->groupBoxWorkloads->Anchor = static_cast<System::Windows::Forms::AnchorStyles>((((System::Windows::Forms::AnchorStyles::Top | System::Windows::Forms::AnchorStyles::Bottom)
                | System::Windows::Forms::AnchorStyles::Left)
                | System::Windows::Forms::AnchorStyles::Right));
            this->groupBoxWorkloads->Controls->Add(this->btnRemoveWorkload);
            this->groupBoxWorkloads->Controls->Add(this->btnAddWorkloadx8);
            this->groupBoxWorkloads->Controls->Add(this->btnAddWorkloadx4);
            this->groupBoxWorkloads->Controls->Add(this->btnAddWorkload);
            this->groupBoxWorkloads->Controls->Add(this->cbWorkloadName);
            this->groupBoxWorkloads->Controls->Add(this->lstWorkloads);
            this->groupBoxWorkloads->Location = System::Drawing::Point(12, 12);
            this->groupBoxWorkloads->Name = L"groupBoxWorkloads";
            this->groupBoxWorkloads->Size = System::Drawing::Size(549, 342);
            this->groupBoxWorkloads->TabIndex = 1;
            this->groupBoxWorkloads->TabStop = false;
            this->groupBoxWorkloads->Text = L"Workloads";
            // 
            // btnRemoveWorkload
            // 
            this->btnRemoveWorkload->Anchor = static_cast<System::Windows::Forms::AnchorStyles>(((System::Windows::Forms::AnchorStyles::Bottom | System::Windows::Forms::AnchorStyles::Left)
                | System::Windows::Forms::AnchorStyles::Right));
            this->btnRemoveWorkload->Enabled = false;
            this->btnRemoveWorkload->Location = System::Drawing::Point(6, 308);
            this->btnRemoveWorkload->Name = L"btnRemoveWorkload";
            this->btnRemoveWorkload->Size = System::Drawing::Size(537, 28);
            this->btnRemoveWorkload->TabIndex = 6;
            this->btnRemoveWorkload->Text = L"Remove selected workload";
            this->btnRemoveWorkload->UseVisualStyleBackColor = true;
            this->btnRemoveWorkload->Click += gcnew System::EventHandler(this, &StressExerciser::btnRemoveWorkload_Click);
            // 
            // btnAddWorkloadx8
            // 
            this->btnAddWorkloadx8->Anchor = static_cast<System::Windows::Forms::AnchorStyles>((System::Windows::Forms::AnchorStyles::Top | System::Windows::Forms::AnchorStyles::Right));
            this->btnAddWorkloadx8->Location = System::Drawing::Point(368, 59);
            this->btnAddWorkloadx8->Name = L"btnAddWorkloadx8";
            this->btnAddWorkloadx8->Size = System::Drawing::Size(175, 28);
            this->btnAddWorkloadx8->TabIndex = 4;
            this->btnAddWorkloadx8->Text = L"Add 8 Workloads";
            this->btnAddWorkloadx8->UseVisualStyleBackColor = true;
            this->btnAddWorkloadx8->Click += gcnew System::EventHandler(this, &StressExerciser::btnAddWorkload_Click);
            // 
            // btnAddWorkloadx4
            // 
            this->btnAddWorkloadx4->Anchor = System::Windows::Forms::AnchorStyles::Top;
            this->btnAddWorkloadx4->Location = System::Drawing::Point(187, 59);
            this->btnAddWorkloadx4->Name = L"btnAddWorkloadx4";
            this->btnAddWorkloadx4->Size = System::Drawing::Size(175, 28);
            this->btnAddWorkloadx4->TabIndex = 3;
            this->btnAddWorkloadx4->Text = L"Add 4 Workloads";
            this->btnAddWorkloadx4->UseVisualStyleBackColor = true;
            this->btnAddWorkloadx4->Click += gcnew System::EventHandler(this, &StressExerciser::btnAddWorkload_Click);
            // 
            // btnAddWorkload
            // 
            this->btnAddWorkload->Location = System::Drawing::Point(6, 59);
            this->btnAddWorkload->Name = L"btnAddWorkload";
            this->btnAddWorkload->Size = System::Drawing::Size(175, 28);
            this->btnAddWorkload->TabIndex = 2;
            this->btnAddWorkload->Text = L"Add 1 Workload";
            this->btnAddWorkload->UseVisualStyleBackColor = true;
            this->btnAddWorkload->Click += gcnew System::EventHandler(this, &StressExerciser::btnAddWorkload_Click);
            // 
            // cbWorkloadName
            // 
            this->cbWorkloadName->Anchor = static_cast<System::Windows::Forms::AnchorStyles>(((System::Windows::Forms::AnchorStyles::Top | System::Windows::Forms::AnchorStyles::Left)
                | System::Windows::Forms::AnchorStyles::Right));
            this->cbWorkloadName->DisplayMember = L"Name";
            this->cbWorkloadName->DropDownStyle = System::Windows::Forms::ComboBoxStyle::DropDownList;
            this->cbWorkloadName->FormattingEnabled = true;
            this->cbWorkloadName->Location = System::Drawing::Point(6, 25);
            this->cbWorkloadName->Name = L"cbWorkloadName";
            this->cbWorkloadName->Size = System::Drawing::Size(537, 28);
            this->cbWorkloadName->TabIndex = 1;
            this->cbWorkloadName->KeyPress += gcnew System::Windows::Forms::KeyPressEventHandler(this, &StressExerciser::cbWorkloadName_KeyPress);
            // 
            // btnStartStopExercise
            // 
            this->btnStartStopExercise->Anchor = static_cast<System::Windows::Forms::AnchorStyles>(((System::Windows::Forms::AnchorStyles::Top | System::Windows::Forms::AnchorStyles::Left)
                | System::Windows::Forms::AnchorStyles::Right));
            this->btnStartStopExercise->Enabled = false;
            this->btnStartStopExercise->Location = System::Drawing::Point(6, 55);
            this->btnStartStopExercise->Name = L"btnStartStopExercise";
            this->btnStartStopExercise->Size = System::Drawing::Size(537, 28);
            this->btnStartStopExercise->TabIndex = 9;
            this->btnStartStopExercise->Text = L"Start the exercise";
            this->btnStartStopExercise->UseVisualStyleBackColor = true;
            this->btnStartStopExercise->Click += gcnew System::EventHandler(this, &StressExerciser::btnStartStopExercise_Click);
            // 
            // groupBoxExercise
            // 
            this->groupBoxExercise->Anchor = static_cast<System::Windows::Forms::AnchorStyles>(((System::Windows::Forms::AnchorStyles::Bottom | System::Windows::Forms::AnchorStyles::Left)
                | System::Windows::Forms::AnchorStyles::Right));
            this->groupBoxExercise->Controls->Add(this->labelSeconds);
            this->groupBoxExercise->Controls->Add(this->numAutostopDelay);
            this->groupBoxExercise->Controls->Add(this->cbAutostop);
            this->groupBoxExercise->Controls->Add(this->btnStartStopExercise);
            this->groupBoxExercise->Location = System::Drawing::Point(12, 360);
            this->groupBoxExercise->Name = L"groupBoxExercise";
            this->groupBoxExercise->Size = System::Drawing::Size(549, 91);
            this->groupBoxExercise->TabIndex = 2;
            this->groupBoxExercise->TabStop = false;
            this->groupBoxExercise->Text = L"Exercise";
            // 
            // labelSeconds
            // 
            this->labelSeconds->Anchor = static_cast<System::Windows::Forms::AnchorStyles>((System::Windows::Forms::AnchorStyles::Top | System::Windows::Forms::AnchorStyles::Right));
            this->labelSeconds->AutoSize = true;
            this->labelSeconds->Location = System::Drawing::Point(319, 26);
            this->labelSeconds->Name = L"labelSeconds";
            this->labelSeconds->Size = System::Drawing::Size(69, 20);
            this->labelSeconds->TabIndex = 10;
            this->labelSeconds->Text = L"seconds";
            // 
            // numAutostopDelay
            // 
            this->numAutostopDelay->Anchor = static_cast<System::Windows::Forms::AnchorStyles>(((System::Windows::Forms::AnchorStyles::Top | System::Windows::Forms::AnchorStyles::Left)
                | System::Windows::Forms::AnchorStyles::Right));
            this->numAutostopDelay->Location = System::Drawing::Point(191, 23);
            this->numAutostopDelay->Maximum = System::Decimal(gcnew cli::array< System::Int32 >(4) { 31536000, 0, 0, 0 });
            this->numAutostopDelay->Minimum = System::Decimal(gcnew cli::array< System::Int32 >(4) { 1, 0, 0, 0 });
            this->numAutostopDelay->Name = L"numAutostopDelay";
            this->numAutostopDelay->Size = System::Drawing::Size(122, 26);
            this->numAutostopDelay->TabIndex = 8;
            this->numAutostopDelay->Value = System::Decimal(gcnew cli::array< System::Int32 >(4) { 60, 0, 0, 0 });
            // 
            // cbAutostop
            // 
            this->cbAutostop->AutoSize = true;
            this->cbAutostop->Checked = true;
            this->cbAutostop->CheckState = System::Windows::Forms::CheckState::Checked;
            this->cbAutostop->Location = System::Drawing::Point(6, 25);
            this->cbAutostop->Name = L"cbAutostop";
            this->cbAutostop->Size = System::Drawing::Size(179, 24);
            this->cbAutostop->TabIndex = 7;
            this->cbAutostop->Text = L"Automatic stop after";
            this->cbAutostop->UseVisualStyleBackColor = true;
            // 
            // StressExerciser
            // 
            this->AutoScaleDimensions = System::Drawing::SizeF(9, 20);
            this->AutoScaleMode = System::Windows::Forms::AutoScaleMode::Font;
            this->ClientSize = System::Drawing::Size(573, 469);
            this->Controls->Add(this->groupBoxExercise);
            this->Controls->Add(this->groupBoxWorkloads);
            this->Icon = (cli::safe_cast<System::Drawing::Icon^>(resources->GetObject(L"$this.Icon")));
            this->MinimumSize = System::Drawing::Size(595, 525);
            this->Name = L"StressExerciser";
            this->Text = L"Stress Exerciser";
            this->FormClosing += gcnew System::Windows::Forms::FormClosingEventHandler(this, &StressExerciser::StressExerciser_FormClosing);
            this->groupBoxWorkloads->ResumeLayout(false);
            this->groupBoxExercise->ResumeLayout(false);
            this->groupBoxExercise->PerformLayout();
            (cli::safe_cast<System::ComponentModel::ISupportInitialize^>(this->numAutostopDelay))->EndInit();
            this->ResumeLayout(false);

        }
#pragma endregion
    private:
        void SetRemoveWorkloadButtonEnabled() {
            if (lstWorkloads->SelectedItems->Count > 1) {
                btnRemoveWorkload->Enabled = true;
                btnRemoveWorkload->Text = "Remove selected workloads";
            } else if (lstWorkloads->SelectedItems->Count == 1) {
                btnRemoveWorkload->Enabled = true;
                btnRemoveWorkload->Text = "Remove selected workload";
            } else {
                btnRemoveWorkload->Enabled = false;
                btnRemoveWorkload->Text = "Remove selected workload";
            }
        }
        void SetStartStopButtonEnabled() {
            if (lstWorkloads->Items->Count > 0) {
                btnStartStopExercise->Enabled = true;
            } else {
                btnStartStopExercise->Enabled = false;
            }
        }
    private: void AddTasks(int count) {
        ExerciseTaskDescription^ taskDesc = (ExerciseTaskDescription^)cbWorkloadName->SelectedItem;
        for (int i = 0; i < count; i++) {
            ExcerciseTask^ task = taskDesc->CreateTask(this, gcnew TaskUiUpdater(this, &StressExerciser::UpdateTaskList));
            ListViewItem^ item = gcnew ListViewItem((lstWorkloads->Items->Count + 1).ToString());
            item->Tag = task;
            item->SubItems->Add(task->Name);
            item->SubItems->Add("");
            item->SubItems->Add("");
            lstWorkloads->Items->Add(item);
        }
        SetStartStopButtonEnabled();
    }
    private: void RemoveSelectedTasks() {
        if (lstWorkloads->SelectedItems->Count == 0) {
            return;
        }
        for each(ListViewItem^ item in lstWorkloads->SelectedItems) {
            item->Remove();
        }
        int i = 1;
        for each(ListViewItem^ item in lstWorkloads->Items) {
            item->SubItems[COL_NUMBER]->Text = i.ToString();
            i++;
        }
        SetRemoveWorkloadButtonEnabled();
        SetStartStopButtonEnabled();
    }
    private: static void TimerEvent(Object^ sender, EventArgs^ eventArgs) {
        long autostop = System::Convert::ToInt64(sInstance->numAutostopDelay->Text) * 1000L;
        if (sInstance->mTasks->Count > 0 && sInstance->cbAutostop->Checked && sInstance->mStopwatch->ElapsedMilliseconds >= autostop) {
            sInstance->StopExercise();
        }
    }
    private: void StartExercise() {
        // Update UI
        btnStartStopExercise->Enabled = false;
        btnRemoveWorkload->Enabled = false;
        btnAddWorkload->Enabled = false;
        btnAddWorkloadx4->Enabled = false;
        btnAddWorkloadx8->Enabled = false;
        cbWorkloadName->Enabled = false;
        lstWorkloads->Enabled = false;
        cbAutostop->Enabled = false;
        numAutostopDelay->Enabled = false;
        btnStartStopExercise->Text = "Stop the exercise";

        // Restart stopwatch and setup timer
        mStopwatch->Restart();
        mTimer->Start();

        // Start new tasks
        Interlocked::Exchange(mErrorCount, 0);
        Interlocked::Exchange(mInitializedCount, 0);
        Interlocked::Exchange(mRunningCount, 0);
        Interlocked::Exchange(mStoppedCount, 0);

        mRunSemaphore = gcnew Semaphore(0, Int32::MaxValue);
        for each(ListViewItem^ item in lstWorkloads->Items) {
            ExcerciseTask^ task = (ExcerciseTask^)item->Tag;
            mTasks->Add(task);
            task->Initialize(mRunSemaphore);
        }
    }
    private: void StopExercise() {
        // Stop running tasks first as under extreme loads even UI updates may take a long time (for start/stop button)
        // NOTE: Currently it's expected that caller has checked if stop is really necessary.
        for each(ExcerciseTask^ task in mTasks) {
            task->Stop();
        }
        mTasks->Clear();

        // Stop timer
        mTimer->Stop();

        // Update UI
        btnStartStopExercise->Enabled = false;
        btnStartStopExercise->Text = "Stopping";
    }

    private: System::Void btnAddWorkload_Click(System::Object^  sender, System::EventArgs^  e) {
        Button^ button = (Button^)sender;
        int count = 1;
        if (String::Compare(button->Name, "btnAddWorkloadx4") == 0) {
            count = 4;
        } else if (String::Compare(button->Name, "btnAddWorkloadx8") == 0) {
            count = 8;
        }
        AddTasks(count);
    }
    private: System::Void btnStartStopExercise_Click(System::Object^  sender, System::EventArgs^  e) {
        if (mTasks->Count > 0) {
            StopExercise();
        } else {
            StartExercise();
        }
    }
    private: System::Void lstWorkloads_SelectedIndexChanged(System::Object^  sender, System::EventArgs^  e) {
        SetRemoveWorkloadButtonEnabled();
    }
    private: System::Void btnRemoveWorkload_Click(System::Object^  sender, System::EventArgs^  e) {
        RemoveSelectedTasks();
    }
    private: System::Void cbWorkloadName_KeyPress(System::Object^  sender, System::Windows::Forms::KeyPressEventArgs^  e) {
        if (e->KeyChar == ' ' || e->KeyChar == '\r') {
            AddTasks(1);
        }
    }
    private: System::Void lstWorkloads_KeyDown(System::Object^  sender, System::Windows::Forms::KeyEventArgs^  e) {
        if (e->KeyData == Keys::Delete) {
            RemoveSelectedTasks();
        }
    }
    private: System::Void StressExerciser_FormClosing(System::Object^  sender, System::Windows::Forms::FormClosingEventArgs^  e) {
        if (mTasks->Count > 0) {
            for each(ListViewItem^ item in lstWorkloads->Items) {
                ExcerciseTask^ task = (ExcerciseTask^)item->Tag;
                // Prevent UI updates to disposed form
                task->UpdateStatusToUi = false;
            }
            StopExercise();
        }
    }
    };
}
