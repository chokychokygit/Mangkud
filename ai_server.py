import cv2
import zmq
import json
import numpy as np
from ultralytics import YOLO

# ===================================================================
# ส่วนที่ 1: สมองกล AI (นี่คือส่วนที่ถูกย่อไว้ตรงคอมเมนต์ครับ)
# ===================================================================
class MangosteenAI:
    def __init__(self, detector_path, classifier_path):
        print("🚀 กำลังโหลดสมองกลทั้ง 2 ตัวเข้าสู่ระบบ...")
        self.detector = YOLO(detector_path)
        self.classifier = YOLO(classifier_path)
        self.grade_memory = {} # สมุดจดจำเกรดเพื่อลดภาระการ์ดจอ

    def process_frame(self, frame, conf_threshold=0.75):
        """รับภาพ 1 เฟรมเข้ามา -> ส่งคืนพิกัดและเกรดออกไป (กรองความมั่นใจด้วย conf_threshold)"""
        # เพิ่มพารามิเตอร์ conf=conf_threshold เพื่อกรองวัตถุที่มันใจน้อยกว่าที่กำหนดทิ้ง
        results = self.detector.track(frame, persist=True, tracker="bytetrack.yaml", imgsz=640, conf=conf_threshold, verbose=False)
        output_data = [] 

        if results[0].boxes.id is not None:
            boxes = results[0].boxes.xyxy.cpu().numpy()
            track_ids = results[0].boxes.id.int().cpu().numpy()

            for box, track_id in zip(boxes, track_ids):
                x1, y1, x2, y2 = map(int, box)

                # ถ้าเป็น ID ใหม่ ให้ส่งไปแยกเกรด
                if track_id not in self.grade_memory:
                    crop_img = frame[y1:y2, x1:x2]
                    if crop_img.size > 0:
                        cls_results = self.classifier(crop_img, verbose=False)
                        top_class_idx = cls_results[0].probs.top1
                        grade_name = cls_results[0].names[top_class_idx]
                        self.grade_memory[track_id] = grade_name
                
                # ดึงเกรดจากสมุดจด
                grade = self.grade_memory.get(track_id, "Unknown")

                # แพ็กข้อมูลใส่ตะกร้าเตรียมส่งกลับ
                output_data.append({
                    "id": int(track_id),
                    "box": {"x1": x1, "y1": y1, "x2": x2, "y2": y2},
                    "grade": grade
                })

        return output_data

# ===================================================================
# ส่วนที่ 2: ระบบเซิร์ฟเวอร์เปิดท่อส่งข้อมูล (ZeroMQ PUB/SUB)
# ===================================================================
def start_zmq_server():
    print("🧠 กำลังเตรียมเปิดท่อรับส่งข้อมูล (PUB/SUB)...")
    
    # โหลดโมเดล (ตรวจสอบชื่อไฟล์ให้ตรงกับที่คุณเซฟไว้นะครับ)
    ai = MangosteenAI("best_mangosteen_yolo26s.pt", "grade_mangosteen_best.pt")

    context = zmq.Context()
    
    # สร้างท่อรับภาพ (SUB) รอรับจาก C++ Publisher
    sub_socket = context.socket(zmq.SUB)
    sub_socket.bind("tcp://*:5555") 
    sub_socket.setsockopt_string(zmq.SUBSCRIBE, "") # รับทุกภาพที่ส่งมาแบบไม่กรอง
    
    # สร้างท่อกระจายเสียง (PUB) เพื่อประกาศเกรดและพิกัดให้ C++ Subscriber
    pub_socket = context.socket(zmq.PUB)
    pub_socket.bind("tcp://*:5556")

    print("🟢 Python AI Server เปิดทำงานแบบสายพานแล้ว!")
    print("   - รอรับรูปภาพ (SUB) ที่พอร์ต 5555")
    print("   - กระจายผลลัพธ์ (PUB) ที่พอร์ต 5556")

    while True:
        try:
            # 1. รอรับไฟล์ภาพที่ฝั่งหน้าบ้าน (C++) ส่งมาแบบต่อเนื่อง
            # ถ้ามีภาพค้างในคิวเยอะๆ ให้ดึงแต่ภาพล่าสุดทิ้งภาพเก่า (แต่ถ้าดึงวนไวพอก็ไม่ล้น)
            message = sub_socket.recv()
            
            # ล้างคิว (Flush) เอาเฉพาะภาพล่าสุดเพื่อการทำงานแบบ Real-time แท้จริง
            while True:
                try:
                    message = sub_socket.recv(zmq.NOBLOCK)
                except zmq.Again:
                    break # คิวว่างแล้ว เอา message ชิ้นสุดท้ายนี้แหละไปรัน AI
            
            # 2. แปลงข้อมูลไบต์ให้กลับมาเป็นรูปภาพ
            nparr = np.frombuffer(message, np.uint8)
            frame = cv2.imdecode(nparr, cv2.IMREAD_COLOR)

            if frame is not None:
                # 3. ส่งภาพเข้าสมองกล AI ด้านบน
                results = ai.process_frame(frame)

                # 4. แปลงผลลัพธ์เป็นข้อความ JSON แล้วกระจายออกโทรโข่ง (PUB)
                json_data = json.dumps(results)
                pub_socket.send_string(json_data)

        except Exception as e:
            print("เกิดข้อผิดพลาด:", e)

# สั่งให้โปรแกรมเริ่มทำงาน
if __name__ == '__main__':
    start_zmq_server()