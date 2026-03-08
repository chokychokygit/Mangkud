#pragma once
// ================================================================
//  mangosteen_core.h  (Refactored for 5-Stage Skin Color Grading)
// ================================================================
#include <algorithm>
#include <opencv2/dnn.hpp>
#include <opencv2/opencv.hpp>
#include <string>
#include <vector>

namespace Mangkudd {

// ================================================================
//  THRESHOLDS (5 Stages of Color)
// ================================================================
struct Thresholds {
  // --- Guard: Prevent false detection ---
  static constexpr int MIN_BOX_SIZE = 10;
  static constexpr float MIN_CONFIDENCE = 0.13f;
  static constexpr float SHAPE_RATIO_MAX = 2.5f;

  // --- Color Stages (HSV Ranges) ---
  // Stage 1: Green
  static constexpr int S1_H_LOW = 36, S1_H_HIGH = 105;

  // Stage 2: Green-Pink (Yellow/Orange/Pink)
  static constexpr int S2_H_LOW = 16, S2_H_HIGH = 35;

  // Stage 3: Red (Hue wraps around 0 and 180)
  static constexpr int S3_H_LOW1 = 0, S3_H_HIGH1 = 15;
  static constexpr int S3_H_LOW2 = 166, S3_H_HIGH2 = 180;

  // Stage 4: Purple-Red
  static constexpr int S4_H_LOW = 110, S4_H_HIGH = 165;

  // Stage 5: Dark / Black (Based on Value/Brightness)
  static constexpr int S5_V_MAX = 50; // if V <= 50, considered dark/black

  static constexpr int MIN_SATURATION = 20; // Ignore grayscale pixels
};

// ================================================================
//  MangosteenGuard
// ================================================================
class MangosteenGuard {
public:
  // Return true if it's a valid bounding box, false if it's junk
  static bool isValidFruit(const cv::Mat &roi, float confidence) {
    if (roi.empty())
      return false;
    if (roi.cols < Thresholds::MIN_BOX_SIZE ||
        roi.rows < Thresholds::MIN_BOX_SIZE)
      return false;
    if (confidence < Thresholds::MIN_CONFIDENCE)
      return false;

    float ratio = (float)roi.cols / (float)roi.rows;
    if (ratio > Thresholds::SHAPE_RATIO_MAX ||
        ratio < (1.0f / Thresholds::SHAPE_RATIO_MAX))
      return false;

    // Disabled dark fill check to avoid skipping red baskets completely, we
    // will grade them instead.
    return true;
  }
};

// ================================================================
//  MangosteenAnalyzer
// ================================================================
class MangosteenAnalyzer {
public:
  struct Result {
    char grade;
    int dominantStage;  // 1 to 5
    double stagePct[5]; // Percentage array: 0=S1, 1=S2, 2=S3, 3=S4, 4=S5
    bool isValid;
    std::string debugText;
  };

