#pragma once
// ================================================================
//  mangosteen_core.h  (v3)
//  - Thresholds เป็น struct ธรรมดา (ไม่ใช่ constexpr) ? แก้จาก UI ได้
//  - analyze() รับ Thresholds& เป็น parameter
//  - pipeline แยกชัด: Guard ? Step1 Ripeness ? Step2 Roughness ? Step3 Blemish
// ================================================================
#include <opencv2/opencv.hpp>
#include <opencv2/dnn.hpp>
#include <vector>
#include <string>
#include <algorithm>

namespace Mangkudd {

    // ================================================================
    //  Thresholds — แก้ค่าได้จากภายนอก
    // ================================================================
    struct Thresholds {
        // Guard
        int    minBoxSize = 40;
        float  minConfidence = 0.60f;
        float  shapeRatioMax = 1.8f;
        double darkFillMin = 0.25;

        // Step 1: Ripeness
        int    huePurpleLow = 120;
        int    huePurpleHigh = 165;
        int    labAThresh = 138;
        int    labLMin = 35;
        double hueWeight = 0.60;
        double labWeight = 0.40;

        // Step 2: Roughness
        double roughnessA = 45.0;
        double roughnessB = 65.0;

        // Step 3: Blemish
        int    blemishHLow = 8;
        int    blemishHHigh = 35;
        int    blemishSLow = 40;
        int    blemishVLow = 40;
        int    blemishVHigh = 210;
        double blemishMinArea = 15.0;
        double blemishPctA = 3.0;
        double blemishPctB = 10.0;

        // Scoring weights
        double wRipeness = 0.40;
        double wBlemish = 0.35;
        double wRoughness = 0.25;

        // Grade cutoff
        double scoreA = 50.0;
        double scoreB = 25.0;
    };

    // ================================================================
    //  MangosteenGuard
    // ================================================================
    class MangosteenGuard {
    public:
        static bool isValidFruit(const cv::Mat& roi,
            float confidence,
            const Thresholds& th)
        {
            if (roi.empty()) return false;
            if (roi.cols < th.minBoxSize || roi.rows < th.minBoxSize) return false;
            if (confidence < th.minConfidence) return false;

            float ratio = (float)roi.cols / (float)roi.rows;
            if (ratio > th.shapeRatioMax ||
                ratio < (1.0f / th.shapeRatioMax)) return false;

            // ตรวจสีเข้ม (H=100-180 ม่วง, V<120 เข้ม)
            cv::Mat hsv;
            cv::cvtColor(roi, hsv, cv::COLOR_BGR2HSV);
            cv::Mat darkMask;
            cv::inRange(hsv,
                cv::Scalar(100, 30, 10),
                cv::Scalar(180, 255, 120),
                darkMask);
            double darkRatio = (double)cv::countNonZero(darkMask)
                / (double)(roi.cols * roi.rows);
            if (darkRatio < th.darkFillMin) return false;

            return true;
        }
    };

    // ================================================================
    //  MangosteenAnalyzer
    // ================================================================
    class MangosteenAnalyzer {
    public:
        struct Result {
            char   grade = 'C';
            double ripenessScore = 0.0;
            double blemishPercent = 0.0;
            double roughness = 0.0;
            double finalScore = 0.0;
            bool   isValid = false;
            std::string debugText;
        };

        // analyze() รับ Thresholds จากภายนอก
        static Result analyze(const cv::Mat& roi,
            float confidence,
            const Thresholds& th)
        {
            Result res;
            if (roi.empty()) return res;

            if (!MangosteenGuard::isValidFruit(roi, confidence, th)) {
                res.grade = '?';
                res.debugText = "SKIP(not fruit)";
                return res;
            }
            res.isValid = true;

            cv::Mat mask = createCircleMask(roi);

            // Step 1
            res.ripenessScore = calcRipeness(roi, mask, th);
            // Step 2
            res.roughness = calcTexture(roi, mask);
            // Step 3
            res.blemishPercent = calcBlemish(roi, mask, th);

            // Scoring
            double ripenessNorm = res.ripenessScore * 100.0;
            double blemishNorm = std::min(res.blemishPercent / 20.0, 1.0) * 100.0;
            double roughnessNorm = std::min(res.roughness / 80.0, 1.0) * 100.0;

            res.finalScore = (ripenessNorm * th.wRipeness)
                - (blemishNorm * th.wBlemish)
                - (roughnessNorm * th.wRoughness);
            res.finalScore = std::max(res.finalScore, 0.0);

            if (res.finalScore >= th.scoreA &&
                res.blemishPercent < th.blemishPctA &&
                res.roughness < th.roughnessA) {
                res.grade = 'A';
            }
            else if (res.finalScore >= th.scoreB &&
                res.blemishPercent < th.blemishPctB &&
                res.roughness < th.roughnessB) {
                res.grade = 'B';
            }
            else {
                res.grade = 'C';
            }

            char buf[128];
            snprintf(buf, sizeof(buf),
                "Ripe:%.2f Blm:%.1f%% Rgh:%.1f Sc:%.1f->%c",
                res.ripenessScore, res.blemishPercent,
                res.roughness, res.finalScore, res.grade);
            res.debugText = buf;
            return res;
        }

