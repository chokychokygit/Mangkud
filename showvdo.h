#pragma once
// ================================================================
//  showvdo.h
//  MangosteenAnalyzer และ YoloParser อยู่ใน camera.h แล้ว
//  include camera.h เพื่อใช้ร่วมกัน ไม่ define ซ้ำ
// ================================================================
#include "camera.h"

namespace Mangkudd {

	using namespace System;
	using namespace System::Windows::Forms;
	using namespace System::Drawing;
	using namespace System::ComponentModel;

	public ref class showvdo : public System::Windows::Forms::Form
	{
	private:
		cv::VideoCapture* video;
		cv::dnn::Net* net;

		System::Windows::Forms::Timer^ videoTimer;
		System::String^ selectedPath = "";
		bool             isPaused = false;

		int countA = 0, countB = 0, countC = 0, countD = 0;

	public:
		showvdo(void)
		{
			InitializeComponent();
			video = new cv::VideoCapture();
			try {
				net = new cv::dnn::Net(cv::dnn::readNetFromONNX("best.onnx"));
				net->setPreferableBackend(cv::dnn::DNN_BACKEND_OPENCV);
				net->setPreferableTarget(cv::dnn::DNN_TARGET_CPU);
			}
			catch (...) {
				MessageBox::Show("ไม่พบไฟล์ best.onnx\nกรุณาวางไฟล์ไว้ในโฟลเดอร์เดียวกับ .exe");
			}
			videoTimer = gcnew System::Windows::Forms::Timer();
			videoTimer->Interval = 33;
			videoTimer->Tick += gcnew EventHandler(this, &showvdo::OnVideoTick);
		}

	protected:
		~showvdo() {
			if (video && video->isOpened()) video->release();
			delete video;
			delete net;
			if (components) delete components;
		}

	private:
		System::Windows::Forms::Button^ pusebutton;
		System::Windows::Forms::Button^ return_main;
		System::Windows::Forms::Button^ openvdofile;
		System::Windows::Forms::PictureBox^ pictureBox1;
		System::Windows::Forms::TextBox^ gradeA;
		System::Windows::Forms::TextBox^ gradeB;
		System::Windows::Forms::TextBox^ gradeC;
		System::Windows::Forms::TextBox^ gradeD;
		System::ComponentModel::Container^ components;

#pragma region Windows Form Designer generated code
		void InitializeComponent(void)
		{
			this->gradeA = (gcnew System::Windows::Forms::TextBox());
			this->gradeB = (gcnew System::Windows::Forms::TextBox());
			this->gradeC = (gcnew System::Windows::Forms::TextBox());
			this->gradeD = (gcnew System::Windows::Forms::TextBox());
			this->pusebutton = (gcnew System::Windows::Forms::Button());
			this->return_main = (gcnew System::Windows::Forms::Button());
			this->openvdofile = (gcnew System::Windows::Forms::Button());
			this->pictureBox1 = (gcnew System::Windows::Forms::PictureBox());
			(cli::safe_cast<System::ComponentModel::ISupportInitialize^>(this->pictureBox1))->BeginInit();
			this->SuspendLayout();

			this->gradeA->Location = System::Drawing::Point(784, 109);
			this->gradeA->Size = System::Drawing::Size(170, 22);
			this->gradeA->Text = L"Grade A: 0";
			this->gradeA->ReadOnly = true;
			this->gradeA->BackColor = System::Drawing::Color::LightGreen;

			this->gradeB->Location = System::Drawing::Point(784, 160);
			this->gradeB->Size = System::Drawing::Size(170, 22);
			this->gradeB->Text = L"Grade B: 0";
			this->gradeB->ReadOnly = true;
			this->gradeB->BackColor = System::Drawing::Color::LightYellow;

			this->gradeC->Location = System::Drawing::Point(784, 211);
			this->gradeC->Size = System::Drawing::Size(170, 22);
			this->gradeC->Text = L"Grade C: 0";
			this->gradeC->ReadOnly = true;
			this->gradeC->BackColor = System::Drawing::Color::LightSalmon;

			this->gradeD->Location = System::Drawing::Point(784, 262);
			this->gradeD->Size = System::Drawing::Size(170, 22);
			this->gradeD->Text = L"Grade D: 0";
			this->gradeD->ReadOnly = true;
			this->gradeD->BackColor = System::Drawing::Color::LightGray;

			this->pusebutton->Location = System::Drawing::Point(162, 32);
			this->pusebutton->Size = System::Drawing::Size(75, 30);
			this->pusebutton->Text = L"Pause";
			this->pusebutton->TabIndex = 6;
			this->pusebutton->Click += gcnew EventHandler(this, &showvdo::pusebutton_Click_1);

			this->return_main->Location = System::Drawing::Point(30, 32);
			this->return_main->Size = System::Drawing::Size(75, 30);
			this->return_main->Text = L"Return";
			this->return_main->TabIndex = 7;
			this->return_main->Click += gcnew EventHandler(this, &showvdo::button1_Click_1);

			this->openvdofile->Location = System::Drawing::Point(309, 32);
			this->openvdofile->Size = System::Drawing::Size(90, 30);
			this->openvdofile->Text = L"Open VDO";
			this->openvdofile->TabIndex = 5;
			this->openvdofile->Click += gcnew EventHandler(this, &showvdo::openvdofile_Click);

			this->pictureBox1->BackColor = System::Drawing::Color::Black;
			this->pictureBox1->Location = System::Drawing::Point(12, 84);
			this->pictureBox1->Size = System::Drawing::Size(766, 538);
			this->pictureBox1->SizeMode = System::Windows::Forms::PictureBoxSizeMode::Zoom;
			this->pictureBox1->TabIndex = 4;
			this->pictureBox1->TabStop = false;

			this->BackColor = System::Drawing::SystemColors::ActiveCaption;
			this->ClientSize = System::Drawing::Size(989, 634);
			this->Text = L"Mangosteen Grading Pro";
			this->Controls->Add(this->gradeD);
			this->Controls->Add(this->gradeC);
			this->Controls->Add(this->gradeB);
			this->Controls->Add(this->gradeA);
			this->Controls->Add(this->pictureBox1);
			this->Controls->Add(this->openvdofile);
			this->Controls->Add(this->pusebutton);
			this->Controls->Add(this->return_main);
			(cli::safe_cast<System::ComponentModel::ISupportInitialize^>(this->pictureBox1))->EndInit();
			this->ResumeLayout(false);
			this->PerformLayout();
		}
#pragma endregion

	private: System::Void OnVideoTick(System::Object^ sender, EventArgs^ e)
	{
		if (isPaused || !video->isOpened()) return;
		cv::Mat frame;
		if (!video->read(frame) || frame.empty()) { videoTimer->Stop(); return; }

		cv::Mat blob = cv::dnn::blobFromImage(
			frame, 1.0 / 255.0, cv::Size(640, 640),
			cv::Scalar(0, 0, 0), true, false);
		net->setInput(blob);

		std::vector<cv::Mat> outputs;
		net->forward(outputs, net->getUnconnectedOutLayersNames());

		auto detections = YoloParser::parse(outputs, frame.cols, frame.rows);

		for (const auto& det : detections)
		{
			if (det.classId == 1) {
				countD++;
				gradeD->Text = "Grade D: " + countD.ToString();
				cv::rectangle(frame, det.box, cv::Scalar(128, 128, 128), 2);
				cv::putText(frame, "D (Unripe)",
					cv::Point(det.box.x, det.box.y - 6),
					cv::FONT_HERSHEY_SIMPLEX, 0.6, cv::Scalar(128, 128, 128), 2);
				continue;
			}

			cv::Rect safeBox = det.box & cv::Rect(0, 0, frame.cols, frame.rows);
			if (safeBox.width < 20 || safeBox.height < 20) continue;

			MangosteenAnalyzer::Result res = MangosteenAnalyzer::analyze(frame(safeBox).clone());

			switch (res.grade) {
			case 'A': countA++; gradeA->Text = "Grade A: " + countA.ToString(); break;
			case 'B': countB++; gradeB->Text = "Grade B: " + countB.ToString(); break;
			default:  countC++; gradeC->Text = "Grade C: " + countC.ToString(); break;
			}

			cv::Scalar boxColor;
			std::string label;
			switch (res.grade) {
			case 'A': boxColor = cv::Scalar(0, 255, 0);   label = "Grade:A"; break;
			case 'B': boxColor = cv::Scalar(0, 255, 255); label = "Grade:B"; break;
			default:  boxColor = cv::Scalar(0, 0, 255);   label = "Grade:C"; break;
			}

			cv::rectangle(frame, det.box, boxColor, 2);
			cv::putText(frame, label,
				cv::Point(det.box.x, det.box.y - 6),
				cv::FONT_HERSHEY_SIMPLEX, 0.65, boxColor, 2);

			char dbg[80];
			snprintf(dbg, sizeof(dbg), "B:%.1f%% R:%.2f T:%.1f Sc:%.1f",
				res.blemishPercent, res.ripenessScore, res.roughness, res.finalScore);
			cv::putText(frame, dbg,
				cv::Point(det.box.x, det.box.y + det.box.height + 16),
				cv::FONT_HERSHEY_SIMPLEX, 0.42, boxColor, 1);
		}

		// BGR -> RGB แล้วแสดง
		cv::Mat rgb;
		cv::cvtColor(frame, rgb, cv::COLOR_BGR2RGB);
		System::Drawing::Bitmap^ bmp = gcnew System::Drawing::Bitmap(
			rgb.cols, rgb.rows, (int)rgb.step,
			System::Drawing::Imaging::PixelFormat::Format24bppRgb,
			(System::IntPtr)rgb.data);
		System::Drawing::Image^ oldImg = this->pictureBox1->Image;
		this->pictureBox1->Image = (System::Drawing::Bitmap^)bmp->Clone();
		delete bmp;
		if (oldImg != nullptr) delete oldImg;
	}

	private: System::Void openvdofile_Click(System::Object^ sender, EventArgs^ e)
	{
		OpenFileDialog^ ofd = gcnew OpenFileDialog();
		ofd->Filter = "Video Files|*.mp4;*.avi;*.mov;*.mkv|All Files|*.*";
		if (ofd->ShowDialog() == System::Windows::Forms::DialogResult::OK) {
			selectedPath = ofd->FileName;
			std::string path;
			for (int i = 0; i < selectedPath->Length; i++)
				path += (char)selectedPath[i];

			if (video->isOpened()) video->release();
			video->open(path);

			if (video->isOpened()) {
				countA = countB = countC = countD = 0;
				gradeA->Text = "Grade A: 0"; gradeB->Text = "Grade B: 0";
				gradeC->Text = "Grade C: 0"; gradeD->Text = "Grade D: 0";
				isPaused = false;
				pusebutton->Text = L"Pause";
				videoTimer->Start();
			}
			else {
				MessageBox::Show("ไม่สามารถเปิดไฟล์วิดีโอได้\nลองใช้ไฟล์ .mp4 หรือ .avi");
			}
		}
	}

	private: System::Void pusebutton_Click_1(System::Object^ sender, EventArgs^ e)
	{
		if (!video->isOpened()) return;
		isPaused = !isPaused;
		pusebutton->Text = isPaused ? L"Play" : L"Pause";
	}

	private: System::Void button1_Click_1(System::Object^ sender, EventArgs^ e)
	{
		videoTimer->Stop();
		if (video->isOpened()) video->release();
		this->Close();
	}
	};

} // namespace Mangkudd