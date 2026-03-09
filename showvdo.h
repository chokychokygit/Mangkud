#pragma once
// ================================================================
//  showvdo.h  (Refactored for YOLO ZeroMQ Integration)
// ================================================================
#include "mangosteen_core.h"

namespace Mangkudd {

using namespace System;
using namespace System::Windows::Forms;
using namespace System::Drawing;
using namespace System::ComponentModel;

public ref class showvdo : public System::Windows::Forms::Form {
private:
  cv::VideoCapture *video;
  AiClient^ aiClient;

  System::Windows::Forms::Timer ^ videoTimer;
  System::String ^ selectedPath = "";
  bool isPaused = false;

  int countA = 0, countB = 0, countC = 0, countD = 0;
private: System::Windows::Forms::Button^ button1;
private: System::Windows::Forms::Button^ importcsv2;
       int frameCounter = 0;

public:
  showvdo(void) {
    InitializeComponent();
    video = new cv::VideoCapture();
    aiClient = gcnew AiClient();
    
    // Connect to Python AI Server
    if (!aiClient->Connect("tcp://localhost:5555")) {
        MessageBox::Show(L"ไม่สามารถเชื่อมต่อกับ Python AI Server ได้\nกรุณาเปิดไฟล์ ai_server.py ทิ้งไว้", L"ข้อผิดพลาด", MessageBoxButtons::OK, MessageBoxIcon::Error);
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
    
    if (aiClient != nullptr) {
        aiClient->Disconnect();
    }

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
      this->button1 = (gcnew System::Windows::Forms::Button());
      this->importcsv2 = (gcnew System::Windows::Forms::Button());
      (cli::safe_cast<System::ComponentModel::ISupportInitialize^>(this->pictureBox1))->BeginInit();
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
      this->pusebutton->Click += gcnew System::EventHandler(this, &showvdo::pusebutton_Click_1);
      // 
      // return_main
      // 
      this->return_main->Location = System::Drawing::Point(30, 32);
      this->return_main->Name = L"return_main";
      this->return_main->Size = System::Drawing::Size(75, 30);
      this->return_main->TabIndex = 7;
      this->return_main->Text = L"Return";
      this->return_main->Click += gcnew System::EventHandler(this, &showvdo::button1_Click_1);
      // 
      // openvdofile
      // 
      this->openvdofile->Location = System::Drawing::Point(309, 32);
      this->openvdofile->Name = L"openvdofile";
      this->openvdofile->Size = System::Drawing::Size(90, 30);
      this->openvdofile->TabIndex = 5;
      this->openvdofile->Text = L"Open VDO";
      this->openvdofile->Click += gcnew System::EventHandler(this, &showvdo::openvdofile_Click);
      // 
      // pictureBox1
      // 
      this->pictureBox1->BackColor = System::Drawing::Color::Black;
      this->pictureBox1->Location = System::Drawing::Point(12, 84);
      this->pictureBox1->Name = L"pictureBox1";
      this->pictureBox1->Size = System::Drawing::Size(766, 538);
      this->pictureBox1->SizeMode = System::Windows::Forms::PictureBoxSizeMode::Zoom;
      this->pictureBox1->TabIndex = 4;
      this->pictureBox1->TabStop = false;
      // 
      // button1
      // 
      this->button1->Location = System::Drawing::Point(442, 32);
      this->button1->Name = L"button1";
      this->button1->Size = System::Drawing::Size(90, 30);
      this->button1->TabIndex = 8;
      this->button1->Text = L"ImportDATA";
      // 
      // importcsv2
      // 
      this->importcsv2->Location = System::Drawing::Point(600, 32);
      this->importcsv2->Name = L"importcsv2";
      this->importcsv2->Size = System::Drawing::Size(100, 30);
      this->importcsv2->TabIndex = 9;
      this->importcsv2->Text = L"SAVE CSV";
      this->importcsv2->BackColor = System::Drawing::Color::LightSkyBlue;
      this->importcsv2->Click += gcnew System::EventHandler(this, &showvdo::importcsv2_Click);
      // 
      // showvdo
      // 
      this->BackColor = System::Drawing::SystemColors::ActiveCaption;
      this->ClientSize = System::Drawing::Size(989, 634);
      this->Controls->Add(this->button1);
      this->Controls->Add(this->importcsv2);
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
      (cli::safe_cast<System::ComponentModel::ISupportInitialize^>(this->pictureBox1))->EndInit();
      this->ResumeLayout(false);
      this->PerformLayout();

  }
#pragma endregion

private:
  void drawDetections(cv::Mat& frame, const std::vector<AiResult>& results) {
    for (const auto &res : results) {
      cv::Scalar boxColor;
      std::string label;
      
      switch (res.mappedGrade) {
      case 'A': boxColor = cv::Scalar(0, 255, 0); label = "Grade A (" + res.grade + ")"; break;
      case 'B': boxColor = cv::Scalar(0, 255, 255); label = "Grade B (" + res.grade + ")"; break;
      case 'C': boxColor = cv::Scalar(0, 165, 255); label = "Grade C (" + res.grade + ")"; break;
      case 'D': boxColor = cv::Scalar(128, 128, 128); label = "Grade D (" + res.grade + ")"; break;
      default:  boxColor = cv::Scalar(0, 0, 255); label = "Unknown"; break;
      }

      // Draw bounding box
      cv::rectangle(frame, res.box, boxColor, 2);

      // Draw Label
      char displayTxt[100];
      snprintf(displayTxt, sizeof(displayTxt), "ID:%d %s", res.id, label.c_str());
      
      cv::putText(frame, displayTxt, cv::Point(res.box.x, (std::max)(20, res.box.y - 10)),
                  cv::FONT_HERSHEY_SIMPLEX, 0.7, boxColor, 2);
    }
  }

private:
  void displayFrame(cv::Mat& frame) {
    // GDI+ Format24bppRgb expects BGR bytes in memory, so we keep it BGR
    cv::Mat bgr;
    if (frame.channels() == 3) {
      bgr = frame;
    } else if (frame.channels() == 4) {
      cv::cvtColor(frame, bgr, cv::COLOR_BGRA2BGR);
    } else {
      cv::cvtColor(frame, bgr, cv::COLOR_GRAY2BGR);
    }

    System::Drawing::Bitmap ^ bmp = gcnew System::Drawing::Bitmap(
        bgr.cols, bgr.rows,
        System::Drawing::Imaging::PixelFormat::Format24bppRgb);

    System::Drawing::Rectangle rect(0, 0, bgr.cols, bgr.rows);
    System::Drawing::Imaging::BitmapData ^ bmpData =
        bmp->LockBits(rect, System::Drawing::Imaging::ImageLockMode::WriteOnly,
                      System::Drawing::Imaging::PixelFormat::Format24bppRgb);

    unsigned char *dest = (unsigned char *)bmpData->Scan0.ToPointer();
    const unsigned char *src = bgr.data;
    int destStride = bmpData->Stride;
    size_t srcStep = bgr.step[0];
    int rowBytes = bgr.cols * bgr.elemSize();

    if (rowBytes == destStride) {
      memcpy(dest, src, (size_t)rowBytes * bgr.rows);
    } else {
      for (int y = 0; y < bgr.rows; y++) {
        memcpy(dest + (size_t)y * destStride, src + (size_t)y * srcStep,
               (size_t)rowBytes);
      }
    }
    bmp->UnlockBits(bmpData);

    System::Drawing::Image ^ oldImg = this->pictureBox1->Image;
    this->pictureBox1->Image = bmp;
    if (oldImg != nullptr)
      delete oldImg;
  }
  std::vector<AiResult>* lastResults = new std::vector<AiResult>();

  System::Void OnVideoTick(System::Object ^ sender, EventArgs ^ e) {
    if (isPaused || !video->isOpened())
      return;

    cv::Mat frame;
    if (!video->read(frame) || frame.empty()) {
      videoTimer->Stop();
      return;
    }

    frameCounter++;
    
    // SEND frame async continuously
    if (frameCounter % 2 == 0) {
        aiClient->SendFrameAsync(frame);
    }

    // RECEIVE results non-blockingly
    std::vector<AiResult> currentResults;
    if (aiClient->TryReceiveResult(currentResults, countA, countB, countC, countD, frame.cols)) {
        *lastResults = currentResults;
        
        // Update UI Labels 
        gradeA->Text = "Grade A: " + countA.ToString();
        gradeB->Text = "Grade B: " + countB.ToString();
        gradeC->Text = "Grade C: " + countC.ToString();
        gradeD->Text = "Grade D: " + countD.ToString();
    }
    
    // Always draw bounding boxes of last known state
    drawDetections(frame, *lastResults);

    // resize ให้พอดี PictureBox แล้วแสดงด้วย displayFrame()
    cv::Mat disp;
    if (this->pictureBox1->Width > 0 && this->pictureBox1->Height > 0)
      cv::resize(frame, disp, cv::Size(this->pictureBox1->Width, this->pictureBox1->Height));
    else
      disp = frame;
    displayFrame(disp);
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
        frameCounter = 0;
        gradeA->Text = "Grade A: 0";
        gradeB->Text = "Grade B: 0";
        gradeC->Text = "Grade C: 0";
        gradeD->Text = "Grade D: 0";
        isPaused = false;
        pusebutton->Text = L"Pause";
        videoTimer->Start();
      } else {
        MessageBox::Show(L"ไม่สามารถเปิดไฟล์วิดีโอได้\nลองใช้ไฟล์ .mp4 หรือ .avi", L"ข้อผิดพลาด", MessageBoxButtons::OK, MessageBoxIcon::Error);
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
  System::Void importcsv2_Click(System::Object ^ sender, System::EventArgs ^ e) {
    System::Windows::Forms::SaveFileDialog^ saveFileDialog1 = gcnew System::Windows::Forms::SaveFileDialog();
    saveFileDialog1->Filter = "CSV File|*.csv";
    saveFileDialog1->Title = "Save Grade Log";
    saveFileDialog1->FileName = "Mangkudd_DataLog.csv";
    
    if (saveFileDialog1->ShowDialog() == System::Windows::Forms::DialogResult::OK) {
        System::String^ timestamp = System::DateTime::Now.ToString("yyyy-MM-dd HH:mm:ss");
        System::String^ csvData = timestamp + "," + countA + "," + countB + "," + countC + "," + countD + "\n";
        
        System::String^ filePath = saveFileDialog1->FileName;
        
        if (!System::IO::File::Exists(filePath)) {
            System::IO::File::WriteAllText(filePath, "Timestamp,Grade A,Grade B,Grade C,Grade D\n", System::Text::Encoding::UTF8);
        }
        
        System::IO::File::AppendAllText(filePath, csvData, System::Text::Encoding::UTF8);
        MessageBox::Show(L"Data exported successfully!", L"Success", MessageBoxButtons::OK, MessageBoxIcon::Information);
    }
  }

private:
  System::Void showvdo_Load(System::Object ^ sender, EventArgs ^ e) {
    // โหลดฟอร์มแล้ว ถ้ามีเตรียมการอะไรเขียนที่นี่ได้
  }
};

} // namespace Mangkudd
