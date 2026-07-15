# DDS Mesh Network - Bee Labs

Hệ thống mạng Mesh DDS (Data Distribution Service) ngang hàng (P2P), không Broker trung tâm, thử nghiệm trên phần cứng thực tế.
Dự án được xây dựng từ đầu để khắc phục hoàn toàn các lỗi kiến trúc của phiên bản cũ.

## 🌟 Tính năng nổi bật
- **Many-to-Many thực thụ**: Android app vừa Publish vừa Subscribe, tự hiển thị dữ liệu của nhau.
- **Auto Discovery SPDP/SEDP**: Phân biệt rõ "Đã vào mạng" (Joined) và "Sẵn sàng truyền dữ liệu" (Matched).
- **Zero-config IP**: Các node tự tìm nhau qua Multicast UDP, không cần cấu hình IP tĩnh.
- **Fault-Tolerance**: Ngắt kết nối 1 node không ảnh hưởng đến toàn hệ thống.
- **Đa nền tảng CMAKE**: Build C++ linh hoạt trên WSL2 và Linux Native mà không cần sửa đường dẫn.

## 📂 Cấu trúc Repo
- `android_app/`: Dự án Android Studio (Kotlin + C++ JNI). Tự động tải & biên dịch Fast DDS cho NDK.
- `backend/`: C++ DDS Subscriber & Logger.
- `dashboard/`: Streamlit Dashboard + SQLite lưu lịch sử.
- `docs/`: Báo cáo kỹ thuật (`report.md`) và Sơ đồ topology (`topology/`).
- `slides/`: Slide thuyết trình (HTML).
- `scripts/`: Script hỗ trợ build đa môi trường.

## 🚀 Hướng dẫn Cài đặt & Tái lập (Reproducibility)

### 1. Build C++ Backend (Laptop)
Yêu cầu: Linux Native (Ubuntu) hoặc WSL2, có cài `cmake`, `g++`, `make`, `java` (để chạy fastddsgen nếu sửa IDL). Thư viện Fast DDS và Fast CDR phải được cài đặt sẵn.
```bash
cd scripts
chmod +x build_backend.sh
./build_backend.sh
```
*Executable sẽ nằm tại `backend/build/backend_node`*.

### 2. Chạy Dashboard (Laptop)
Yêu cầu: Python 3.9+
```bash
cd dashboard
pip install -r requirements.txt
streamlit run app.py
```
*Dashboard tự động gọi C++ backend chạy ngầm và đọc dữ liệu. Giao diện mở tại `http://localhost:8501`*

### 3. Build Android App (Điện thoại)
Yêu cầu: Android Studio, NDK, Cmake.
- Mở thư mục `android_app/` bằng Android Studio.
- Sync Gradle: Quá trình này sẽ sử dụng `FetchContent` để tự động tải Fast DDS, Fast CDR, Asio từ Github và biên dịch chéo qua NDK (có thể mất 5-10 phút tuỳ máy).
- Bấm **Run** để cài đặt APK lên điện thoại thật. (Yêu cầu chung mạng Wi-Fi với Laptop).

## 📊 Hướng dẫn Quay Video Demo & Checklist
Để quay video nghiệm thu (2-5 phút), hãy làm theo kịch bản:
1. Mở Streamlit Dashboard trên màn hình Laptop.
2. Mở App trên thiết bị Android. (Nếu thiếu máy thật, có thể chạy thêm Emulator, nhưng nên ưu tiên ít nhất 2 máy thật để demo mạng Wi-Fi thực tế).
3. **[Chứng minh SPDP/SEDP]**: Chỉ ra Dashboard hiện các node ở trạng thái `MATCHED` (Online/Xanh lá).
4. **[Chứng minh Many-to-Many]**: Quay sát màn hình 1 chiếc điện thoại, cho thấy nó đang hiển thị dữ liệu nhận được từ các thiết bị còn lại (Sẽ bổ sung logic hiển thị trên màn hình App khi code chi tiết hơn nếu cần).
5. **[Chứng minh Fault Tolerance]**: Tắt Wi-Fi 1 chiếc điện thoại. Chỉ ra Dashboard cập nhật node đó thành `Offline`, trong khi thiết bị kia vẫn đang nhảy số liệu liên tục bình thường.

## 🌐 Hướng dẫn Deploy Slides (GitHub Pages)
Slide thuyết trình được viết thuần bằng HTML (Reveal.js). Để deploy:
1. Vào Settings của Github Repo -> **Pages**.
2. Chọn Source là **Deploy from a branch**.
3. Chọn nhánh `master`, thư mục gốc `/` (hoặc tạo một nhánh `gh-pages` riêng tuỳ ý).
4. Save. Link slide sẽ có dạng `https://<username>.github.io/<repo>/slides/index.html`.

## ⚙️ Đo đạc QoS & Latency
**LƯU Ý:** 
Kết quả chạy thử trên WSL2 (Windows) chỉ mang tính chất Smoke Test / Compile-check do độ trễ của cơ chế Mirrored Networking.
Để đo được Latency và QoS chính thức cho báo cáo, bắt buộc phải copy mã nguồn sang máy tính chạy **Linux Native** (Ubuntu) và đo đạc trong môi trường mạng Wi-Fi LAN vật lý.
