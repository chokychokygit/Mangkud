#include "pch.h"

using namespace System;
using namespace NetMQ;
using namespace NetMQ::Sockets;
using namespace OpenCvSharp;
using namespace Newtonsoft::Json::Linq;

void FruitProject_Main_Deprecated(array<System::String^>^ args)
{
    Console::WriteLine("🖥️ กำลังเริ่มระบบ Frontend ด้วย C++ .NET...");

    // 💡 แก้ไข 1: ใส่ค่าที่ซ่อนอยู่ของกล้อง (VideoCaptureAPIs) เข้าไปให้ครบ
    VideoCapture^ capture = gcnew VideoCapture(0, OpenCvSharp::VideoCaptureAPIs::ANY);
    if (!capture->IsOpened())
    {
        Console::WriteLine("❌ ไม่สามารถเปิดกล้องได้ครับ!");
        return;
    }

    String^ windowName = "Mangosteen Smart Sorter";
    Cv2::NamedWindow(windowName, WindowFlags::AutoSize);
    Mat^ frame = gcnew Mat();

    // 💡 แก้ไข 2: สร้าง RequestSocket แล้วเชื่อมต่อกับ Python AI
    RequestSocket^ client = gcnew RequestSocket(gcnew String("tcp://localhost:5555"));

    Console::WriteLine("✅ เชื่อมต่อกับ Python AI สำเร็จ! (คลิกที่หน้าต่างวิดีโอแล้วกด ESC เพื่อออก)");

    while (true)
    {
        capture->Read(frame);
        if (frame->Empty()) break;

        // 🚀 อัปเกรด 1: ย่อภาพให้เหลือ 640px พอดีกับโมเดล AI
        Mat^ displayFrame = gcnew Mat();
        double scale = 640.0 / frame->Width;
        int newHeight = (int)(frame->Height * scale);
        Cv2::Resize(frame, displayFrame, OpenCvSharp::Size(640, newHeight), 0, 0, InterpolationFlags::Linear);

        // 🚀 อัปเกรด 2: บีบอัดภาพก่อนส่ง (ลด Quality ลงเหลือ 70%)
        array<int>^ encodeParams = { (int)OpenCvSharp::ImwriteFlags::JpegQuality, 70 };
        array<Byte>^ imageBytes;
        Cv2::ImEncode(".jpg", displayFrame, imageBytes, encodeParams);

        // ส่งภาพไซส์เบาหวิวให้ Python
        NetMQ::OutgoingSocketExtensions::SendFrame(client, imageBytes, false);
        String^ jsonResponse = NetMQ::ReceivingSocketExtensions::ReceiveFrameString(client);

        if (jsonResponse != "[]" && !jsonResponse->Contains("error"))
        {
            JArray^ aiResults = JArray::Parse(jsonResponse);

            for each (JToken ^ token in aiResults)
            {
                int trackId = Convert::ToInt32(token->SelectToken("id")->ToString());
                String^ grade = token->SelectToken("grade")->ToString();

                int x1 = Convert::ToInt32(token->SelectToken("box.x1")->ToString());
                int y1 = Convert::ToInt32(token->SelectToken("box.y1")->ToString());
                int x2 = Convert::ToInt32(token->SelectToken("box.x2")->ToString());
                int y2 = Convert::ToInt32(token->SelectToken("box.y2")->ToString());

                OpenCvSharp::Point pt1(x1, y1);
                OpenCvSharp::Point pt2(x2, y2);
                OpenCvSharp::Scalar colorGreen(0, 255, 0);

                // ⚠️ วาดกล่องลงบนภาพขนาด 640px (displayFrame)
                Cv2::Rectangle(displayFrame, pt1, pt2, colorGreen, 2, OpenCvSharp::LineTypes::Link8, 0);

                String^ label = String::Format("ID:{0} {1}", trackId, grade);
                OpenCvSharp::Point ptText(x1, Math::Max(20, y1 - 10));
                OpenCvSharp::Scalar colorYellow(0, 255, 255);

                Cv2::PutText(displayFrame, label, ptText, OpenCvSharp::HersheyFonts::HersheySimplex, 0.7, colorYellow, 2, OpenCvSharp::LineTypes::Link8, false);
            }
        }

        // ⚠️ โชว์ภาพที่ถูกย่อและวาดกล่องแล้ว
        Cv2::ImShow(windowName, displayFrame);

        if (Cv2::WaitKey(1) == 27)
            break;
    }


    client->Close();
    capture->Release();
}