  static Result analyze(const cv::Mat &roi, float confidence = 1.0f) {
    Result res = {'C', 1, {0, 0, 0, 0, 0}, false, ""};
    if (roi.empty())
      return res;

    // 1. Check Guard
    if (!MangosteenGuard::isValidFruit(roi, confidence)) {
      res.grade = '?';
      res.debugText = "SKIP(not fruit)";
      return res;
    }
    res.isValid = true;

    cv::Mat mask = createCircleMask(roi);

    // Convert to HSV
    cv::Mat hsv;
    cv::cvtColor(roi, hsv, cv::COLOR_BGR2HSV);

    // Stage 5 mask: Very dark pixels regardless of hue
    cv::Mat m5;
    cv::inRange(hsv, cv::Scalar(0, 0, 0),
                cv::Scalar(180, 255, Thresholds::S5_V_MAX), m5);

    // Filter valid color pixels (not too dark, not completely desaturated)
    cv::Mat validColorMask;
    cv::inRange(
        hsv,
        cv::Scalar(0, Thresholds::MIN_SATURATION, Thresholds::S5_V_MAX + 1),
        cv::Scalar(180, 255, 255), validColorMask);

    cv::Mat m1, m2, m3_1, m3_2, m3, m4;
    cv::inRange(hsv, cv::Scalar(Thresholds::S1_H_LOW, 0, 0),
                cv::Scalar(Thresholds::S1_H_HIGH, 255, 255), m1);
    cv::inRange(hsv, cv::Scalar(Thresholds::S2_H_LOW, 0, 0),
                cv::Scalar(Thresholds::S2_H_HIGH, 255, 255), m2);
    cv::inRange(hsv, cv::Scalar(Thresholds::S3_H_LOW1, 0, 0),
                cv::Scalar(Thresholds::S3_H_HIGH1, 255, 255), m3_1);
    cv::inRange(hsv, cv::Scalar(Thresholds::S3_H_LOW2, 0, 0),
                cv::Scalar(Thresholds::S3_H_HIGH2, 255, 255), m3_2);
    cv::bitwise_or(m3_1, m3_2, m3);
    cv::inRange(hsv, cv::Scalar(Thresholds::S4_H_LOW, 0, 0),
                cv::Scalar(Thresholds::S4_H_HIGH, 255, 255), m4);

    // Apply valid brightness/saturation
    cv::bitwise_and(m1, validColorMask, m1);
    cv::bitwise_and(m2, validColorMask, m2);
    cv::bitwise_and(m3, validColorMask, m3);
    cv::bitwise_and(m4, validColorMask, m4);

    // Apply circular circle mask
    cv::bitwise_and(m1, mask, m1);
    cv::bitwise_and(m2, mask, m2);
    cv::bitwise_and(m3, mask, m3);
    cv::bitwise_and(m4, mask, m4);
    cv::bitwise_and(m5, mask, m5);

    // Calculate Percentages
    int totalPx = cv::countNonZero(mask);
    if (totalPx > 0) {
      res.stagePct[0] = (cv::countNonZero(m1) * 100.0) / totalPx;
      res.stagePct[1] = (cv::countNonZero(m2) * 100.0) / totalPx;
      res.stagePct[2] = (cv::countNonZero(m3) * 100.0) / totalPx;
      res.stagePct[3] = (cv::countNonZero(m4) * 100.0) / totalPx;
      res.stagePct[4] = (cv::countNonZero(m5) * 100.0) / totalPx;
    }

    // Find Dominant Stage
    double maxPct = -1.0;
    int domStage = 1;
    for (int i = 0; i < 5; i++) {
      if (res.stagePct[i] > maxPct) {
        maxPct = res.stagePct[i];
        domStage = i + 1;
      }
    }

    // Heuristic: Prevent shadow on a green fruit from falsely classifying as
    // Stage 5 (Dark)
    if (domStage == 5) {
      double unripePct = res.stagePct[0] + res.stagePct[1];
      double ripePct = res.stagePct[2] + res.stagePct[3];
      if (unripePct > ripePct * 2.0 && unripePct > 10.0) {
        // Fall back to the most dominant unripe stage
        domStage = (res.stagePct[0] > res.stagePct[1]) ? 1 : 2;
        maxPct = res.stagePct[domStage - 1];
      }
    }

    res.dominantStage = domStage;

    // Grade Mapping based on 5 Stages
    // Stage 1 -> D
    // Stage 2 -> C
    // Stage 3 -> B
    // Stage 4, 5 -> A
    if (domStage == 1)
      res.grade = 'D';
    else if (domStage == 2)
      res.grade = 'C';
    else if (domStage == 3)
      res.grade = 'B';
    else
      res.grade = 'A';

    char buf[128];
    snprintf(buf, sizeof(buf), "Stage:%d P%.0f%%->%c", domStage, maxPct,
             res.grade);
    res.debugText = buf;

    return res;
  }

private:
  static cv::Mat createCircleMask(const cv::Mat &src) {
    cv::Mat mask = cv::Mat::zeros(src.size(), CV_8UC1);
    cv::circle(mask, cv::Point(src.cols / 2, src.rows / 2),
               (int)(std::min(src.cols, src.rows) * 0.46), cv::Scalar(255), -1);
    return mask;
  }
};

// ================================================================
//  YoloParser
// ================================================================
class YoloParser {
public:
  struct Detection {
    cv::Rect box;
    int classId; // 0=Ripe, 1=Un_Ripe
    float confidence;
  };

