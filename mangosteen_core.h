#pragma once
// ================================================================
//  mangosteen_core.h
//  Unmanaged C++ classes สำหรับวิเคราะห์มังคุด
//  ไฟล์นี้ถูก include โดย camera.h และ showvdo.h
//  #pragma once ป้องกัน define ซ้ำ (แก้ C2011)
// ================================================================
#include <opencv2/opencv.hpp>
#include <opencv2/dnn.hpp>
#include <vector>
#include <string>
#include <algorithm>

namespace Mangkudd {

	// ================================================================
	//  MangosteenAnalyzer
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
	//  YoloParser
	// ================================================================
	class YoloParser {
	public:
		struct Detection {
			cv::Rect box;
			int      classId;   // 0=Ripe, 1=Un_Ripe
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
				out = out.reshape(1, out.size[1]); // [6, 8400]

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

} // namespace Mangkudd