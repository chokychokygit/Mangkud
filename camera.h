#pragma once
// ================================================================
//  camera.h  (v2)
//  เปลี่ยนแปลง:
//  - ส่ง det.confidence เข้า MangosteenAnalyzer::analyze()
//  - ข้าม detection ที่ grade == '?' (ไม่ผ่าน guard)
//  - วาด debug threshold text ใต้ bounding box
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

	public:
		camera(void)
		{
			InitializeComponent();
			cap = new cv::VideoCapture();
			try {
				net = new cv::dnn::Net(cv::dnn::readNetFromONNX("C:\\Users\\ASUS\\Downloads\\best.onnx"));
				net->setPreferableBackend(cv::dnn::DNN_BACKEND_OPENCV);
				net->setPreferableTarget(cv::dnn::DNN_TARGET_CPU);
			}
			catch (...) {
				MessageBox::Show("ไม่พบไฟล์ model ที่ C:\\Users\\ASUS\\Downloads\\best.onnx\nกรุณาตรวจสอบ path ของไฟล์");
			}
			cameraTimer = gcnew System::Windows::Forms::Timer();
			cameraTimer->Interval = 33;
			cameraTimer->Tick += gcnew EventHandler(this, &camera::OnCameraTick);
		}

	protected:
		~camera() {
			if (cap && cap->isOpened()) cap->release();
			delete cap;
			delete net;
			if (components) delete components;
		}

	private:
		System::Windows::Forms::SplitContainer^ splitContainer1;
		System::Windows::Forms::PictureBox^ camerashow;
		System::Windows::Forms::Button^ startbutton;
		System::Windows::Forms::Button^ button1;
		System::Windows::Forms::Label^ lblGradeA;
		System::Windows::Forms::Label^ lblGradeB;
		System::Windows::Forms::Label^ lblGradeC;
		System::Windows::Forms::Label^ lblGradeD;
		System::Windows::Forms::Button^ btnReset;
		System::ComponentModel::Container^ components;

