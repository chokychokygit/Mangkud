#pragma once
// ================================================================
//  mangosteen_core.h  (v2 - Fixed Logic + Threshold Guard)
//
//  การเปลี่ยนแปลงหลัก:
//  1. เพิ่ม MangosteenGuard::isValidFruit() กัน false detection
//     (หน้าคน / วัตถุอื่น) ก่อนเข้า analyze pipeline
//  2. analyze() ทำงานทีละขั้น Step 1?2?3 ใน bounding box เท่านั้น
//  3. THRESHOLD ทุกตัวรวมไว้ที่เดียว (struct Thresholds) แก้ง่าย
//  4. Result มี field debugText สำหรับวาดบน frame
// ================================================================
#include <opencv2/opencv.hpp>
#include <opencv2/dnn.hpp>
#include <vector>
#include <string>
#include <algorithm>

namespace Mangkudd {

    // ================================================================
    //  THRESHOLDS — แก้ค่าที่นี่ที่เดียว
    // ================================================================
    struct Thresholds {

        // --- Guard: กัน false detection ---
        // bounding box ต้องมีขนาดขั้นต่ำ (pixel)
        static constexpr int   MIN_BOX_SIZE = 40;
        // YOLO confidence ขั้นต่ำ
        static constexpr float MIN_CONFIDENCE = 0.60f;
        // วัตถุต้องกลม (aspect ratio ใกล้ 1.0) ?tolerance
        static constexpr float SHAPE_RATIO_MAX = 1.8f;   // w/h ไม่เกิน 1.8
        // พื้นที่สีเข้ม (มังคุด) ต้องมากกว่า X% ของ bounding box
        static constexpr double DARK_FILL_MIN = 0.30;   // 30%

        // --- Step 1: Ripeness (Hue + Lab a) ---
        static constexpr int   HUE_PURPLE_LOW = 120;   // ช่วงสีม่วงใน HSV
        static constexpr int   HUE_PURPLE_HIGH = 165;
        static constexpr int   LAB_A_RED_THRESH = 138;   // a > 138 = แดง/ม่วงเข้ม
        static constexpr int   LAB_L_MIN = 35;    // ตัดเงาออก L < 35
        static constexpr double HUE_WEIGHT = 0.60;
        static constexpr double LAB_WEIGHT = 0.40;

        // --- Step 2: Skin Damage / Roughness (LBP) ---
        // StdDev ของ LBP: สูง = ผิวขรุขระ
        static constexpr double ROUGHNESS_A = 45.0;
        static constexpr double ROUGHNESS_B = 65.0;

        // --- Step 3: Blemish (Color Mask + Contour) ---
        // HSV range สีน้ำตาล/เหลือง (ยาง, รอยถลอก)
        static constexpr int   BLEMISH_H_LOW = 8;
        static constexpr int   BLEMISH_H_HIGH = 35;
        static constexpr int   BLEMISH_S_LOW = 40;
        static constexpr int   BLEMISH_V_LOW = 40;
        static constexpr int   BLEMISH_V_HIGH = 210;
        // พื้นที่ตำหนิขั้นต่ำที่นับ (ตัด noise เล็กๆ)
        static constexpr double BLEMISH_MIN_AREA = 15.0;
        static constexpr double BLEMISH_PCT_A = 3.0;   // < 3%  = A
        static constexpr double BLEMISH_PCT_B = 10.0;  // < 10% = B

        // --- Final Score weights ---
        static constexpr double W_RIPENESS = 0.40;
        static constexpr double W_BLEMISH = 0.35;
        static constexpr double W_ROUGHNESS = 0.25;

        // --- Grade cutoff ---
        static constexpr double SCORE_A = 50.0;
        static constexpr double SCORE_B = 25.0;
    };

