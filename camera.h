#pragma once
// ================================================================
//  camera.h  (v3)
//  ·°È‰¢:
//  1. · ¥ß¿“æ ’ª°µ‘: „™È Mat.data ‚¥¬µ√ßºË“π Bitmap scan (BGRPixelFormat)
//     À√◊Õ·ª≈ß§√—Èß‡¥’¬«µÕπ display ‡∑Ë“π—Èπ ‰¡Ë·ª≈ß´È”
//  2. Thresholds ‡ªÁπ member ¢Õß class  ·°È§Ë“‰¥È®“° TrackBar/NumericUpDown
//  3. ‡æ‘Ë¡·∂∫ Settings Panel ¥È“π≈Ë“ß ”À√—∫ª√—∫ threshold À≈—°
// ================================================================
#include "mangosteen_core.h"

namespace Mangkudd {

    using namespace System;
    using namespace System::Windows::Forms;
    using namespace System::Drawing;
    using namespace System::ComponentModel;

    public ref class camera : public System::Windows::Forms::Form
    {
    private:
        cv::VideoCapture* cap;
        cv::dnn::Net* net;
        System::Windows::Forms::Timer^ cameraTimer;
        bool isCameraRunning = false;
        int countA = 0, countB = 0, countC = 0, countD = 0;

        // ƒƒ Thresholds: ª√—∫‰¥È®“° UI ƒƒƒƒƒƒƒƒƒƒƒƒƒƒƒƒƒƒƒƒƒƒƒƒƒƒƒƒƒƒƒƒ
        Thresholds th;

        // ƒƒ Controls ƒƒƒƒƒƒƒƒƒƒƒƒƒƒƒƒƒƒƒƒƒƒƒƒƒƒƒƒƒƒƒƒƒƒƒƒƒƒƒƒƒƒƒƒƒƒƒƒƒ
        System::Windows::Forms::SplitContainer^ splitMain;
        System::Windows::Forms::PictureBox^ camerashow;
        System::Windows::Forms::Button^ startbutton;
        System::Windows::Forms::Button^ button1;
        System::Windows::Forms::Button^ btnReset;
        System::Windows::Forms::Label^ lblGradeA;
        System::Windows::Forms::Label^ lblGradeB;
        System::Windows::Forms::Label^ lblGradeC;
        System::Windows::Forms::Label^ lblGradeD;

        // Settings controls
        System::Windows::Forms::GroupBox^ grpSettings;
        System::Windows::Forms::Label^ lblConfVal;
        System::Windows::Forms::TrackBar^ tbConf;
        System::Windows::Forms::Label^ lblRghAVal;
        System::Windows::Forms::TrackBar^ tbRghA;
        System::Windows::Forms::Label^ lblRghBVal;
        System::Windows::Forms::TrackBar^ tbRghB;
        System::Windows::Forms::Label^ lblBlmAVal;
        System::Windows::Forms::TrackBar^ tbBlmA;
        System::Windows::Forms::Label^ lblBlmBVal;
        System::Windows::Forms::TrackBar^ tbBlmB;
        System::Windows::Forms::Label^ lblScoreAVal;
        System::Windows::Forms::TrackBar^ tbScoreA;
        System::Windows::Forms::Label^ lblDarkVal;
        System::Windows::Forms::TrackBar^ tbDark;

        System::ComponentModel::Container^ components;

    public:
        camera(void) {
            InitializeComponent();
            cap = new cv::VideoCapture();
            try {
                net = new cv::dnn::Net(
                    cv::dnn::readNetFromONNX("C:\\Users\\ASUS\\Downloads\\best.onnx"));
                net->setPreferableBackend(cv::dnn::DNN_BACKEND_OPENCV);
                net->setPreferableTarget(cv::dnn::DNN_TARGET_CPU);
            }
            catch (...) {
                MessageBox::Show("‰¡Ëæ∫‰ø≈Ï model\n°√ÿ≥“µ√«® Õ∫ path");
            }
            cameraTimer = gcnew System::Windows::Forms::Timer();
            cameraTimer->Interval = 33;
            cameraTimer->Tick += gcnew EventHandler(this, &camera::OnCameraTick);
        }

    protected:
        ~camera() {
            if (cap && cap->isOpened()) cap->release();
            delete cap; delete net;
            if (components) delete components;
        }

    private:
        void InitializeComponent(void) {
            splitMain = gcnew SplitContainer();
            camerashow = gcnew PictureBox();
            startbutton = gcnew Button();
            button1 = gcnew Button();
            btnReset = gcnew Button();
            lblGradeA = gcnew Label();
            lblGradeB = gcnew Label();
            lblGradeC = gcnew Label();
            lblGradeD = gcnew Label();
            grpSettings = gcnew GroupBox();

            tbConf = gcnew TrackBar(); lblConfVal = gcnew Label();
            tbRghA = gcnew TrackBar(); lblRghAVal = gcnew Label();
            tbRghB = gcnew TrackBar(); lblRghBVal = gcnew Label();
            tbBlmA = gcnew TrackBar(); lblBlmAVal = gcnew Label();
            tbBlmB = gcnew TrackBar(); lblBlmBVal = gcnew Label();
            tbScoreA = gcnew TrackBar(); lblScoreAVal = gcnew Label();
            tbDark = gcnew TrackBar(); lblDarkVal = gcnew Label();

            (cli::safe_cast<ISupportInitialize^>(splitMain))->BeginInit();
            splitMain->Panel1->SuspendLayout();
            splitMain->Panel2->SuspendLayout();
            splitMain->SuspendLayout();
            (cli::safe_cast<ISupportInitialize^>(camerashow))->BeginInit();
            this->SuspendLayout();

            // splitMain
            splitMain->Dock = DockStyle::Fill;
            splitMain->SplitterDistance = 800;
            splitMain->Panel1->Controls->Add(camerashow);
            splitMain->Panel2->BackColor = SystemColors::ActiveCaption;
            splitMain->Panel2->Controls->Add(lblGradeA);
            splitMain->Panel2->Controls->Add(lblGradeB);
            splitMain->Panel2->Controls->Add(lblGradeC);
            splitMain->Panel2->Controls->Add(lblGradeD);
            splitMain->Panel2->Controls->Add(btnReset);
            splitMain->Panel2->Controls->Add(button1);
            splitMain->Panel2->Controls->Add(startbutton);
            splitMain->Panel2->Controls->Add(grpSettings);

            // camerashow ó · ¥ß¿“æ BGR µ√ßÊ
            camerashow->BackColor = Color::Black;
            camerashow->Dock = DockStyle::Fill;
            camerashow->SizeMode = PictureBoxSizeMode::Zoom;

            // startbutton
            startbutton->Location = Point(10, 10);
            startbutton->Size = Size(165, 45);
            startbutton->Text = L"START";
            startbutton->Font = gcnew Drawing::Font(L"Microsoft YaHei", 11, FontStyle::Bold);
            startbutton->BackColor = Color::LightGreen;
            startbutton->Click += gcnew EventHandler(this, &camera::startbutton_Click);

            // btnReset
            btnReset->Location = Point(10, 65);
            btnReset->Size = Size(165, 35);
            btnReset->Text = L"RESET COUNT";
            btnReset->BackColor = Color::LightYellow;
            btnReset->Click += gcnew EventHandler(this, &camera::btnReset_Click);

            // Grade labels
            auto makeGradeLabel = [](Label^ lbl, String^ txt, Point pt,
                Color fg, Color bg) {
                    lbl->Location = pt;
                    lbl->Size = Size(180, 38);
                    lbl->Text = txt;
                    lbl->Font = gcnew Drawing::Font(L"Microsoft YaHei", 12, FontStyle::Bold);
                    lbl->ForeColor = fg;
                    lbl->BackColor = bg;
                    lbl->TextAlign = ContentAlignment::MiddleCenter;
                    lbl->BorderStyle = BorderStyle::FixedSingle;
                };
            makeGradeLabel(lblGradeA, L"Grade A: 0", Point(10, 115), Color::DarkGreen, Color::Honeydew);
            makeGradeLabel(lblGradeB, L"Grade B: 0", Point(10, 160), Color::DarkGoldenrod, Color::LightYellow);
            makeGradeLabel(lblGradeC, L"Grade C: 0", Point(10, 205), Color::DarkRed, Color::MistyRose);
            makeGradeLabel(lblGradeD, L"Grade D: 0", Point(10, 250), Color::Gray, Color::WhiteSmoke);

            // ƒƒ Settings GroupBox ƒƒƒƒƒƒƒƒƒƒƒƒƒƒƒƒƒƒƒƒƒƒƒƒƒƒƒƒƒƒƒƒƒƒƒƒƒ
            grpSettings->Location = Point(5, 295);
            grpSettings->Size = Size(195, 290);
            grpSettings->Text = L"Threshold Settings";
            grpSettings->Font = gcnew Drawing::Font(L"Segoe UI", 8);

            // helper ∑’Ë‡æ‘Ë¡ TrackBar + Label ‡¢È“ grpSettings
            int row = 18;
            auto addTrack = [&](TrackBar^ tb, Label^ lbl,
                String^ name, int minV, int maxV, int val, int step) {
                    Label^ caption = gcnew Label();
                    caption->Text = name;
                    caption->Location = Point(5, row);
                    caption->Size = Size(90, 16);
                    caption->Font = gcnew Drawing::Font(L"Segoe UI", 7.5f);
                    grpSettings->Controls->Add(caption);

                    lbl->Location = Point(155, row);
                    lbl->Size = Size(35, 16);
                    lbl->Font = gcnew Drawing::Font(L"Segoe UI", 7.5f);
                    grpSettings->Controls->Add(lbl);

                    tb->Location = Point(95, row - 2);
                    tb->Size = Size(58, 20);
                    tb->Minimum = minV;
                    tb->Maximum = maxV;
                    tb->Value = val;
                    tb->TickFrequency = step;
                    tb->SmallChange = step;
                    tb->LargeChange = step * 5;
                    tb->AutoSize = false;
                    tb->Height = 20;
                    grpSettings->Controls->Add(tb);
                    row += 38;
                };

            addTrack(tbConf, lblConfVal, L"Confidence%", 30, 95, 60, 5);
            addTrack(tbRghA, lblRghAVal, L"Rough A", 10, 100, 45, 5);
            addTrack(tbRghB, lblRghBVal, L"Rough B", 10, 120, 65, 5);
            addTrack(tbBlmA, lblBlmAVal, L"Blemish A%", 1, 20, 3, 1);
            addTrack(tbBlmB, lblBlmBVal, L"Blemish B%", 1, 30, 10, 1);
            addTrack(tbScoreA, lblScoreAVal, L"Score A", 10, 90, 50, 5);
            addTrack(tbDark, lblDarkVal, L"Dark Fill%", 5, 80, 25, 5);

            // µ—Èß§Ë“‡√‘Ë¡µÈπ label
            lblConfVal->Text = "60";
            lblRghAVal->Text = "45";
            lblRghBVal->Text = "65";
            lblBlmAVal->Text = "3";
            lblBlmBVal->Text = "10";
            lblScoreAVal->Text = "50";
            lblDarkVal->Text = "25";

            // wire events
            tbConf->ValueChanged += gcnew EventHandler(this, &camera::OnThresholdChanged);
            tbRghA->ValueChanged += gcnew EventHandler(this, &camera::OnThresholdChanged);
            tbRghB->ValueChanged += gcnew EventHandler(this, &camera::OnThresholdChanged);
            tbBlmA->ValueChanged += gcnew EventHandler(this, &camera::OnThresholdChanged);
            tbBlmB->ValueChanged += gcnew EventHandler(this, &camera::OnThresholdChanged);
            tbScoreA->ValueChanged += gcnew EventHandler(this, &camera::OnThresholdChanged);
            tbDark->ValueChanged += gcnew EventHandler(this, &camera::OnThresholdChanged);

            // button1 (RETURN)
            button1->Location = Point(10, 590);
            button1->Size = Size(165, 45);
            button1->Text = L"RETURN";
            button1->Font = gcnew Drawing::Font(L"Microsoft YaHei", 11, FontStyle::Bold);
            button1->BackColor = Color::LightCoral;
            button1->Click += gcnew EventHandler(this, &camera::button1_Click);

            // Form
            this->ClientSize = Size(1010, 660);
            this->Controls->Add(splitMain);
            this->Text = L"Camera Grading System";

            splitMain->Panel1->ResumeLayout(false);
            splitMain->Panel2->ResumeLayout(false);
            (cli::safe_cast<ISupportInitialize^>(splitMain))->EndInit();
            splitMain->ResumeLayout(false);
            (cli::safe_cast<ISupportInitialize^>(camerashow))->EndInit();
            this->ResumeLayout(false);
        }

        // ƒƒ TrackBar changed  Õ—ª‡¥µ th ƒƒƒƒƒƒƒƒƒƒƒƒƒƒƒƒƒƒƒƒƒƒƒƒƒƒƒƒƒƒ
        System::Void OnThresholdChanged(Object^ sender, EventArgs^ e) {
            th.minConfidence = (float)tbConf->Value / 100.0f;
            th.roughnessA = (double)tbRghA->Value;
            th.roughnessB = (double)tbRghB->Value;
            th.blemishPctA = (double)tbBlmA->Value;
            th.blemishPctB = (double)tbBlmB->Value;
            th.scoreA = (double)tbScoreA->Value;
            th.darkFillMin = (double)tbDark->Value / 100.0;

            lblConfVal->Text = tbConf->Value.ToString();
            lblRghAVal->Text = tbRghA->Value.ToString();
            lblRghBVal->Text = tbRghB->Value.ToString();
            lblBlmAVal->Text = tbBlmA->Value.ToString();
            lblBlmBVal->Text = tbBlmB->Value.ToString();
            lblScoreAVal->Text = tbScoreA->Value.ToString();
            lblDarkVal->Text = tbDark->Value.ToString();
        }

        // ƒƒ OnCameraTick ƒƒƒƒƒƒƒƒƒƒƒƒƒƒƒƒƒƒƒƒƒƒƒƒƒƒƒƒƒƒƒƒƒƒƒƒƒƒƒƒƒƒƒƒƒƒ
        System::Void OnCameraTick(Object^ sender, EventArgs^ e) {
            if (!isCameraRunning || !cap || !cap->isOpened()) return;
            cv::Mat frame;
            if (!cap->read(frame) || frame.empty()) return;

            if (!net) { displayBGR(frame); return; }

            // YOLO
            cv::Mat blob = cv::dnn::blobFromImage(
                frame, 1.0 / 255.0, cv::Size(640, 640), cv::Scalar(0, 0, 0), true, false);
            net->setInput(blob);
            std::vector<cv::Mat> outputs;
            net->forward(outputs, net->getUnconnectedOutLayersNames());

            auto dets = YoloParser::parse(outputs, frame.cols, frame.rows,
                th.minConfidence);

            for (const auto& det : dets) {
                // Un_Ripe  Grade D
                if (det.classId == 1) {
                    countD++;
                    lblGradeD->Text = "Grade D: " + countD.ToString();
                    cv::rectangle(frame, det.box, cv::Scalar(160, 160, 160), 2);
                    char buf[50];
                    snprintf(buf, sizeof(buf), "D Unripe  conf:%.2f", det.confidence);
                    cv::putText(frame, buf,
                        cv::Point(det.box.x, det.box.y - 6),
                        cv::FONT_HERSHEY_SIMPLEX, 0.55, cv::Scalar(160, 160, 160), 1);
                    continue;
                }

                // Ripe  µ√«®¢π“¥
                cv::Rect safe = det.box & cv::Rect(0, 0, frame.cols, frame.rows);
                if (safe.width < th.minBoxSize || safe.height < th.minBoxSize) continue;

                // «‘‡§√“–ÀÏ‡©æ“–„π bounding box
                MangosteenAnalyzer::Result res =
                    MangosteenAnalyzer::analyze(frame(safe).clone(), det.confidence, th);

                if (!res.isValid) {
                    // «“¥°√Õ∫ ’‡À≈◊Õß + SKIP
                    cv::rectangle(frame, det.box, cv::Scalar(0, 200, 255), 1);
                    cv::putText(frame, res.debugText,
                        cv::Point(det.box.x, det.box.y - 5),
                        cv::FONT_HERSHEY_SIMPLEX, 0.38, cv::Scalar(0, 200, 255), 1);
                    continue;
                }

                // π—∫‡°√¥
                switch (res.grade) {
                case 'A': countA++; lblGradeA->Text = "Grade A: " + countA.ToString(); break;
                case 'B': countB++; lblGradeB->Text = "Grade B: " + countB.ToString(); break;
                default:  countC++; lblGradeC->Text = "Grade C: " + countC.ToString(); break;
                }

                cv::Scalar col;
                std::string lbl;
                switch (res.grade) {
                case 'A': col = cv::Scalar(0, 220, 0);   lbl = "A"; break;
                case 'B': col = cv::Scalar(0, 220, 220); lbl = "B"; break;
                default:  col = cv::Scalar(0, 0, 220);   lbl = "C"; break;
                }

                cv::rectangle(frame, det.box, col, 2);

                // ∫π box
                char top[60];
                snprintf(top, sizeof(top), "Grade:%s conf:%.2f", lbl.c_str(), det.confidence);
                cv::putText(frame, top,
                    cv::Point(det.box.x, det.box.y - 6),
                    cv::FONT_HERSHEY_SIMPLEX, 0.50, col, 2);

                // „µÈ box: §Ë“ + threshold
                char ln1[80], ln2[80];
                snprintf(ln1, sizeof(ln1),
                    "Ripe:%.2f  Rgh:%.1f(<%.0f)",
                    res.ripenessScore, res.roughness, th.roughnessA);
                snprintf(ln2, sizeof(ln2),
                    "Blm:%.1f%%(<%.0f%%)  Sc:%.1f",
                    res.blemishPercent, th.blemishPctA, res.finalScore);

                cv::putText(frame, ln1,
                    cv::Point(det.box.x, det.box.y + det.box.height + 14),
                    cv::FONT_HERSHEY_SIMPLEX, 0.36, col, 1);
                cv::putText(frame, ln2,
                    cv::Point(det.box.x, det.box.y + det.box.height + 27),
                    cv::FONT_HERSHEY_SIMPLEX, 0.36, col, 1);
            }

            // · ¥ßº≈ ’ª°µ‘ (BGR  ‰¡Ë flip)
            displayBGR(frame);
        }

        // ƒƒ displayBGR: · ¥ß¿“æ ’∏√√¡™“µ‘ ƒƒƒƒƒƒƒƒƒƒƒƒƒƒƒƒƒƒƒƒƒƒƒƒƒƒƒ
        //  PictureBox √—∫ RGB ·µË OpenCV ‡°Á∫ BGR
        //  ∑”§√—Èß‡¥’¬«∑’Ëπ’Ë∑’Ë‡¥’¬«
        void displayBGR(cv::Mat& bgr) {
            cv::Mat rgb;
            cv::cvtColor(bgr, rgb, cv::COLOR_BGR2RGB);   // ·ª≈ß§√—Èß‡¥’¬«

            System::Drawing::Bitmap^ bmp = gcnew System::Drawing::Bitmap(
                rgb.cols, rgb.rows,
                System::Drawing::Imaging::PixelFormat::Format24bppRgb);

            System::Drawing::Rectangle rect(0, 0, rgb.cols, rgb.rows);
            System::Drawing::Imaging::BitmapData^ bd =
                bmp->LockBits(rect,
                    System::Drawing::Imaging::ImageLockMode::WriteOnly,
                    System::Drawing::Imaging::PixelFormat::Format24bppRgb);

            // copy ∑’≈– row ‡æ√“– Bitmap stride Õ“®‰¡Ëµ√ß°—∫ Mat step
            for (int y = 0; y < rgb.rows; y++) {
                memcpy((unsigned char*)bd->Scan0.ToPointer() + y * bd->Stride,
                    rgb.ptr(y),
                    rgb.cols * 3);
            }
            bmp->UnlockBits(bd);

            System::Drawing::Image^ old = camerashow->Image;
            camerashow->Image = bmp;
            if (old != nullptr) delete old;
        }

        System::Void startbutton_Click(Object^ sender, EventArgs^ e) {
            if (!isCameraRunning) {
                if (!cap) return;
                if (cap->isOpened()) cap->release();
                if (cap->open(0)) {
                    isCameraRunning = true;
                    cameraTimer->Start();
                    startbutton->Text = L"STOP";
                    startbutton->BackColor = Color::Tomato;
                }
                else {
                    MessageBox::Show("‰¡Ë “¡“√∂‡ª‘¥°≈ÈÕß‰¥È");
                }
            }
            else {
                isCameraRunning = false;
                cameraTimer->Stop();
                if (cap && cap->isOpened()) cap->release();
                System::Drawing::Image^ old = camerashow->Image;
                camerashow->Image = nullptr;
                if (old != nullptr) delete old;
                startbutton->Text = L"START";
                startbutton->BackColor = Color::LightGreen;
            }
        }

        System::Void btnReset_Click(Object^ sender, EventArgs^ e) {
            countA = countB = countC = countD = 0;
            lblGradeA->Text = "Grade A: 0"; lblGradeB->Text = "Grade B: 0";
            lblGradeC->Text = "Grade C: 0"; lblGradeD->Text = "Grade D: 0";
        }

        System::Void button1_Click(Object^ sender, EventArgs^ e) {
            if (isCameraRunning) {
                isCameraRunning = false;
                cameraTimer->Stop();
                if (cap && cap->isOpened()) cap->release();
            }
            System::Drawing::Image^ old = camerashow->Image;
            camerashow->Image = nullptr;
            if (old != nullptr) delete old;
            this->Close();
        }
    };

} // namespace Mangkudd