#pragma region Windows Form Designer generated code
		void InitializeComponent(void)
		{
			this->splitContainer1 = (gcnew System::Windows::Forms::SplitContainer());
			this->camerashow = (gcnew System::Windows::Forms::PictureBox());
			this->button1 = (gcnew System::Windows::Forms::Button());
			this->startbutton = (gcnew System::Windows::Forms::Button());
			this->lblGradeA = (gcnew System::Windows::Forms::Label());
			this->lblGradeB = (gcnew System::Windows::Forms::Label());
			this->lblGradeC = (gcnew System::Windows::Forms::Label());
			this->lblGradeD = (gcnew System::Windows::Forms::Label());
			this->btnReset = (gcnew System::Windows::Forms::Button());
			(cli::safe_cast<System::ComponentModel::ISupportInitialize^>(this->splitContainer1))->BeginInit();
			this->splitContainer1->Panel1->SuspendLayout();
			this->splitContainer1->Panel2->SuspendLayout();
			this->splitContainer1->SuspendLayout();
			(cli::safe_cast<System::ComponentModel::ISupportInitialize^>(this->camerashow))->BeginInit();
			this->SuspendLayout();

			this->splitContainer1->Dock = System::Windows::Forms::DockStyle::Fill;
			this->splitContainer1->Location = System::Drawing::Point(0, 0);
			this->splitContainer1->Name = L"splitContainer1";
			this->splitContainer1->Size = System::Drawing::Size(1019, 669);
			this->splitContainer1->SplitterDistance = 804;
			this->splitContainer1->TabIndex = 0;
			this->splitContainer1->Panel1->Controls->Add(this->camerashow);
			this->splitContainer1->Panel2->BackColor = System::Drawing::SystemColors::ActiveCaption;
			this->splitContainer1->Panel2->Controls->Add(this->lblGradeA);
			this->splitContainer1->Panel2->Controls->Add(this->lblGradeB);
			this->splitContainer1->Panel2->Controls->Add(this->lblGradeC);
			this->splitContainer1->Panel2->Controls->Add(this->lblGradeD);
			this->splitContainer1->Panel2->Controls->Add(this->btnReset);
			this->splitContainer1->Panel2->Controls->Add(this->button1);
			this->splitContainer1->Panel2->Controls->Add(this->startbutton);

			this->camerashow->BackColor = System::Drawing::Color::Black;
			this->camerashow->Dock = System::Windows::Forms::DockStyle::Fill;
			this->camerashow->SizeMode = System::Windows::Forms::PictureBoxSizeMode::Zoom;
			this->camerashow->Name = L"camerashow";
			this->camerashow->TabStop = false;

			this->startbutton->Location = System::Drawing::Point(25, 20);
			this->startbutton->Size = System::Drawing::Size(161, 50);
			this->startbutton->Text = L"START";
			this->startbutton->Font = (gcnew System::Drawing::Font(L"Microsoft YaHei", 12.0F, System::Drawing::FontStyle::Bold));
			this->startbutton->BackColor = System::Drawing::Color::LightGreen;
			this->startbutton->Click += gcnew System::EventHandler(this, &camera::startbutton_Click);

			this->btnReset->Location = System::Drawing::Point(25, 80);
			this->btnReset->Size = System::Drawing::Size(161, 40);
			this->btnReset->Text = L"RESET COUNT";
			this->btnReset->BackColor = System::Drawing::Color::LightYellow;
			this->btnReset->Click += gcnew System::EventHandler(this, &camera::btnReset_Click);

			this->lblGradeA->Location = System::Drawing::Point(10, 150);
			this->lblGradeA->Size = System::Drawing::Size(190, 40);
			this->lblGradeA->Text = L"Grade A: 0";
			this->lblGradeA->Font = (gcnew System::Drawing::Font(L"Microsoft YaHei", 13.0F, System::Drawing::FontStyle::Bold));
			this->lblGradeA->ForeColor = System::Drawing::Color::DarkGreen;
			this->lblGradeA->BackColor = System::Drawing::Color::Honeydew;
			this->lblGradeA->TextAlign = System::Drawing::ContentAlignment::MiddleCenter;
			this->lblGradeA->BorderStyle = System::Windows::Forms::BorderStyle::FixedSingle;

			this->lblGradeB->Location = System::Drawing::Point(10, 200);
			this->lblGradeB->Size = System::Drawing::Size(190, 40);
			this->lblGradeB->Text = L"Grade B: 0";
			this->lblGradeB->Font = (gcnew System::Drawing::Font(L"Microsoft YaHei", 13.0F, System::Drawing::FontStyle::Bold));
			this->lblGradeB->ForeColor = System::Drawing::Color::DarkGoldenrod;
			this->lblGradeB->BackColor = System::Drawing::Color::LightYellow;
			this->lblGradeB->TextAlign = System::Drawing::ContentAlignment::MiddleCenter;
			this->lblGradeB->BorderStyle = System::Windows::Forms::BorderStyle::FixedSingle;

			this->lblGradeC->Location = System::Drawing::Point(10, 250);
			this->lblGradeC->Size = System::Drawing::Size(190, 40);
			this->lblGradeC->Text = L"Grade C: 0";
			this->lblGradeC->Font = (gcnew System::Drawing::Font(L"Microsoft YaHei", 13.0F, System::Drawing::FontStyle::Bold));
			this->lblGradeC->ForeColor = System::Drawing::Color::DarkRed;
			this->lblGradeC->BackColor = System::Drawing::Color::MistyRose;
			this->lblGradeC->TextAlign = System::Drawing::ContentAlignment::MiddleCenter;
			this->lblGradeC->BorderStyle = System::Windows::Forms::BorderStyle::FixedSingle;

			this->lblGradeD->Location = System::Drawing::Point(10, 300);
			this->lblGradeD->Size = System::Drawing::Size(190, 40);
			this->lblGradeD->Text = L"Grade D: 0";
			this->lblGradeD->Font = (gcnew System::Drawing::Font(L"Microsoft YaHei", 13.0F, System::Drawing::FontStyle::Bold));
			this->lblGradeD->ForeColor = System::Drawing::Color::Gray;
			this->lblGradeD->BackColor = System::Drawing::Color::WhiteSmoke;
			this->lblGradeD->TextAlign = System::Drawing::ContentAlignment::MiddleCenter;
			this->lblGradeD->BorderStyle = System::Windows::Forms::BorderStyle::FixedSingle;

			this->button1->Font = (gcnew System::Drawing::Font(L"Microsoft YaHei", 12.0F, System::Drawing::FontStyle::Bold));
			this->button1->Location = System::Drawing::Point(25, 580);
			this->button1->Size = System::Drawing::Size(161, 50);
			this->button1->Text = L"RETURN";
			this->button1->BackColor = System::Drawing::Color::LightCoral;
			this->button1->Click += gcnew System::EventHandler(this, &camera::button1_Click);

			this->ClientSize = System::Drawing::Size(1019, 669);
			this->Controls->Add(this->splitContainer1);
			this->Name = L"camera";
			this->Text = L"Camera Grading System";
			this->Load += gcnew System::EventHandler(this, &camera::camera_Load);
			this->splitContainer1->Panel1->ResumeLayout(false);
			this->splitContainer1->Panel2->ResumeLayout(false);
			(cli::safe_cast<System::ComponentModel::ISupportInitialize^>(this->splitContainer1))->EndInit();
			this->splitContainer1->ResumeLayout(false);
			(cli::safe_cast<System::ComponentModel::ISupportInitialize^>(this->camerashow))->EndInit();
			this->ResumeLayout(false);
		}