    // ================================================================
    //  MangosteenGuard — กัน false detection ก่อนเข้า pipeline
    // ================================================================
    class MangosteenGuard {
    public:
        // คืน true = น่าจะเป็นมังคุดจริง, false = ข้ามไป
        static bool isValidFruit(const cv::Mat& roi, float confidence) {
            if (roi.empty()) return false;

            // 1. ขนาด bounding box
            if (roi.cols < Thresholds::MIN_BOX_SIZE ||
                roi.rows < Thresholds::MIN_BOX_SIZE) return false;

            // 2. YOLO confidence
            if (confidence < Thresholds::MIN_CONFIDENCE) return false;

            // 3. รูปร่างต้องใกล้กลม (ไม่ยาวเกินไป)
            float ratio = (float)roi.cols / (float)roi.rows;
            if (ratio > Thresholds::SHAPE_RATIO_MAX ||
                ratio < (1.0f / Thresholds::SHAPE_RATIO_MAX)) return false;

            // 4. ตรวจสีเข้ม: มังคุดต้องมีพื้นที่สีเข้ม (ม่วง/ดำ) มากพอ
            //    ถ้าวัตถุสีอ่อน (หน้าคน, กระดาษ) ผ่านมา จะตกที่นี่
            cv::Mat hsv;
            cv::cvtColor(roi, hsv, cv::COLOR_BGR2HSV);
            // มังคุด: H ม่วง + V ต่ำ (สีเข้ม)
            cv::Mat darkMask;
            // H=100-180 (ม่วง-แดงม่วง), S>30, V<120 (เข้ม)
            cv::inRange(hsv,
                cv::Scalar(100, 30, 10),
                cv::Scalar(180, 255, 120),
                darkMask);
            double darkRatio = (double)cv::countNonZero(darkMask)
                / (double)(roi.cols * roi.rows);
            if (darkRatio < Thresholds::DARK_FILL_MIN) return false;

            return true;
        }
    };

    // ================================================================
    //  MangosteenAnalyzer
    // ================================================================
    class MangosteenAnalyzer {
    public:
        struct Result {
            char   grade;
            double ripenessScore;   // Step 1: 0.0-1.0
            double blemishPercent;  // Step 3: %
            double roughness;       // Step 2: LBP stddev
            double finalScore;
            bool   isValid;         // false = ไม่ผ่าน guard
            // ข้อความ debug สำหรับวาดบน frame
            std::string debugText;  // "Th:R>0.5 B<3% Rg<45"
        };

        // ----------------------------------------------------------------
        //  analyze() — pipeline ทีละขั้น ทำงานใน bounding box เท่านั้น
        //  confidence ส่งมาจาก YoloParser เพื่อใช้ใน guard
        // ----------------------------------------------------------------
        static Result analyze(const cv::Mat& roi, float confidence = 1.0f) {
            Result res = { 'C', 0.0, 0.0, 0.0, 0.0, false, "" };
            if (roi.empty()) return res;

            // ?? Guard: ตรวจก่อนว่าเป็นมังคุดจริงไหม ??????????????????
            if (!MangosteenGuard::isValidFruit(roi, confidence)) {
                res.grade = '?';   // ไม่นับเกรด
                res.debugText = "SKIP(not fruit)";
                return res;
            }
            res.isValid = true;

            // ?? สร้าง circular mask ครอบตัวผล ?????????????????????????
            cv::Mat mask = createCircleMask(roi);

            // ?? Step 1: วิเคราะห์สีผิว (Ripeness) ????????????????????
            res.ripenessScore = calcRipeness(roi, mask);

            // ?? Step 2: ตรวจผิวเสีย (Texture / Roughness) ?????????????
            res.roughness = calcTexture(roi, mask);

            // ?? Step 3: ตรวจตำหนิ (Blemish) ??????????????????????????
            res.blemishPercent = calcBlemish(roi, mask);

            // ?? Scoring ????????????????????????????????????????????????
            double ripenessNorm = res.ripenessScore * 100.0;
            double blemishNorm = std::min(res.blemishPercent / 20.0, 1.0) * 100.0;
            double roughnessNorm = std::min(res.roughness / 80.0, 1.0) * 100.0;

            res.finalScore = (ripenessNorm * Thresholds::W_RIPENESS)
                - (blemishNorm * Thresholds::W_BLEMISH)
                - (roughnessNorm * Thresholds::W_ROUGHNESS);
            res.finalScore = std::max(res.finalScore, 0.0);

            // ?? Grade Decision (ต้องผ่านทุก threshold) ?????????????????
            if (res.finalScore >= Thresholds::SCORE_A &&
                res.blemishPercent < Thresholds::BLEMISH_PCT_A &&
                res.roughness < Thresholds::ROUGHNESS_A) {
                res.grade = 'A';
            }
            else if (res.finalScore >= Thresholds::SCORE_B &&
                res.blemishPercent < Thresholds::BLEMISH_PCT_B &&
                res.roughness < Thresholds::ROUGHNESS_B) {
                res.grade = 'B';
            }
            else {
                res.grade = 'C';
            }

            // ?? Debug text แสดงค่า Threshold ที่ใช้ ????????????????????
            char buf[128];
            snprintf(buf, sizeof(buf),
                "Ripe:%.2f Blm:%.1f%% Rgh:%.1f Sc:%.1f->%c",
                res.ripenessScore,
                res.blemishPercent,
                res.roughness,
                res.finalScore,
                res.grade);
            res.debugText = buf;

            return res;
        }

