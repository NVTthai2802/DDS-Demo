# Nền tảng Học và Trình diễn DDS (DDS Demonstration Platform)

Ứng dụng demo Publisher/Subscriber dựa trên chuẩn DDS (Data Distribution Service) cho kiến trúc mạng ngang hàng (Mesh/P2P) không cần máy chủ trung tâm (Brokerless).

## 1. Thành phần Dự án
- **android_app**: Mã nguồn ứng dụng Android (Kotlin + C++ JNI) sử dụng Fast DDS. Hoạt động như một **Publisher** giả lập dữ liệu cảm biến môi trường. Tích hợp khả năng thay đổi QoS (Reliable/Best Effort) động.
- **wsl_backend**: Mã nguồn C++ Fast DDS Subscriber chạy trên môi trường WSL2 (Ubuntu). Thu thập dữ liệu từ mạng nội bộ, sử dụng **DomainParticipantListener** để bắt sự kiện Discovery (Node Join/Leave) và xuất JSON.
- **laptop_dashboard**: Ứng dụng Python Streamlit kết hợp **SQLite** để lưu trữ và hiển thị Dashboard vẽ biểu đồ Plotly thời gian thực, quản lý Topology và phân tích hiệu năng QoS.

## 2. Tiêu chí đã nghiệm thu (Bao gồm Giai đoạn 2)
- Cấu hình kết nối **Many-to-Many** thông qua UDP Multicast (Tự động khám phá SPDP).
- Giao tiếp ngang hàng (P2P), giảm thiểu Single Point of Failure (SPoF).
- **Discovery Tracking:** Bắt sự kiện mạng (Node tham gia/rời khỏi mạng) ngay tức thì.
- **Liveliness QoS:** Phát hiện thiết bị sập nguồn/rớt Wi-Fi sau 3 giây mất tín hiệu (Lease Duration).
- **QoS Tuning:** Thay đổi QoS linh hoạt (Reliable/Best Effort) từ điện thoại.
- **Hệ thống lưu trữ độc lập:** Dữ liệu được lưu trữ qua SQLite để tối ưu hóa việc phân tích và tránh nghẽn luồng Streamlit UI.

## 3. Hướng dẫn Triển khai (Tái lập Hệ thống)

### Bước 1: Cấu hình Mạng (Windows Host)
Để WSL2 (Ubuntu) có thể bắt được gói tin UDP Multicast trong mạng LAN nội bộ, tính năng **Mirrored Networking** cần được bật.

1. Mở file `C:\Users\<Your_Username>\.wslconfig` và thêm đoạn sau:
   ```ini
   [wsl2]
   memory=8GB
   processors=4
   swap=2GB
   networkingMode=mirrored
   ```
2. Khởi động lại WSL: Mở PowerShell và chạy `wsl --shutdown`.
3. Cấp quyền Firewall cho DDS (Mở PowerShell với quyền Administrator):
   ```powershell
   netsh advfirewall firewall add rule name="DDS UDP In" dir=in action=allow protocol=UDP localport=7400-8000
   netsh advfirewall firewall add rule name="DDS UDP Out" dir=out action=allow protocol=UDP localport=7400-8000
   ```

### Bước 2: Build & Chạy C++ Backend (WSL2)
1. Mở Terminal Ubuntu (WSL2) và cài đặt Fast DDS:
   ```bash
   sudo apt update
   sudo apt install -y libfastrtps-dev fastddsgen cmake g++
   ```
2. Biên dịch mã nguồn Backend:
   ```bash
   cd "/mnt/c/Project TTS/DDS-Demo/wsl_backend/backend/build"
   cmake ..
   make -j4
   ```

### Bước 3: Cài đặt và Chạy Dashboard (Python Streamlit)
1. Cài đặt môi trường Python ảo (trên Terminal Ubuntu):
   ```bash
   cd "/mnt/c/Project TTS/DDS-Demo/laptop_dashboard"
   python3 -m venv venv
   source venv/bin/activate
   pip install -r requirements.txt
   ```
2. Khởi động Dashboard:
   ```bash
   streamlit run app.py
   ```
   *Dashboard sẽ mở tại `http://localhost:8501` trên trình duyệt Windows.*

### Bước 4: Chạy Ứng dụng Android (Publisher)
1. Mở thư mục `android_app` bằng **Android Studio**.
2. Kết nối thiết bị di động Android vào Laptop (hoặc dùng Emulator). 
   *Lưu ý: Điện thoại bắt buộc phải kết nối chung một mạng Wi-Fi (cùng Subnet) với Laptop.*
3. Bấm **Build > Clean Project**, sau đó bấm nút **Play ▶️ (Run 'app')** để nạp lên thiết bị.
4. Mở app trên điện thoại, tuỳ chọn cấu hình QoS (Reliable vs Best Effort), rồi bấm nút **"Khởi tạo DDS (Auto-Discovery)"** -> **"Start Publishing"**.
5. Mở trình duyệt Web trên Laptop (Dashboard), dữ liệu môi trường và cấu trúc liên kết mạng sẽ được vẽ biểu đồ tự động tại các Tab.

## 4. Tài liệu Kỹ thuật
Vui lòng xem chi tiết file `Bao_Cao_Ky_Thuat_Bee_Labs.md` và `Network_Topology.md` ở thư mục gốc để nắm rõ lý do kỹ thuật, kiến trúc, đo lường Latency/QoS, và các vấn đề vướng mắc.