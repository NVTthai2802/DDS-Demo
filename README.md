# DDS Mesh Demo v2 (Bee Labs)

Hệ thống thử nghiệm giao thức **Data Distribution Service (DDS)** ngang hàng (P2P), không Broker trung tâm, trên nền tảng di động Android và Laptop Dashboard (Native Windows 11). Thử nghiệm chứng minh kiến trúc mạng ngang hàng P2P Many-to-Many với Fast DDS trên phần cứng thực tế.

## Kiến Trúc
- **Android App:** C++ Native JNI + Kotlin. Thiết bị vừa là Publisher vừa là Subscriber. Đo độ trễ tự động và hiển thị log real-time.
- **Backend Node (C++):** C++ Fast DDS Subscriber thu nhận log discovery (SPDP/SEDP) và sensor data. Chạy Native trên Windows 11.
- **Streamlit Dashboard (Python):** Hiển thị UI realtime với các metrics Latency, Packet Loss, và kiểm soát Fault Tolerance bằng Liveliness QoS.

## Cài đặt & Build

### 1. Build C++ Backend (Native Windows)
**Điều kiện tiên quyết:** Bạn phải build & install Fast DDS 2.11.2 (từ git tag) và Fast-CDR vào thư mục local trước. Quá trình này được thực hiện thông qua script `backend/Fast-DDS/configure_fastdds.bat` và `build_fastdds.bat`.

Do máy hiện tại cài Visual Studio 2026 Build Tools (v145) làm mặc định (gây lỗi biên dịch `_Mtx_init` trong `TimedMutex.hpp`), chúng ta bắt buộc phải dùng toolset **MSVC v143**. Đồng thời, quá trình build phải ưu tiên liên kết với Fast DDS 2.11.2 tự build thay vì bản 3.x từ vcpkg.

Vì vậy, **không mở Developer Command Prompt for VS 2022 mặc định từ Start Menu**. Thay vào đó, mở Command Prompt bình thường và chạy:
```cmd
cd C:\Project TTS\DDS-Demo
scripts\build_backend.bat
```
*(Script này sẽ tự động gọi `vcvars64.bat -vcvars_ver=14.44` để nạp đúng toolset v143, thiết lập `CMAKE_PREFIX_PATH` trỏ vào bản Fast DDS local, rồi cấu hình `CMAKE_TOOLCHAIN_FILE` cho vcpkg trước khi gọi Ninja build).*

### 2. Build Android App
- Mở thư mục `android_app/` bằng **Android Studio**.
- Sync Gradle và cắm điện thoại Android vào để build.
- App yêu cầu **minSdk = 26**.
- **Quan trọng:** App có xin quyền `WifiManager.MulticastLock` để SPDP Discovery chạy được qua Wi-Fi mạng LAN. Hãy đảm bảo Router Wi-Fi của bạn không chặn Multicast UDP.

### 3. Chạy Dashboard
Cài đặt thư viện Python (yêu cầu Python 3.9+):
```bash
cd dashboard
pip install -r requirements.txt
```

Khởi chạy (Code Python sẽ tự gọi trực tiếp executable Windows Native `backend_node.exe`):
```cmd
streamlit run app.py
```

## Tính năng Chính (Mới trong v2)
1. **Dynamic Device ID:** Không còn cứng ID. App tự sinh bằng `Build.MODEL` và lưu vào SharedPreferences.
2. **Liveliness QoS (Fault Tolerance):** Nếu bạn ngắt kết nối Wi-Fi trên 1 thiết bị, Dashboard sẽ báo đỏ (Offline) trong chưa đầy 3 giây nhờ cơ chế Liveliness (Lease Duration).
3. **Many-to-Many:** Các điện thoại đều thấy thông điệp của nhau.
4. **Packet Loss & Latency:** Đo trực tiếp để giám sát độ tin cậy.

## Tài liệu
- `Bao_Cao_Ky_Thuat_Bee_Labs_v2.md`: Báo cáo kỹ thuật chi tiết cùng Topology.
- `docs/topology.md`: Sơ đồ mạng hệ thống.
- `docs/slides.html`: Slide thuyết trình Reveal.js ngắn gọn.
- `docs/test_checklist.md`: Checklist kịch bản test cho team QA/Dev.


## 🌐 Hướng dẫn Deploy Slides (GitHub Pages)
Slide thuyết trình được viết thuần bằng HTML (Reveal.js). Để deploy:
1. Vào Settings của Github Repo -> **Pages**.
2. Chọn Source là **Deploy from a branch**.
3. Chọn nhánh `rebuild-v2`, thư mục gốc `/` (hoặc tạo một nhánh `gh-pages` riêng tuỳ ý).
4. Save. Link slide sẽ có dạng `https://<username>.github.io/<repo>/docs/slides.html`.

## ⚙️ Đo đạc QoS & Latency
**LƯU Ý:** 
Kết quả chạy thử được thực thi trên môi trường Native Windows. Để dữ liệu thực tế không bị rớt, bắt buộc phải cấp quyền Allow cho `backend_node.exe` và `dummy_pub.exe` thông qua **Windows Firewall** (hoặc tắt tường lửa với mạng Private). 
Môi trường đo đạc lý tưởng là 1 máy tính Windows 11 và tối thiểu 2 thiết bị Android chạy trên cùng một mạng Wi-Fi vật lý.