    private:
        // ?? Utility: สร้าง circular mask ?????????????????????????????
        static cv::Mat createCircleMask(const cv::Mat& src) {
            cv::Mat mask = cv::Mat::zeros(src.size(), CV_8UC1);
            cv::circle(mask,
                cv::Point(src.cols / 2, src.rows / 2),
                (int)(std::min(src.cols, src.rows) * 0.46),
                cv::Scalar(255), -1);
            return mask;
        }

        // ?? Step 1: Ripeness — Hue Ratio + Lab a-channel ?????????????
        static double calcRipeness(const cv::Mat& src, const cv::Mat& fruitMask) {
            cv::Mat hsv, lab;
            cv::cvtColor(src, hsv, cv::COLOR_BGR2HSV);
            cv::cvtColor(src, lab, cv::COLOR_BGR2Lab);

            std::vector<cv::Mat> hsvCh, labCh;
            cv::split(hsv, hsvCh);
            cv::split(lab, labCh);

            // ตัดเงาออก
            cv::Mat brightMask, validMask;
            cv::threshold(labCh[0], brightMask,
                Thresholds::LAB_L_MIN, 255, cv::THRESH_BINARY);
            cv::bitwise_and(fruitMask, brightMask, validMask);

            double totalPx = (double)cv::countNonZero(validMask);
            if (totalPx == 0) return 0.0;

            // Hue ratio
            cv::Mat purpleMask;
            cv::inRange(hsvCh[0],
                cv::Scalar(Thresholds::HUE_PURPLE_LOW),
                cv::Scalar(Thresholds::HUE_PURPLE_HIGH),
                purpleMask);
            cv::bitwise_and(purpleMask, validMask, purpleMask);
            double hueRatio = (double)cv::countNonZero(purpleMask) / totalPx;

            // Lab a-channel ratio
            cv::Mat redMask;
            cv::threshold(labCh[1], redMask,
                Thresholds::LAB_A_RED_THRESH, 255, cv::THRESH_BINARY);
            cv::bitwise_and(redMask, validMask, redMask);
            double aRatio = (double)cv::countNonZero(redMask) / totalPx;

            return (hueRatio * Thresholds::HUE_WEIGHT)
                + (aRatio * Thresholds::LAB_WEIGHT);
        }

        // ?? Step 2: Texture / Roughness — LBP StdDev ?????????????????
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

        // ?? Step 3: Blemish — HSV Color Mask + Contour ???????????????
        static double calcBlemish(const cv::Mat& src, const cv::Mat& fruitMask) {
            cv::Mat hsv, bMask;
            cv::cvtColor(src, hsv, cv::COLOR_BGR2HSV);
            cv::inRange(hsv,
                cv::Scalar(Thresholds::BLEMISH_H_LOW,
                    Thresholds::BLEMISH_S_LOW,
                    Thresholds::BLEMISH_V_LOW),
                cv::Scalar(Thresholds::BLEMISH_H_HIGH,
                    255,
                    Thresholds::BLEMISH_V_HIGH),
                bMask);
            cv::bitwise_and(bMask, fruitMask, bMask);

            cv::Mat kernel = cv::getStructuringElement(
                cv::MORPH_ELLIPSE, cv::Size(3, 3));
            cv::morphologyEx(bMask, bMask, cv::MORPH_OPEN, kernel);

            std::vector<std::vector<cv::Point>> contours;
            cv::findContours(bMask, contours,
                cv::RETR_EXTERNAL, cv::CHAIN_APPROX_SIMPLE);

            double blemishArea = 0;
            for (const auto& cnt : contours) {
                double a = cv::contourArea(cnt);
                if (a > Thresholds::BLEMISH_MIN_AREA) blemishArea += a;
            }
            double fruitArea = (double)cv::countNonZero(fruitMask);
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
            int      classId;     // 0=Ripe, 1=Un_Ripe
            float    confidence;
        };

        static std::vector<Detection> parse(
            const std::vector<cv::Mat>& outputs,
            int frameW, int frameH,
            float confThresh = Thresholds::MIN_CONFIDENCE,
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

} // namespace Mangkudd