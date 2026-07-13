# Báo Cáo Kỹ Thuật: Hệ thống Giám sát Môi trường P2P DDS Mesh

**Dự án:** Bee Labs — Peer-to-Peer DDS Mesh Network  
**Ngày:** 13/07/2026  
**Môi trường:** WSL2 Ubuntu 24.04 + Android 14  
**Middleware:** eProsima Fast DDS 2.11.2  

---

## 1. Lý do lựa chọn Middleware (Fast DDS)
Trong giai đoạn nghiên cứu, nhóm đã xem xét giữa hai middleware mã nguồn mở phổ biến: **Eclipse CycloneDDS** và **eProsima Fast DDS**.
- **Quyết định:** Chọn **Fast DDS**.
- **Lý do chính:** 
  1. **Tương thích Android (NDK):** Fast DDS cung cấp tài liệu rất tốt về việc cross-compile cho Android ARM64, trong khi CycloneDDS đòi hỏi nhiều config rườm rà đối với CMake trên Android.
  2. **Hỗ trợ C++ Hiện đại:** Fast DDS hỗ trợ C++11/14 tốt, rất thích hợp để viết JNI (Java Native Interface) giao tiếp với Kotlin.
  3. **Hiệu năng:** Tối ưu hóa đặc biệt tốt cho môi trường mạng Wi-Fi và nhúng, quản lý bộ nhớ linh hoạt với Fast CDR.

## 2. Kết quả Đo lường QoS & Latency
Dựa trên thực tế thử nghiệm 5 thiết bị (1 Laptop + 4 Phone):
- **Topology:** Mạng Wi-Fi nội bộ LAN (Subnet 192.168.0.x).
- **Latency (Độ trễ):** Trung bình đạt `< 10ms` từ lúc Phone sinh dữ liệu đến lúc xuất hiện trên Streamlit Dashboard.
- **Throughput:** Khi cả 4 thiết bị gửi bản tin ở tốc độ 1Hz, hệ thống dễ dàng xử lý mà không có hiện tượng bottleneck.
- **Packet Loss:** Với thiết lập `RELIABLE_RELIABILITY_QOS` và `KEEP_LAST_HISTORY_QOS` (depth = 10), tỉ lệ rớt gói tin đạt **0%** trong điều kiện sóng Wi-Fi ổn định.
- **Fault Tolerance:** Tắt ngẫu nhiên 1 Phone, hoặc ngắt kết nối Laptop rồi bật lại, các thiết bị lập tức khám phá lại nhau (Plug & Play) trong vòng < 3 giây nhờ cơ chế SPDP Multicast.

## 3. Các Lỗi / Vướng mắc Kỹ thuật đã gặp & Cách Xử Lý

### 3.1. Rào cản Multicast trên Android (Lỗi IGMP / Wi-Fi Sleep)
**Vấn đề:** 
Android mặc định chặn các gói tin UDP Multicast để tiết kiệm pin. Khi Fast DDS cố gắng gửi bản tin SPDP (Simple Participant Discovery Protocol) qua địa chỉ `239.255.0.1`, nó bị hệ điều hành rớt gói, khiến Laptop không thể "nhìn thấy" điện thoại.

**Cách xử lý:**
1. Thêm quyền vào `AndroidManifest.xml`: `<uses-permission android:name="android.permission.CHANGE_WIFI_MULTICAST_STATE" />`
2. Kích hoạt `MulticastLock` trong mã nguồn Kotlin (Activity):
   ```kotlin
   val wifiManager = applicationContext.getSystemService(Context.WIFI_SERVICE) as WifiManager
   multicastLock = wifiManager.createMulticastLock("DDS_Multicast_Lock")
   multicastLock.setReferenceCounted(true)
   multicastLock.acquire()
   ```

### 3.2. Vấn đề Mạng Cô lập trên WSL2
**Vấn đề:**
WSL2 mặc định sử dụng kiến trúc NAT (Network Address Translation). WSL2 có dải IP riêng biệt với Windows Host (ví dụ: `172.x.x.x`), khiến nó không thể hứng được gói tin UDP Multicast phát ra từ mạng LAN vật lý (`192.168.0.x`).

**Cách xử lý:**
Sử dụng tính năng **Mirrored Networking** của Windows 11. Cập nhật file `.wslconfig` trên máy chủ Windows để WSL2 nhận chung giao diện mạng vật lý:
```ini
[wsl2]
networkingMode=mirrored
```
Ngoài ra, cần cấu hình mở tường lửa (Firewall) của Windows cho các cổng UDP `7400-8000` theo cả hai chiều Inbound và Outbound.

### 3.3. Lỗi Crash App Android do sai Tên Thư Viện JNI
**Vấn đề:**
Khi ứng dụng khởi chạy trên thiết bị Android, nó bị văng (crash) ngay lập tức với lỗi `UnsatisfiedLinkError`.

**Cách xử lý:**
Nguyên nhân là do tên dự án trong file `CMakeLists.txt` C++ (`project("dsdemo")`) không khớp với lệnh tải thư viện trong Kotlin (`System.loadLibrary("ddsdemo")`). Đã sửa đổi file CMake để đồng nhất tên module `ddsdemo`, giúp Android nhận diện chính xác thư viện `.so`.

## 4. Tổng Kết
Hệ thống DDS Mesh đã hoạt động hoàn toàn theo đúng mô hình phi tập trung. Dữ liệu chạy trực tiếp theo cơ chế Many-to-Many giữa bất kỳ node nào Subscribe vào Topic `DeviceLogTopic`, không đi qua Broker. Kiến trúc hiện tại sẵn sàng để mở rộng quy mô lên hàng chục hoặc hàng trăm thiết bị IoT theo chuẩn công nghiệp.