  static std::vector<Detection>
  parse(const std::vector<cv::Mat> &outputs, int frameW, int frameH,
        float confThresh = Thresholds::MIN_CONFIDENCE,
        float nmsThresh = 0.45f,
        int inputSize = 640) {
    std::vector<Detection> results;
    if (outputs.empty())
      return results;

    cv::Mat out = outputs[0];
    if (out.dims == 3)
      out = out.reshape(1, out.size[1]);

    int numDetections = out.cols;
    int numClasses = out.rows - 4;
    float scaleX = (float)frameW / (float)inputSize;
    float scaleY = (float)frameH / (float)inputSize;

    std::vector<cv::Rect> boxes;
    std::vector<float> scores;
    std::vector<int> classIds;

    for (int i = 0; i < numDetections; i++) {
      float bestConf = 0;
      int bestCls = 0;
      for (int c = 0; c < numClasses; c++) {
        float conf = out.at<float>(4 + c, i);
        if (conf > bestConf) {
          bestConf = conf;
          bestCls = c;
        }
      }
      if (bestConf < confThresh)
        continue;

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
//  FruitTracker (IoU + Moving Average)
// ================================================================
struct TrackedFruit {
  int id;
  cv::Rect box;
  int classId;     // 0=Ripe, 1=Un_Ripe
  int missCount;   // How many frames we haven't seen this fruit
  
  // For Moving Average of Stage Percentages
  std::vector<std::vector<double>> historyPct; 
  MangosteenAnalyzer::Result smoothedResult;

  void addHistory(const MangosteenAnalyzer::Result& res) {
    std::vector<double> currentPct(5);
    for(int i=0; i<5; i++) currentPct[i] = res.stagePct[i];
    
    historyPct.push_back(currentPct);
    if(historyPct.size() > 7) { // Keep last 5 frames
      historyPct.erase(historyPct.begin());
    }

    // Calculate Average
    double avgPct[5] = {0,0,0,0,0};
    for(const auto& h : historyPct) {
      for(int i=0; i<5; i++) avgPct[i] += h[i];
    }
    for(int i=0; i<5; i++) avgPct[i] /= historyPct.size();

    // Re-evaluate dominant stage based on average
    double maxPct = -1.0;
    int domStage = 1;
    for (int i = 0; i < 5; i++) {
      smoothedResult.stagePct[i] = avgPct[i];
      if (avgPct[i] > maxPct) {
        maxPct = avgPct[i];
        domStage = i + 1;
      }
    }

    // Anti-shadow heuristic
    if (domStage == 5) {
      double unripePct = avgPct[0] + avgPct[1];
      double ripePct = avgPct[2] + avgPct[3];
      if (unripePct > ripePct * 2.0 && unripePct > 10.0) {
        domStage = (avgPct[0] > avgPct[1]) ? 1 : 2;
        maxPct = avgPct[domStage - 1];
      }
    }

    smoothedResult.dominantStage = domStage;
    if (domStage == 1) smoothedResult.grade = 'D';
    else if (domStage == 2) smoothedResult.grade = 'C';
    else if (domStage == 3) smoothedResult.grade = 'B';
    else smoothedResult.grade = 'A';

    smoothedResult.isValid = true;
    char buf[128];
    snprintf(buf, sizeof(buf), "Stage:%d P%.0f%%->%c", domStage, maxPct, smoothedResult.grade);
    smoothedResult.debugText = buf;
  }
};

class FruitTracker {
private:
  std::vector<TrackedFruit> activeTracks;
  int nextId = 1;
  const int MAX_MISS_COUNT = 3; // Remove fruit if unseen for 3 tracked frames
  const float IOU_THRESHOLD = 0.3f;

  static float calculateIoU(const cv::Rect& r1, const cv::Rect& r2) {
    cv::Rect intersection = r1 & r2;
    float intersectionArea = intersection.area();
    float unionArea = r1.area() + r2.area() - intersectionArea;
    return (unionArea <= 0) ? 0 : intersectionArea / unionArea;
  }

public:
  void reset() {
    activeTracks.clear();
    nextId = 1;
  }

  // Returns list of current tracks, outputting new counts for A,B,C,D
  std::vector<TrackedFruit> update(const std::vector<YoloParser::Detection>& detections, 
                                   const cv::Mat& frame,
                                   int* countA, int* countB, int* countC, int* countD) {
    
    std::vector<bool> matched(activeTracks.size(), false);
    std::vector<TrackedFruit> nextTracks;

    for (const auto& det : detections) {
      // Find best matching existing track
      int bestIdx = -1;
      float bestIoU = 0.0f;
      
      for (size_t i = 0; i < activeTracks.size(); i++) {
        if (matched[i] || activeTracks[i].classId != det.classId) continue;
        float iou = calculateIoU(det.box, activeTracks[i].box);
        if (iou > bestIoU && iou > IOU_THRESHOLD) {
          bestIoU = iou;
          bestIdx = i;
        }
      }

      if (bestIdx >= 0) {
        // Update existing track
        matched[bestIdx] = true;
        activeTracks[bestIdx].box = det.box;
        activeTracks[bestIdx].missCount = 0;
        
        if (det.classId == 0) { // Ripe (Grade A,B,C)
          cv::Rect safeBox = det.box & cv::Rect(0, 0, frame.cols, frame.rows);
          if (safeBox.width >= Thresholds::MIN_BOX_SIZE && safeBox.height >= Thresholds::MIN_BOX_SIZE) {
            MangosteenAnalyzer::Result instRes = MangosteenAnalyzer::analyze(frame(safeBox), det.confidence);
            if (instRes.isValid) {
              // Record previous grade to check if it changed (though usually we don't count twice, 
              // but to handle counting correctly we only count when NEW fruit appears)
              activeTracks[bestIdx].addHistory(instRes);
            }
          }
        }
        nextTracks.push_back(activeTracks[bestIdx]);
      } else {
        // Create NEW track
        TrackedFruit newFruit;
        newFruit.id = nextId++;
        newFruit.box = det.box;
        newFruit.classId = det.classId;
        newFruit.missCount = 0;

        if (det.classId == 1) { // Unripe is D immediately
          newFruit.smoothedResult.grade = 'D';
          newFruit.smoothedResult.isValid = true;
          if (countD) (*countD)++;
        } else {
          cv::Rect safeBox = det.box & cv::Rect(0, 0, frame.cols, frame.rows);
          if (safeBox.width >= Thresholds::MIN_BOX_SIZE && safeBox.height >= Thresholds::MIN_BOX_SIZE) {
            MangosteenAnalyzer::Result instRes = MangosteenAnalyzer::analyze(frame(safeBox), det.confidence);
            if(instRes.isValid){
               newFruit.addHistory(instRes);
               // ONLY count when a new fruit appears
               if(newFruit.smoothedResult.grade == 'A' && countA) (*countA)++;
               else if(newFruit.smoothedResult.grade == 'B' && countB) (*countB)++;
               else if(newFruit.smoothedResult.grade == 'C' && countC) (*countC)++;
            } else {
               newFruit.smoothedResult.isValid = false; // invalid guard
               newFruit.smoothedResult.debugText = instRes.debugText;
            }
          } else {
             newFruit.smoothedResult.isValid = false;
          }
        }
        nextTracks.push_back(newFruit);
      }
    }

    // Keep unmatched tracks alive for a bit (solves flickering visibility)
    for (size_t i = 0; i < activeTracks.size(); i++) {
      if (!matched[i]) {
        activeTracks[i].missCount++;
        if (activeTracks[i].missCount < MAX_MISS_COUNT) {
          nextTracks.push_back(activeTracks[i]);
        }
      }
    }

    activeTracks = nextTracks;
    return activeTracks;
  }
};

} // namespace Mangkudd