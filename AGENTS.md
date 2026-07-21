# DDS Mesh Network - Workspace Agent Configuration

## Tiêu Chí Nghiệm Thu (Acceptance Criteria)
- Đảm bảo thiết bị Android chạy DDS Discovery ngang hàng (SPDP/SEDP).
- QoS Liveliness hoạt động đúng để detect node offline/online.

## Tiến Độ Roadmap 6 Giai Đoạn
- **Giai đoạn 1:** Bảng so sánh middleware, sơ đồ topology dự kiến (Chưa hoàn thành bảng so sánh).
- **Giai đoạn 2:** Build middleware trên laptop + Android (Hoàn thành).
- **Giai đoạn 3:** Xác nhận Discovery tự động cả 5 thiết bị (Chưa có log kiểm chứng Phase C).
- **Giai đoạn 4:** App demo many-to-many hoàn chỉnh, dashboard laptop (Hoàn thành code).
- **Giai đoạn 5:** Kết quả đo latency, thử nghiệm QoS, fault tolerance (Chưa có log thực tế).
- **Giai đoạn 6:** Báo cáo kỹ thuật, đề xuất ứng dụng (Hoàn thành file MD/HTML, thiếu PDF/Word/Video).

## Danh Sách Bug Đã Biết
1. Chưa có bug nào được ghi nhận (Cần test Phase B và C để cập nhật).

## Trạng Thái Deliverables (Cập Nhật Sau Kiểm Toán)
| Hạng mục | Trạng thái | Ghi chú / Bằng chứng |
|----------|------------|---------------------|
| Source code đầy đủ + README | Done | Code Native/JNI và `README.md` đã có mặt ở root. |
| Sơ đồ topology THỰC TẾ | Partial | `docs/topology.md` đã hoàn thiện cấu trúc, chỉ thiếu IP thật từ Phase C. |
| Báo cáo kỹ thuật (Word/PDF) | Partial | Đã có `Bao_Cao_Ky_Thuat_Bee_Labs_v2.md`, nhưng chưa convert ra PDF/Word. |
| Video demo 2-5 phút | Missing | Chưa có file mp4/video demo kết quả test vật lý. |
| Slide trình bày HTML | Done | Đã có `docs/slides.html`. |