#pragma endregion

	private: System::Void OnCameraTick(System::Object^ sender, System::EventArgs^ e)
	{
		if (!isCameraRunning || !cap || !cap->isOpened()) return;

		cv::Mat frame;
		if (!cap->read(frame) || frame.empty()) return;

		// ── ไม่มี model: แสดง frame ดิบ ──────────────────────────────
		if (!net) {
			displayFrame(frame);
			return;
		}

		// ── YOLO inference ────────────────────────────────────────────
		cv::Mat blob = cv::dnn::blobFromImage(
			frame, 1.0 / 255.0, cv::Size(640, 640),
			cv::Scalar(0, 0, 0), true, false);
		net->setInput(blob);

		std::vector<cv::Mat> outputs;
		net->forward(outputs, net->getUnconnectedOutLayersNames());

		auto detections = YoloParser::parse(outputs, frame.cols, frame.rows);

		for (const auto& det : detections)
		{
			// ── Un_Ripe (classId==1) → Grade D ทันที ─────────────────
			if (det.classId == 1) {
				countD++;
				lblGradeD->Text = "Grade D: " + countD.ToString();
				cv::rectangle(frame, det.box, cv::Scalar(128, 128, 128), 2);
				cv::putText(frame, "D Unripe",
					cv::Point(det.box.x, det.box.y - 6),
					cv::FONT_HERSHEY_SIMPLEX, 0.6,
					cv::Scalar(128, 128, 128), 2);
				// แสดง confidence ด้านล่าง box
				char conf[40];
				snprintf(conf, sizeof(conf), "conf:%.2f", det.confidence);
				cv::putText(frame, conf,
					cv::Point(det.box.x, det.box.y + det.box.height + 16),
					cv::FONT_HERSHEY_SIMPLEX, 0.40,
					cv::Scalar(160, 160, 160), 1);
				continue;
			}

			// ── Ripe (classId==0): ตรวจสอบขนาด box ──────────────────
			cv::Rect safeBox = det.box & cv::Rect(0, 0, frame.cols, frame.rows);
			if (safeBox.width < Thresholds::MIN_BOX_SIZE ||
				safeBox.height < Thresholds::MIN_BOX_SIZE) continue;

			// ── Crop เฉพาะใน bounding box ────────────────────────────
			cv::Mat roi = frame(safeBox).clone();

			// ── Pipeline 3 ขั้น (Guard → Step1 → Step2 → Step3) ──────
			// ส่ง confidence เข้าไปด้วยเพื่อให้ Guard ตรวจ
			MangosteenAnalyzer::Result res =
				MangosteenAnalyzer::analyze(roi, det.confidence);

			// ── ไม่ผ่าน Guard → วาดกรอบสีเหลืองบอก SKIP ─────────────
			if (!res.isValid) {
				cv::rectangle(frame, det.box, cv::Scalar(0, 200, 255), 1);
				cv::putText(frame, res.debugText,
					cv::Point(det.box.x, det.box.y - 6),
					cv::FONT_HERSHEY_SIMPLEX, 0.40,
					cv::Scalar(0, 200, 255), 1);
				continue;
			}

			// ── นับเกรด ───────────────────────────────────────────────
			switch (res.grade) {
			case 'A': countA++; lblGradeA->Text = "Grade A: " + countA.ToString(); break;
			case 'B': countB++; lblGradeB->Text = "Grade B: " + countB.ToString(); break;
			default:  countC++; lblGradeC->Text = "Grade C: " + countC.ToString(); break;
			}

			// ── เลือกสี box ───────────────────────────────────────────
			cv::Scalar boxColor;
			std::string label;
			switch (res.grade) {
			case 'A': boxColor = cv::Scalar(0, 255, 0);   label = "Grade:A"; break;
			case 'B': boxColor = cv::Scalar(0, 255, 255); label = "Grade:B"; break;
			default:  boxColor = cv::Scalar(0, 0, 255);   label = "Grade:C"; break;
			}

			cv::rectangle(frame, det.box, boxColor, 2);

			// ── บน box: Grade + confidence ────────────────────────────
			char topText[60];
			snprintf(topText, sizeof(topText), "%s conf:%.2f",
				label.c_str(), det.confidence);
			cv::putText(frame, topText,
				cv::Point(det.box.x, det.box.y - 6),
				cv::FONT_HERSHEY_SIMPLEX, 0.55, boxColor, 2);

			// ── ใต้ box: ค่า Threshold แต่ละขั้น ────────────────────
			// บรรทัดที่ 1: Ripeness + Roughness + Blemish
			char line1[80];
			snprintf(line1, sizeof(line1),
				"Ripe:%.2f(>0.5) Rgh:%.1f(<%.0f)",
				res.ripenessScore,
				res.roughness,
				(res.grade == 'A') ? Thresholds::ROUGHNESS_A : Thresholds::ROUGHNESS_B);
			cv::putText(frame, line1,
				cv::Point(det.box.x, det.box.y + det.box.height + 14),
				cv::FONT_HERSHEY_SIMPLEX, 0.38, boxColor, 1);

			// บรรทัดที่ 2: Blemish + FinalScore
			char line2[80];
			snprintf(line2, sizeof(line2),
				"Blm:%.1f%%(<%.0f%%) Sc:%.1f(>%.0f)",
				res.blemishPercent,
				(res.grade == 'A') ? Thresholds::BLEMISH_PCT_A : Thresholds::BLEMISH_PCT_B,
				res.finalScore,
				(res.grade == 'A') ? Thresholds::SCORE_A : Thresholds::SCORE_B);
			cv::putText(frame, line2,
				cv::Point(det.box.x, det.box.y + det.box.height + 28),
				cv::FONT_HERSHEY_SIMPLEX, 0.38, boxColor, 1);
		}

		displayFrame(frame);
	}

		   // ── Helper: แปลง Mat → Bitmap → PictureBox ─────────────────────
		   void displayFrame(cv::Mat& frame) {
			   cv::Mat rgb;
			   cv::cvtColor(frame, rgb, cv::COLOR_BGR2RGB);

			   System::Drawing::Bitmap^ bmp = gcnew System::Drawing::Bitmap(
				   rgb.cols, rgb.rows,
				   System::Drawing::Imaging::PixelFormat::Format24bppRgb);

			   System::Drawing::Rectangle rect(0, 0, rgb.cols, rgb.rows);
			   System::Drawing::Imaging::BitmapData^ bmpData =
				   bmp->LockBits(rect,
					   System::Drawing::Imaging::ImageLockMode::WriteOnly,
					   System::Drawing::Imaging::PixelFormat::Format24bppRgb);

			   memcpy((unsigned char*)bmpData->Scan0.ToPointer(),
				   rgb.data, rgb.total() * rgb.elemSize());
			   bmp->UnlockBits(bmpData);

			   System::Drawing::Image^ oldImg = this->camerashow->Image;
			   this->camerashow->Image = bmp;
			   if (oldImg != nullptr) delete oldImg;
		   }

	private: System::Void startbutton_Click(System::Object^ sender, System::EventArgs^ e)
	{
		if (!isCameraRunning) {
			if (!cap) return;
			if (cap->isOpened()) cap->release();
			if (cap->open(0)) {
				isCameraRunning = true;
				cameraTimer->Start();
				startbutton->Text = L"STOP";
				startbutton->BackColor = System::Drawing::Color::Tomato;
			}
			else {
				MessageBox::Show("ไม่สามารถเปิดกล้องได้\nกรุณาตรวจสอบการเชื่อมต่อกล้อง");
			}
		}
		else {
			isCameraRunning = false;
			cameraTimer->Stop();
			if (cap && cap->isOpened()) cap->release();
			System::Drawing::Image^ oldImg = this->camerashow->Image;
			this->camerashow->Image = nullptr;
			if (oldImg != nullptr) delete oldImg;
			startbutton->Text = L"START";
			startbutton->BackColor = System::Drawing::Color::LightGreen;
		}
	}

	private: System::Void btnReset_Click(System::Object^ sender, System::EventArgs^ e)
	{
		countA = countB = countC = countD = 0;
		lblGradeA->Text = "Grade A: 0";
		lblGradeB->Text = "Grade B: 0";
		lblGradeC->Text = "Grade C: 0";
		lblGradeD->Text = "Grade D: 0";
	}

	private: System::Void button1_Click(System::Object^ sender, System::EventArgs^ e)
	{
		if (isCameraRunning) {
			isCameraRunning = false;
			cameraTimer->Stop();
			if (cap && cap->isOpened()) cap->release();
		}
		System::Drawing::Image^ oldImg = this->camerashow->Image;
		this->camerashow->Image = nullptr;
		if (oldImg != nullptr) delete oldImg;
		this->Close();
	}

	private: System::Void camera_Load(System::Object^ sender, System::EventArgs^ e)
	{
		this->splitContainer1->BringToFront();
	}
	};

} // namespace Mangkudd