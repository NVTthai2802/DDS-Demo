# Checklist Kiểm Thử Vật Lý (DDS Mesh Demo)

**Quy mô:** 1 Laptop (Dashboard) + 4 Điện thoại Android.
**Mạng:** Cùng 1 mạng Wi-Fi (Lưu ý: Router phải hỗ trợ và đang bật tính năng UDP Multicast).

## 1. Chuẩn Bị (Preparation)
- [ ] Laptop đã build thành công `backend_node` (Native Windows 11 bằng MSVC).
- [ ] Cài đặt gói thư viện Python cho Dashboard (`pip install -r dashboard/requirements.txt`).
- [ ] Build thành công file APK (`android_app`) và cài đặt lên cả 4 điện thoại Android.
- [ ] Kết nối tất cả thiết bị (1 Laptop + 4 Phone) vào **cùng một mạng Wi-Fi**.

## 2. Giai đoạn B: Phân Tán Hẹp (1 Laptop + 2 Phone)
- [ ] Bật Dashboard trên Laptop (`streamlit run dashboard/app.py`). Đảm bảo bảng Peers trống.
- [ ] Mở App trên Phone 1. Nhập ID `Phone-A`. Bấm **Start**.
  - [ ] Kiểm tra Dashboard: Hiện SPDP Discovered, sau đó SEDP Matched màu xanh lá cây.
  - [ ] Dashboard có biểu đồ Latency, Temperature, Humidity của `Phone-A`.
- [ ] Mở App trên Phone 2. Nhập ID `Phone-B`. Bấm **Start**.
  - [ ] Kiểm tra Dashboard: `Phone-B` xuất hiện, báo Matched. Biểu đồ có 2 line.
  - [ ] Kiểm tra màn hình Phone 1: Phải có dòng log báo nhận được dữ liệu từ `Phone-B` kèm Latency.
  - [ ] Kiểm tra màn hình Phone 2: Phải có dòng log báo nhận được dữ liệu từ `Phone-A` kèm Latency.

## 3. Giai đoạn C: Full Scale (1 Laptop + 4 Phone)
- [ ] Mở App trên Phone 3 và Phone 4. Bấm Start.
  - [ ] Dashboard nhận đủ 4 thiết bị.
  - [ ] Bảng Packet Loss (ở dưới biểu đồ) hiển thị Loss % của cả 4 thiết bị với con số sát 0%.
  - [ ] Log trên tất cả Phone đều chạy liên tục (nhận được dữ liệu chéo nhau, thể hiện tính Many-to-many).

## 4. Kiểm Thử Khả Năng Chịu Lỗi (Fault Tolerance)
- [ ] Tắt Wi-Fi trên Phone 2 (hoặc đóng hoàn toàn App).
  - [ ] Đợi tối đa 3-4 giây (theo Lease Duration).
  - [ ] Dashboard tự động cập nhật trạng thái của `Phone-B` thành **OFFLINE** (màu đỏ).
- [ ] Bật lại Wi-Fi và App trên Phone 2.
  - [ ] Dashboard cập nhật lại trạng thái thành **MATCHED** (màu xanh).
  - [ ] Dữ liệu tiếp tục chảy trên biểu đồ.

## 5. Tiêu Chí Nghiệm Thu Khác
- [ ] Chạy trong 5 phút để kiểm tra độ ổn định, không thiết bị nào bị crash / ANR.
- [ ] Cáp điện thoại có thể rút ra (không cần cắm vào laptop) để chứng minh giao tiếp hoàn toàn qua mạng không dây.
