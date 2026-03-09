#pragma once
// ================================================================
//  mangosteen_core.h  (Refactored for YOLO ZeroMQ Integration)
// ================================================================
#include <string>
#include <vector>
#include <set>

#include <opencv2/core.hpp>
#include <opencv2/imgcodecs.hpp>
#include <opencv2/videoio.hpp>
#include <opencv2/imgproc.hpp>
#include <msclr/marshal_cppstd.h>

namespace Mangkudd {

    // ================================================================
    //  Data Structures
    // ================================================================
    struct AiResult {
        int id;
        cv::Rect box;
        std::string grade; // "mangosteen 1", "mangosteen 2", etc.
        char mappedGrade;  // 'A', 'B', 'C', 'D'
    };

    // ================================================================
    //  AiClient (Connects to python ai_server.py)
    // ================================================================
    public ref class AiClient {
    private:
        NetMQ::Sockets::PublisherSocket^ pubSocket;
        NetMQ::Sockets::SubscriberSocket^ subSocket;
        bool isConnected;
        std::set<int>* countedIds;

    public:
        AiClient() {
            isConnected = false;
            countedIds = new std::set<int>();
        }

        ~AiClient() {
            Disconnect();
            delete countedIds;
        }

        void ResetCounters() {
            if (countedIds) {
                countedIds->clear();
            }
        }

        bool Connect(System::String^ serverIpUnused) {
            try {
                // Publisher sends frames to AI Server (Port 5555)
                pubSocket = gcnew NetMQ::Sockets::PublisherSocket();
                pubSocket->Connect("tcp://localhost:5555");
                
                // Subscriber receives JSON results from AI Server (Port 5556)
                subSocket = gcnew NetMQ::Sockets::SubscriberSocket();
                subSocket->Connect("tcp://localhost:5556");
                subSocket->Subscribe(""); // Subscribe to everything
                
                isConnected = true;
                return true;
            }
            catch (...) {
                isConnected = false;
                countedIds = new std::set<int>();
                return false;
            }
        }

        void Disconnect() {
            if (pubSocket != nullptr) {
                pubSocket->Close();
                delete pubSocket;
                pubSocket = nullptr;
            }
            if (subSocket != nullptr) {
                subSocket->Close();
                delete subSocket;
                subSocket = nullptr;
            }
            isConnected = false;
            countedIds = new std::set<int>();
        }

        // Sends a frame out to the network immediately without waiting for a reply
        void SendFrameAsync(const cv::Mat& frame) {
            if (!isConnected || frame.empty() || pubSocket == nullptr) return;

            try {
                // Resize for network efficiency to match python script expectation (640 width)
                cv::Mat displayFrame;
                double scale = 640.0 / frame.cols;
                int newHeight = (int)(frame.rows * scale);
                cv::resize(frame, displayFrame, cv::Size(640, newHeight), 0, 0, cv::INTER_LINEAR);

                // Compress JPG to send over ZMQ
                std::vector<int> encodeParams = { cv::IMWRITE_JPEG_QUALITY, 70 };
                std::vector<uchar> buf;
                cv::imencode(".jpg", displayFrame, buf, encodeParams);

                // Convert std::vector to .NET byte array
                array<System::Byte>^ imageBytes = gcnew array<System::Byte>(buf.size());
                System::Runtime::InteropServices::Marshal::Copy((System::IntPtr)buf.data(), imageBytes, 0, buf.size());

                // Send frame (Non-blocking by nature of PUB socket)
                NetMQ::OutgoingSocketExtensions::SendFrame(pubSocket, imageBytes, false);
            }
            catch (...) {
                // Ignore send errors to maintain stream fluidity
            }
        }

        // Polls the SubscriberSocket for any incoming results (Non-blocking)
        bool TryReceiveResult(std::vector<AiResult>& outResults, int% countA, int% countB, int% countC, int% countD, double originalFrameWidth) {
            if (!isConnected || subSocket == nullptr) return false;

            try {
                System::String^ jsonResponse = nullptr;
                
                // Try to drain the socket to get the absolute latest result
                bool receivedAnything = false;
                System::String^ latestMsg = nullptr;
                
                while (NetMQ::ReceivingSocketExtensions::TryReceiveFrameString(subSocket, latestMsg)) {
                    jsonResponse = latestMsg;
                    receivedAnything = true;
                }

                if (!receivedAnything || jsonResponse == nullptr || jsonResponse == "[]" || jsonResponse->Contains("error")) {
                    return false;
                }

                double scale = 640.0 / originalFrameWidth;

                // Parse JSON
                Newtonsoft::Json::Linq::JArray^ aiResultsArray = Newtonsoft::Json::Linq::JArray::Parse(jsonResponse);

                for each (Newtonsoft::Json::Linq::JToken ^ token in aiResultsArray) {
                    AiResult res;
                    res.id = System::Convert::ToInt32(token->SelectToken("id")->ToString());
                    
                    System::String^ gradeStr = token->SelectToken("grade")->ToString();
                    res.grade = msclr::interop::marshal_as<std::string>(gradeStr);

                    // Original JSON coordinates are mapped to 640px. 
                    // Map back to original frame size.
                    int x1 = System::Convert::ToInt32(token->SelectToken("box.x1")->ToString());
                    int y1 = System::Convert::ToInt32(token->SelectToken("box.y1")->ToString());
                    int x2 = System::Convert::ToInt32(token->SelectToken("box.x2")->ToString());
                    int y2 = System::Convert::ToInt32(token->SelectToken("box.y2")->ToString());

                    int orig_x1 = (int)(x1 / scale);
                    int orig_y1 = (int)(y1 / scale);
                    int orig_x2 = (int)(x2 / scale);
                    int orig_y2 = (int)(y2 / scale);

                    res.box = cv::Rect(cv::Point(orig_x1, orig_y1), cv::Point(orig_x2, orig_y2));

                    // Map YOLO grade to A, B, C, D
                    if (gradeStr == "mangosteen 1") { res.mappedGrade = 'D'; }
                    else if (gradeStr == "mangosteen 2" || gradeStr == "mangosteen 3") { res.mappedGrade = 'C'; }
                    else if (gradeStr == "mangosteen 4") { res.mappedGrade = 'B'; }
                    else { res.mappedGrade = 'A'; } 

                    // Check duplicate counting using tracking ID
                    if (countedIds && countedIds->find(res.id) == countedIds->end()) {
                        countedIds->insert(res.id); // Mark as counted
                        if (res.mappedGrade == 'A') countA++;
                        else if (res.mappedGrade == 'B') countB++;
                        else if (res.mappedGrade == 'C') countC++;
                        else if (res.mappedGrade == 'D') countD++;
                    }

                    outResults.push_back(res);
                }

                return true;
            }
            catch (...) {
                return false;
            }
        }    };

} // namespace Mangkudd


