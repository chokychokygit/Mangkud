#pragma once
// ================================================================
//  showvdo.h  (v2)
//  เปลี่ยนแปลง:
//  - ส่ง det.confidence เข้า MangosteenAnalyzer::analyze()
//  - ข้าม detection ที่ grade == '?' (ไม่ผ่าน guard)
//  - วาด debug threshold text ใต้ bounding box (เหมือน camera.h)
// ================================================================
#include "mangosteen_core.h"

namespace Mangkudd {

using namespace System;
using namespace System::Windows::Forms;
using namespace System::Drawing;
using namespace System::ComponentModel;

public
ref class showvdo : public System::Windows::Forms::Form {
private:
  cv::VideoCapture *video;
  cv::dnn::Net *net;

  System::Windows::Forms::Timer ^ videoTimer;
  System::String ^ selectedPath = "";
  bool isPaused = false;

  int countA = 0, countB = 0, countC = 0, countD = 0;

public:
  showvdo(void) {
    InitializeComponent();
    video = new cv::VideoCapture();
    try {
      net = new cv::dnn::Net(cv::dnn::readNetFromONNX("best.onnx"));
      net->setPreferableBackend(cv::dnn::DNN_BACKEND_OPENCV);
      net->setPreferableTarget(cv::dnn::DNN_TARGET_CPU);
    } catch (...) {
      MessageBox::Show(
          "ไม่พบไฟล์ model ที่ C:\\Users\\ASUS\\Downloads\\best.onnx\nกรุณาตรวจสอบ "
          "path ของไฟล์");
    }
    videoTimer = gcnew System::Windows::Forms::Timer();
    videoTimer->Interval = 33;
    videoTimer->Tick += gcnew EventHandler(this, &showvdo::OnVideoTick);
  }

protected:
  ~showvdo() {
    if (video && video->isOpened())
      video->release();
    delete video;
    delete net;
    if (components)
      delete components;
  }

private:
  System::Windows::Forms::Button ^ pusebutton;
  System::Windows::Forms::Button ^ return_main;
  System::Windows::Forms::Button ^ openvdofile;
  System::Windows::Forms::PictureBox ^ pictureBox1;
  System::Windows::Forms::TextBox ^ gradeA;
  System::Windows::Forms::TextBox ^ gradeB;
  System::Windows::Forms::TextBox ^ gradeC;
  System::Windows::Forms::TextBox ^ gradeD;
  System::ComponentModel::Container ^ components;

#pragma region Windows Form Designer generated code
  void InitializeComponent(void) {
    this->gradeA = (gcnew System::Windows::Forms::TextBox());
    this->gradeB = (gcnew System::Windows::Forms::TextBox());
    this->gradeC = (gcnew System::Windows::Forms::TextBox());
    this->gradeD = (gcnew System::Windows::Forms::TextBox());
    this->pusebutton = (gcnew System::Windows::Forms::Button());
    this->return_main = (gcnew System::Windows::Forms::Button());
    this->openvdofile = (gcnew System::Windows::Forms::Button());
    this->pictureBox1 = (gcnew System::Windows::Forms::PictureBox());
    (cli::safe_cast<System::ComponentModel::ISupportInitialize ^>(
         this->pictureBox1))
        ->BeginInit();
    this->SuspendLayout();
    //
    // gradeA
    //
    this->gradeA->BackColor = System::Drawing::Color::LightGreen;
    this->gradeA->Location = System::Drawing::Point(784, 109);
    this->gradeA->Name = L"gradeA";
    this->gradeA->ReadOnly = true;
    this->gradeA->Size = System::Drawing::Size(170, 22);
    this->gradeA->TabIndex = 3;
    this->gradeA->Text = L"Grade A: 0";
    //
    // gradeB
    //
    this->gradeB->BackColor = System::Drawing::Color::LightYellow;
    this->gradeB->Location = System::Drawing::Point(784, 160);
    this->gradeB->Name = L"gradeB";
    this->gradeB->ReadOnly = true;
    this->gradeB->Size = System::Drawing::Size(170, 22);
    this->gradeB->TabIndex = 2;
    this->gradeB->Text = L"Grade B: 0";
    //
    // gradeC
    //
    this->gradeC->BackColor = System::Drawing::Color::LightSalmon;
    this->gradeC->Location = System::Drawing::Point(784, 211);
    this->gradeC->Name = L"gradeC";
    this->gradeC->ReadOnly = true;
    this->gradeC->Size = System::Drawing::Size(170, 22);
    this->gradeC->TabIndex = 1;
    this->gradeC->Text = L"Grade C: 0";
    //
    // gradeD
    //
    this->gradeD->BackColor = System::Drawing::Color::LightGray;
    this->gradeD->Location = System::Drawing::Point(784, 262);
    this->gradeD->Name = L"gradeD";
    this->gradeD->ReadOnly = true;
    this->gradeD->Size = System::Drawing::Size(170, 22);
    this->gradeD->TabIndex = 0;
    this->gradeD->Text = L"Grade D: 0";
    //
    // pusebutton
    //
    this->pusebutton->Location = System::Drawing::Point(162, 32);
    this->pusebutton->Name = L"pusebutton";
    this->pusebutton->Size = System::Drawing::Size(75, 30);
    this->pusebutton->TabIndex = 6;
    this->pusebutton->Text = L"Pause";
    this->pusebutton->Click +=
        gcnew System::EventHandler(this, &showvdo::pusebutton_Click_1);
    //
    // return_main
    //
    this->return_main->Location = System::Drawing::Point(30, 32);
    this->return_main->Name = L"return_main";
    this->return_main->Size = System::Drawing::Size(75, 30);
    this->return_main->TabIndex = 7;
    this->return_main->Text = L"Return";
    this->return_main->Click +=
        gcnew System::EventHandler(this, &showvdo::button1_Click_1);
    //
    // openvdofile
    //
    this->openvdofile->Location = System::Drawing::Point(309, 32);
    this->openvdofile->Name = L"openvdofile";
    this->openvdofile->Size = System::Drawing::Size(90, 30);
    this->openvdofile->TabIndex = 5;
    this->openvdofile->Text = L"Open VDO";
    this->openvdofile->Click +=
        gcnew System::EventHandler(this, &showvdo::openvdofile_Click);
    //
    // pictureBox1
    //
    this->pictureBox1->BackColor = System::Drawing::Color::Black;
    this->pictureBox1->Location = System::Drawing::Point(12, 84);
    this->pictureBox1->Name = L"pictureBox1";
    this->pictureBox1->Size = System::Drawing::Size(766, 538);
    this->pictureBox1->SizeMode =
        System::Windows::Forms::PictureBoxSizeMode::Zoom;
    this->pictureBox1->TabIndex = 4;
    this->pictureBox1->TabStop = false;
    //
    // showvdo
    //
    this->BackColor = System::Drawing::SystemColors::ActiveCaption;
    this->ClientSize = System::Drawing::Size(989, 634);
    this->Controls->Add(this->gradeD);
    this->Controls->Add(this->gradeC);
    this->Controls->Add(this->gradeB);
    this->Controls->Add(this->gradeA);
    this->Controls->Add(this->pictureBox1);
    this->Controls->Add(this->openvdofile);
    this->Controls->Add(this->pusebutton);
    this->Controls->Add(this->return_main);
    this->Name = L"showvdo";
    this->Text = L"Mangosteen Grading Pro";
    this->Load += gcnew System::EventHandler(this, &showvdo::showvdo_Load);
    (cli::safe_cast<System::ComponentModel::ISupportInitialize ^>(
         this->pictureBox1))
        ->EndInit();
    this->ResumeLayout(false);
    this->PerformLayout();
  }
#pragma endregion

private:
  System::Void OnVideoTick(System::Object ^ sender, EventArgs ^ e) {
    if (isPaused || !video->isOpened())
      return;

    cv::Mat frame;
    if (!video->read(frame) || frame.empty()) {
      videoTimer->Stop();
      return;
    }

    // ── YOLO inference ────────────────────────────────────────────
    cv::Mat blob =
        cv::dnn::blobFromImage(frame, 1.0 / 255.0, cv::Size(640, 640),
                               cv::Scalar(0, 0, 0), true, false);
    net->setInput(blob);

    std::vector<cv::Mat> outputs;
    net->forward(outputs, net->getUnconnectedOutLayersNames());

    auto detections = YoloParser::parse(outputs, frame.cols, frame.rows);

    for (const auto &det : detections) {
      // ── Un_Ripe (classId==1) → Grade D ทันที ─────────────────
      if (det.classId == 1) {
        countD++;
        gradeD->Text = "Grade D: " + countD.ToString();
        cv::rectangle(frame, det.box, cv::Scalar(128, 128, 128), 2);
        cv::putText(frame, "D Unripe", cv::Point(det.box.x, det.box.y - 6),
                    cv::FONT_HERSHEY_SIMPLEX, 0.6, cv::Scalar(128, 128, 128),
                    2);
        char conf[40];
        snprintf(conf, sizeof(conf), "conf:%.2f", det.confidence);
        cv::putText(
            frame, conf, cv::Point(det.box.x, det.box.y + det.box.height + 16),
            cv::FONT_HERSHEY_SIMPLEX, 0.40, cv::Scalar(160, 160, 160), 1);
        continue;
      }

      // ── Ripe (classId==0): ตรวจสอบขนาด box ──────────────────
      cv::Rect safeBox = det.box & cv::Rect(0, 0, frame.cols, frame.rows);
      if (safeBox.width < Thresholds::MIN_BOX_SIZE ||
          safeBox.height < Thresholds::MIN_BOX_SIZE)
        continue;

      // ── Crop เฉพาะใน bounding box ────────────────────────────
      cv::Mat roi = frame(safeBox).clone();

      // ── Pipeline 3 ขั้น พร้อมส่ง confidence ─────────────────
      MangosteenAnalyzer::Result res =
          MangosteenAnalyzer::analyze(roi, det.confidence);

      // ── ไม่ผ่าน Guard → วาดกรอบสีเหลืองบอก SKIP ─────────────
      if (!res.isValid) {
        cv::rectangle(frame, det.box, cv::Scalar(0, 200, 255), 1);
        cv::putText(frame, res.debugText, cv::Point(det.box.x, det.box.y - 6),
                    cv::FONT_HERSHEY_SIMPLEX, 0.40, cv::Scalar(0, 200, 255), 1);
        continue;
      }

      // ── นับเกรด ───────────────────────────────────────────────
      switch (res.grade) {
      case 'A':
        countA++;
        gradeA->Text = "Grade A: " + countA.ToString();
        break;
      case 'B':
        countB++;
        gradeB->Text = "Grade B: " + countB.ToString();
        break;
      default:
        countC++;
        gradeC->Text = "Grade C: " + countC.ToString();
        break;
      }

      // ── เลือกสี box ───────────────────────────────────────────
      cv::Scalar boxColor;
      std::string label;
      switch (res.grade) {
      case 'A':
        boxColor = cv::Scalar(0, 255, 0);
        label = "Grade:A";
        break;
      case 'B':
        boxColor = cv::Scalar(0, 255, 255);
        label = "Grade:B";
        break;
      default:
        boxColor = cv::Scalar(0, 0, 255);
        label = "Grade:C";
        break;
      }

      cv::rectangle(frame, det.box, boxColor, 2);

      // ── บน box: Grade + confidence ────────────────────────────
      char topText[60];
      snprintf(topText, sizeof(topText), "%s conf:%.2f", label.c_str(),
               det.confidence);
      cv::putText(frame, topText, cv::Point(det.box.x, det.box.y - 6),
                  cv::FONT_HERSHEY_SIMPLEX, 0.55, boxColor, 2);

      // ── ใต้ box: ค่า Threshold แต่ละขั้น ─────────────────────
      char line1[80];
      snprintf(line1, sizeof(line1), "Grn:%.0f%% Pnk:%.0f%% Red:%.0f%%",
               res.stagePct[0], res.stagePct[1], res.stagePct[2]);
      cv::putText(frame, line1,
                  cv::Point(det.box.x, det.box.y + det.box.height + 14),
                  cv::FONT_HERSHEY_SIMPLEX, 0.38, boxColor, 1);

      char line2[80];
      snprintf(line2, sizeof(line2), "Pur:%.0f%% Blk:%.0f%% (S%d)",
               res.stagePct[3], res.stagePct[4], res.dominantStage);
      cv::putText(frame, line2,
                  cv::Point(det.box.x, det.box.y + det.box.height + 28),
                  cv::FONT_HERSHEY_SIMPLEX, 0.38, boxColor, 1);
    }

    // ── แสดงผล frame ──────────────────────────────────────────────
    // ยกเลิกการแปลงสี BGR2RGB เพื่อแก้ปัญหาหน้าคน/วิดีโอเป็นสีฟ้า
    System::Drawing::Bitmap ^ bmp = gcnew System::Drawing::Bitmap(
        frame.cols, frame.rows,
        System::Drawing::Imaging::PixelFormat::Format24bppRgb);

    System::Drawing::Rectangle rect(0, 0, frame.cols, frame.rows);
    System::Drawing::Imaging::BitmapData ^ bmpData =
        bmp->LockBits(rect, System::Drawing::Imaging::ImageLockMode::WriteOnly,
                      System::Drawing::Imaging::PixelFormat::Format24bppRgb);

    memcpy((unsigned char *)bmpData->Scan0.ToPointer(), frame.data,
           frame.total() * frame.elemSize());
    bmp->UnlockBits(bmpData);

    System::Drawing::Image ^ oldImg = this->pictureBox1->Image;
    this->pictureBox1->Image = bmp;
    if (oldImg != nullptr)
      delete oldImg;
  }

private:
  System::Void openvdofile_Click(System::Object ^ sender, EventArgs ^ e) {
    OpenFileDialog ^ ofd = gcnew OpenFileDialog();
    ofd->Filter = "Video Files|*.mp4;*.avi;*.mov;*.mkv|All Files|*.*";
    if (ofd->ShowDialog() == System::Windows::Forms::DialogResult::OK) {
      selectedPath = ofd->FileName;

      // การแปลง String^ ของ C# ไปเป็น std::string ของ C++
      System::IntPtr ptr =
          System::Runtime::InteropServices::Marshal::StringToHGlobalAnsi(
              selectedPath);
      std::string path = static_cast<char *>(ptr.ToPointer());
      System::Runtime::InteropServices::Marshal::FreeHGlobal(ptr);

      if (video->isOpened())
        video->release();
      video->open(path);

      if (video->isOpened()) {
        countA = countB = countC = countD = 0;
        gradeA->Text = "Grade A: 0";
        gradeB->Text = "Grade B: 0";
        gradeC->Text = "Grade C: 0";
        gradeD->Text = "Grade D: 0";
        isPaused = false;
        pusebutton->Text = L"Pause";
        videoTimer->Start();
      } else {
        MessageBox::Show("ไม่สามารถเปิดไฟล์วิดีโอได้\nลองใช้ไฟล์ .mp4 หรือ .avi");
      }
    }
  }

private:
  System::Void pusebutton_Click_1(System::Object ^ sender, EventArgs ^ e) {
    if (!video->isOpened())
      return;
    isPaused = !isPaused;
    pusebutton->Text = isPaused ? L"Play" : L"Pause";

    // เพิ่มการ Start/Stop videoTimer เพื่อให้วิดีโอหยุดจริงและเล่นต่อได้
    if (isPaused) {
      videoTimer->Stop();
    } else {
      videoTimer->Start();
    }
  }

private:
  System::Void button1_Click_1(System::Object ^ sender, EventArgs ^ e) {
    videoTimer->Stop();
    if (video->isOpened())
      video->release();
    this->Close();
  }

private:
  System::Void showvdo_Load(System::Object ^ sender, System::EventArgs ^ e) {}
};

} // namespace Mangkudd