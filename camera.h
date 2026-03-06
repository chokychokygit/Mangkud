#pragma once
#include <opencv2/opencv.hpp>
#include <opencv2/dnn.hpp>
#include <vector>
#include <string>
#include <algorithm>

namespace Mangkudd {

	using namespace System;
	using namespace System::Windows::Forms;
	using namespace System::Drawing;
	using namespace System::ComponentModel;

	// ================================================================
	//  MangosteenAnalyzer  –  defined ONCE here, used by both forms
	// ================================================================
	class MangosteenAnalyzer {
	public:
		struct Result {
			char   grade;
			double ripenessScore;
			double blemishPercent;
			double roughness;
			double finalScore;
		};

		static Result analyze(const cv::Mat& roi) {
			Result res = { 'C', 0.0, 0.0, 0.0, 0.0 };
			if (roi.empty()) return res;

			cv::Mat mask = createCircleMask(roi);
			res.ripenessScore = calcRipeness(roi, mask);
			res.blemishPercent = calcBlemish(roi, mask);
			res.roughness = calcTexture(roi, mask);

			double ripenessNorm = res.ripenessScore * 100.0;
			double blemishNorm = std::min(res.blemishPercent / 20.0, 1.0) * 100.0;
			double roughnessNorm = std::min(res.roughness / 80.0, 1.0) * 100.0;

			res.finalScore = (ripenessNorm * 0.40)
				- (blemishNorm * 0.35)
				- (roughnessNorm * 0.25);
			res.finalScore = std::max(res.finalScore, 0.0);

			if (res.finalScore >= 50.0 && res.blemishPercent < 3.0 && res.roughness < 45.0) res.grade = 'A';
			else if (res.finalScore >= 25.0 && res.blemishPercent < 10.0 && res.roughness < 65.0) res.grade = 'B';
			else                                                                                   res.grade = 'C';
			return res;
		}

	private:
		static cv::Mat createCircleMask(const cv::Mat& src) {
			cv::Mat mask = cv::Mat::zeros(src.size(), CV_8UC1);
			cv::circle(mask,
				cv::Point(src.cols / 2, src.rows / 2),
				(int)(std::min(src.cols, src.rows) * 0.46),
				cv::Scalar(255), -1);
			return mask;
		}

		static double calcRipeness(const cv::Mat& src, const cv::Mat& fruitMask) {
			cv::Mat hsv, lab;
			cv::cvtColor(src, hsv, cv::COLOR_BGR2HSV);
			cv::cvtColor(src, lab, cv::COLOR_BGR2Lab);

			std::vector<cv::Mat> hsvCh, labCh;
			cv::split(hsv, hsvCh);
			cv::split(lab, labCh);

			cv::Mat brightMask, validMask;
			cv::threshold(labCh[0], brightMask, 40, 255, cv::THRESH_BINARY);
			cv::bitwise_and(fruitMask, brightMask, validMask);

			double totalPx = (double)cv::countNonZero(validMask);
			if (totalPx == 0) return 0.0;

			cv::Mat purpleMask;
			cv::inRange(hsvCh[0], cv::Scalar(125), cv::Scalar(160), purpleMask);
			cv::bitwise_and(purpleMask, validMask, purpleMask);
			double hueRatio = (double)cv::countNonZero(purpleMask) / totalPx;

			cv::Mat redMask;
			cv::threshold(labCh[1], redMask, 140, 255, cv::THRESH_BINARY);
			cv::bitwise_and(redMask, validMask, redMask);
			double aRatio = (double)cv::countNonZero(redMask) / totalPx;

			return (hueRatio * 0.60) + (aRatio * 0.40);
		}

