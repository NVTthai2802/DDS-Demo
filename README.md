# DDS Mesh Network - Bee Labs

Hệ thống mạng Mesh DDS (Data Distribution Service) ngang hàng (P2P), không Broker trung tâm, thử nghiệm trên phần cứng thực tế.
# DDS Mesh Demo v2 (Bee Labs)

Hệ thống thử nghiệm giao thức **Data Distribution Service (DDS)** trên nền tảng di động Android và Laptop Dashboard (WSL2/Linux). Thử nghiệm chứng minh kiến trúc mạng ngang hàng P2P Many-to-Many với Fast DDS.

## Kiến Trúc
- **Android App:** C++ Native JNI + Kotlin. Thiết bị vừa là Publisher vừa là Subscriber. Đo độ trễ tự động và hiển thị log real-time.
- **Backend Node (C++):** C++ Fast DDS Subscriber thu nhận log discovery (SPDP/SEDP) và sensor data. Chạy Native trên Windows 11.
- **Streamlit Dashboard (Python):** Hiển thị UI realtime với các metrics Latency, Packet Loss, và kiểm soát Fault Tolerance bằng Liveliness QoS.

## Cài đặt & Build

### 1. Build C++ Backend (Native Windows)
Dự án yêu cầu cài đặt **Visual Studio 2022 Build Tools (MSVC v143)** và vcpkg.
Mở **Developer Command Prompt for VS 2022** và chạy:
```cmd
cd backend
mkdir build_app && cd build_app
cmake -G Ninja -DCMAKE_BUILD_TYPE=Release -DCMAKE_POLICY_VERSION_MINIMUM=3.5 -DCMAKE_PREFIX_PATH="<đường_dẫn_tới_vcpkg_và_fastdds>" -DCMAKE_TOOLCHAIN_FILE="<đường_dẫn_tới_vcpkg.cmake>" ..
cmake --build .
```
*(Bạn cũng có thể chạy trực tiếp script PowerShell `scripts/build_backend.ps1` nếu đã setup vcpkg đúng)*

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

## 📊 Hướng dẫn Quay Video Demo & Checklist
1. Mở Streamlit Dashboard trên màn hình Laptop.
2. Mở App trên thiết bị Android. (Nếu thiếu máy thật, có thể chạy thêm Emulator, nhưng nên ưu tiên ít nhất 2 máy thật để demo mạng Wi-Fi thực tế).
3. **[Chứng minh SPDP/SEDP]**: Chỉ ra Dashboard hiện các node ở trạng thái `MATCHED` (Online/Xanh lá).
4. **[Chứng minh Many-to-Many]**: Quay sát màn hình 1 chiếc điện thoại, cho thấy nó đang hiển thị dữ liệu nhận được từ các thiết bị còn lại.
5. **[Chứng minh Fault Tolerance]**: Tắt Wi-Fi 1 chiếc điện thoại. Chỉ ra Dashboard cập nhật node đó thành `Offline`, trong khi thiết bị kia vẫn đang nhảy số liệu liên tục bình thường.

## 🌐 Hướng dẫn Deploy Slides (GitHub Pages)
Slide thuyết trình được viết thuần bằng HTML (Reveal.js). Để deploy:
1. Vào Settings của Github Repo -> **Pages**.
2. Chọn Source là **Deploy from a branch**.
3. Chọn nhánh `master`, thư mục gốc `/` (hoặc tạo một nhánh `gh-pages` riêng tuỳ ý).
4. Save. Link slide sẽ có dạng `https://<username>.github.io/<repo>/slides/index.html`.

## ⚙️ Đo đạc QoS & Latency
**LƯU Ý:** 
Kết quả chạy thử được thực thi trên môi trường Native Windows. Để dữ liệu thực tế không bị rớt, bắt buộc phải cấp quyền Allow cho `backend_node.exe` và `dummy_pub.exe` thông qua **Windows Firewall** (hoặc tắt tường lửa với mạng Private). 
Môi trường đo đạc lý tưởng là 1 máy tính Windows 11 và tối thiểu 2 thiết bị Android chạy trên cùng một mạng Wi-Fi vật lý.
