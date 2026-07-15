# DDS Mesh Network Demo

Dự án xây dựng mạng Mesh DDS (Data Distribution Service) ngang hàng (peer-to-peer), không sử dụng Broker trung tâm, thử nghiệm trên phần cứng thật (Laptop và Android).

**Mục tiêu**: Chứng minh khả năng giao tiếp P2P, tự động khám phá và tính chịu lỗi (fault-tolerance) của các thiết bị trong cùng DDS Domain.

## 🏗 Cấu trúc hệ thống
- **Middleware**: eProsima Fast DDS 2.11.2
- **Backend (Laptop)**: Đóng vai trò Node giám sát/Subscriber. Lắng nghe cả sự kiện phát hiện thiết bị (SPDP) và sự kiện ghép cặp (SEDP). Viết bằng C++.
- **Android App**: Đóng vai trò các Node hoạt động, vừa xuất (Publish) vừa nhận (Subscribe) dữ liệu trên cùng Topic `SensorData`. Tự sinh `device_id` độc lập và cấu hình QoS từ UI.
- **Dashboard**: Viết bằng Python Streamlit, thu thập số liệu và hiển thị trạng thái kết nối + lưu lượng thực tế.

## 📝 Lưu ý về Số liệu Đo đạc (Latency / QoS)
Quá trình build và thử nghiệm chia làm hai giai đoạn:
1. **Giai đoạn Smoke Test (trên WSL2 Windows)**: Nhằm kiểm tra khả năng biên dịch, phát hiện lỗi sơ bộ. Số liệu trên WSL2 (với Mirrored Networking/NAT) **không** đại diện cho latency thực tế do qua nhiều lớp mạng ảo hóa của Windows.
2. **Giai đoạn Đo đạc Chính thức (trên Linux Native)**: Toàn bộ chỉ số Latency, Packet Loss, Throughput trong `docs/report.md` được đo đạc và xác nhận trên thiết bị chạy Linux native trực tiếp.

## 🚀 Hướng dẫn Build và Cài đặt

*(Tài liệu chi tiết các bước setup sẽ được cập nhật trong quá trình hoàn thiện code)*

1. Cài đặt eProsima Fast DDS 2.11.2 (sẽ có script tự động).
2. Tích hợp NDK cho Android.
3. Build Backend C++ bằng CMake độc lập môi trường.
4. Chạy Dashboard Streamlit.