		static double calcBlemish(const cv::Mat& src, const cv::Mat& fruitMask) {
			cv::Mat hsv, bMask;
			cv::cvtColor(src, hsv, cv::COLOR_BGR2HSV);
			cv::inRange(hsv, cv::Scalar(8, 40, 40), cv::Scalar(35, 255, 210), bMask);
			cv::bitwise_and(bMask, fruitMask, bMask);

			cv::Mat kernel = cv::getStructuringElement(cv::MORPH_ELLIPSE, cv::Size(3, 3));
			cv::morphologyEx(bMask, bMask, cv::MORPH_OPEN, kernel);

			std::vector<std::vector<cv::Point>> contours;
			cv::findContours(bMask, contours, cv::RETR_EXTERNAL, cv::CHAIN_APPROX_SIMPLE);

			double blemishArea = 0;
			for (const auto& cnt : contours) {
				double a = cv::contourArea(cnt);
				if (a > 15.0) blemishArea += a;
			}
			double fruitArea = (double)cv::countNonZero(fruitMask);
			if (fruitArea == 0) return 0.0;
			return (blemishArea / fruitArea) * 100.0;
		}

		static double calcTexture(const cv::Mat& src, const cv::Mat& fruitMask) {
			cv::Mat gray;
			cv::cvtColor(src, gray, cv::COLOR_BGR2GRAY);

			cv::Mat lbp = cv::Mat::zeros(src.size(), CV_8UC1);
			for (int i = 1; i < gray.rows - 1; i++) {
				for (int j = 1; j < gray.cols - 1; j++) {
					uchar ctr = gray.at<uchar>(i, j);
					unsigned char code = 0;
					code |= (gray.at<uchar>(i - 1, j - 1) > ctr) << 7;
					code |= (gray.at<uchar>(i - 1, j) > ctr) << 6;
					code |= (gray.at<uchar>(i - 1, j + 1) > ctr) << 5;
					code |= (gray.at<uchar>(i, j + 1) > ctr) << 4;
					code |= (gray.at<uchar>(i + 1, j + 1) > ctr) << 3;
					code |= (gray.at<uchar>(i + 1, j) > ctr) << 2;
					code |= (gray.at<uchar>(i + 1, j - 1) > ctr) << 1;
					code |= (gray.at<uchar>(i, j - 1) > ctr) << 0;
					lbp.at<uchar>(i, j) = code;
				}
			}
			cv::Scalar mean, stddev;
			cv::meanStdDev(lbp, mean, stddev, fruitMask);
			return stddev[0];
		}
	};

	// ================================================================
	//  YoloParser  –  defined ONCE here, used by both forms
	// ================================================================
	class YoloParser {
	public:
		struct Detection {
			cv::Rect box;
			int      classId;
			float    confidence;
		};

		static std::vector<Detection> parse(
			const std::vector<cv::Mat>& outputs,
			int frameW, int frameH,
			float confThresh = 0.50f,
			float nmsThresh = 0.45f)
		{
			std::vector<Detection> results;
			if (outputs.empty()) return results;

			cv::Mat out = outputs[0];
			if (out.dims == 3)
				out = out.reshape(1, out.size[1]);

			int numDetections = out.cols;
			int numClasses = out.rows - 4;
			float scaleX = (float)frameW / 640.0f;
			float scaleY = (float)frameH / 640.0f;

			std::vector<cv::Rect>  boxes;
			std::vector<float>     scores;
			std::vector<int>       classIds;

			for (int i = 0; i < numDetections; i++) {
				float bestConf = 0; int bestCls = 0;
				for (int c = 0; c < numClasses; c++) {
					float conf = out.at<float>(4 + c, i);
					if (conf > bestConf) { bestConf = conf; bestCls = c; }
				}
				if (bestConf < confThresh) continue;

				float cx = out.at<float>(0, i) * scaleX;
				float cy = out.at<float>(1, i) * scaleY;
				float w = out.at<float>(2, i) * scaleX;
				float h = out.at<float>(3, i) * scaleY;

				int x1 = std::max(0, (int)(cx - w / 2));
				int y1 = std::max(0, (int)(cy - h / 2));
				int bw = std::min((int)w, frameW - x1);
				int bh = std::min((int)h, frameH - y1);

				boxes.push_back(cv::Rect(x1, y1, bw, bh));
				scores.push_back(bestConf);
				classIds.push_back(bestCls);
			}

			std::vector<int> indices;
			cv::dnn::NMSBoxes(boxes, scores, confThresh, nmsThresh, indices);

			for (int idx : indices) {
				Detection d;
				d.box = boxes[idx];
				d.classId = classIds[idx];
				d.confidence = scores[idx];
				results.push_back(d);
			}
			return results;
		}
	};