    private:
        static cv::Mat createCircleMask(const cv::Mat& src) {
            cv::Mat m = cv::Mat::zeros(src.size(), CV_8UC1);
            cv::circle(m,
                cv::Point(src.cols / 2, src.rows / 2),
                (int)(std::min(src.cols, src.rows) * 0.46),
                cv::Scalar(255), -1);
            return m;
        }

        static double calcRipeness(const cv::Mat& src,
            const cv::Mat& mask,
            const Thresholds& th)
        {
            cv::Mat hsv, lab;
            cv::cvtColor(src, hsv, cv::COLOR_BGR2HSV);
            cv::cvtColor(src, lab, cv::COLOR_BGR2Lab);
            std::vector<cv::Mat> hsvCh, labCh;
            cv::split(hsv, hsvCh);
            cv::split(lab, labCh);

            cv::Mat brightMask, validMask;
            cv::threshold(labCh[0], brightMask, th.labLMin, 255, cv::THRESH_BINARY);
            cv::bitwise_and(mask, brightMask, validMask);

            double totalPx = (double)cv::countNonZero(validMask);
            if (totalPx == 0) return 0.0;

            cv::Mat purpleMask;
            cv::inRange(hsvCh[0],
                cv::Scalar(th.huePurpleLow), cv::Scalar(th.huePurpleHigh),
                purpleMask);
            cv::bitwise_and(purpleMask, validMask, purpleMask);
            double hueRatio = (double)cv::countNonZero(purpleMask) / totalPx;

            cv::Mat redMask;
            cv::threshold(labCh[1], redMask, th.labAThresh, 255, cv::THRESH_BINARY);
            cv::bitwise_and(redMask, validMask, redMask);
            double aRatio = (double)cv::countNonZero(redMask) / totalPx;

            return (hueRatio * th.hueWeight) + (aRatio * th.labWeight);
        }

        static double calcTexture(const cv::Mat& src, const cv::Mat& mask) {
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
            cv::meanStdDev(lbp, mean, stddev, mask);
            return stddev[0];
        }

        static double calcBlemish(const cv::Mat& src,
            const cv::Mat& mask,
            const Thresholds& th)
        {
            cv::Mat hsv, bMask;
            cv::cvtColor(src, hsv, cv::COLOR_BGR2HSV);
            cv::inRange(hsv,
                cv::Scalar(th.blemishHLow, th.blemishSLow, th.blemishVLow),
                cv::Scalar(th.blemishHHigh, 255, th.blemishVHigh),
                bMask);
            cv::bitwise_and(bMask, mask, bMask);

            cv::Mat kernel = cv::getStructuringElement(cv::MORPH_ELLIPSE, cv::Size(3, 3));
            cv::morphologyEx(bMask, bMask, cv::MORPH_OPEN, kernel);

            std::vector<std::vector<cv::Point>> contours;
            cv::findContours(bMask, contours, cv::RETR_EXTERNAL, cv::CHAIN_APPROX_SIMPLE);

            double blemishArea = 0;
            for (const auto& cnt : contours) {
                double a = cv::contourArea(cnt);
                if (a > th.blemishMinArea) blemishArea += a;
            }
            double fruitArea = (double)cv::countNonZero(mask);
            if (fruitArea == 0) return 0.0;
            return (blemishArea / fruitArea) * 100.0;
        }
    };

    // ================================================================
    //  YoloParser
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
            if (out.dims == 3) out = out.reshape(1, out.size[1]);

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

                float cx = out.at<float>(0, i) * scaleX, cy = out.at<float>(1, i) * scaleY;
                float w = out.at<float>(2, i) * scaleX, h = out.at<float>(3, i) * scaleY;
                int x1 = std::max(0, (int)(cx - w / 2));
                int y1 = std::max(0, (int)(cy - h / 2));
                boxes.push_back(cv::Rect(x1, y1,
                    std::min((int)w, frameW - x1),
                    std::min((int)h, frameH - y1)));
                scores.push_back(bestConf);
                classIds.push_back(bestCls);
            }

            std::vector<int> indices;
            cv::dnn::NMSBoxes(boxes, scores, confThresh, nmsThresh, indices);
            for (int idx : indices) {
                Detection d{ boxes[idx], classIds[idx], scores[idx] };
                results.push_back(d);
            }
            return results;
        }
    };

} // namespace Mangkudd