	// ================================================================
	//  camera Form
	// ================================================================
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
				net = new cv::dnn::Net(cv::dnn::readNetFromONNX("best.onnx"));
				net->setPreferableBackend(cv::dnn::DNN_BACKEND_OPENCV);
				net->setPreferableTarget(cv::dnn::DNN_TARGET_CPU);
			}
			catch (...) {
				MessageBox::Show("ไม่พบไฟล์ best.onnx\nกรุณาวางไฟล์ไว้ในโฟลเดอร์เดียวกับ .exe");
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

			// splitContainer1
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

			// camerashow
			this->camerashow->BackColor = System::Drawing::Color::Black;
			this->camerashow->Dock = System::Windows::Forms::DockStyle::Fill;
			this->camerashow->SizeMode = System::Windows::Forms::PictureBoxSizeMode::Zoom;
			this->camerashow->Name = L"camerashow";
			this->camerashow->TabStop = false;

			// startbutton
			this->startbutton->Location = System::Drawing::Point(25, 20);
			this->startbutton->Size = System::Drawing::Size(161, 50);
			this->startbutton->Text = L"START";
			this->startbutton->Font = (gcnew System::Drawing::Font(L"Microsoft YaHei", 12.0F, System::Drawing::FontStyle::Bold));
			this->startbutton->BackColor = System::Drawing::Color::LightGreen;
			this->startbutton->Click += gcnew System::EventHandler(this, &camera::startbutton_Click);

			// btnReset
			this->btnReset->Location = System::Drawing::Point(25, 80);
			this->btnReset->Size = System::Drawing::Size(161, 40);
			this->btnReset->Text = L"RESET COUNT";
			this->btnReset->BackColor = System::Drawing::Color::LightYellow;
			this->btnReset->Click += gcnew System::EventHandler(this, &camera::btnReset_Click);

			// Grade Labels
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

			// button1 (RETURN)
			this->button1->Font = (gcnew System::Drawing::Font(L"Microsoft YaHei", 12.0F, System::Drawing::FontStyle::Bold));
			this->button1->Location = System::Drawing::Point(25, 580);
			this->button1->Size = System::Drawing::Size(161, 50);
			this->button1->Text = L"RETURN";
			this->button1->BackColor = System::Drawing::Color::LightCoral;
			this->button1->Click += gcnew System::EventHandler(this, &camera::button1_Click);

			// Form
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
		cv::Mat frame;
		if (!cap->read(frame) || frame.empty()) return;

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
				lblGradeD->Text = "Grade D: " + countD.ToString();
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
			case 'A': countA++; lblGradeA->Text = "Grade A: " + countA.ToString(); break;
			case 'B': countB++; lblGradeB->Text = "Grade B: " + countB.ToString(); break;
			default:  countC++; lblGradeC->Text = "Grade C: " + countC.ToString(); break;
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
		System::Drawing::Image^ oldImg = this->camerashow->Image;
		this->camerashow->Image = (System::Drawing::Bitmap^)bmp->Clone();
		delete bmp;
		if (oldImg != nullptr) delete oldImg;
	}

	private: System::Void startbutton_Click(System::Object^ sender, System::EventArgs^ e)
	{
		if (!isCameraRunning) {
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
			cap->release();
			this->camerashow->Image = nullptr;
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
		if (isCameraRunning) { isCameraRunning = false; cameraTimer->Stop(); cap->release(); }
		this->Close();
	}

	private: System::Void camera_Load(System::Object^ sender, System::EventArgs^ e)
	{
		this->splitContainer1->BringToFront();
	}
	};

} // namespace Mangkudd