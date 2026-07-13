# Sơ đồ Topology Mạng: P2P DDS Mesh (Giai đoạn 3)

Dưới đây là sơ đồ kiến trúc mạng DDS Mesh không máy chủ (Brokerless) cho hệ thống gồm 1 Laptop và 4 Phone.

## Thông số Mạng Chung
- **Wi-Fi Access Point (LAN):** Subnet `192.168.0.0/24`
- **DDS Domain ID:** `0` (Tất cả thiết bị phải cùng Domain ID để thấy nhau)
- **Giao thức Discovery (SPDP):** UDP Multicast địa chỉ `239.255.0.1`, Cổng `7400`
- **Giao thức Dữ liệu (SEDP/User Data):** UDP Unicast P2P trực tiếp giữa các thiết bị.

## Sơ đồ Mermaid Topology

```mermaid
graph TD
    %% Mạng Wi-Fi trung tâm
    WIFI(("Wi-Fi Router / Access Point<br/>Subnet: 192.168.0.0/24<br/>(IGMP Snooping Enabled)"))

    %% Khai báo các thiết bị
    subgraph "DDS Domain 0 (Mạng Mesh P2P)"
        
        %% Laptop Subscriber
        LAPTOP["💻 Laptop (Windows 11 / WSL2)<br/>IP: 192.168.0.114<br/>Vai trò: Subscriber<br/>App: Streamlit Dashboard"]
        
        %% Các Phone Publishers
        P1["📱 Phone 1<br/>IP: 192.168.0.101<br/>Vai trò: Publisher<br/>Device ID: MeetingRoom-01"]
        P2["📱 Phone 2<br/>IP: 192.168.0.102<br/>Vai trò: Publisher<br/>Device ID: MeetingRoom-02"]
        P3["📱 Phone 3<br/>IP: 192.168.0.103<br/>Vai trò: Publisher<br/>Device ID: MeetingRoom-03"]
        P4["📱 Phone 4<br/>IP: 192.168.0.104<br/>Vai trò: Publisher<br/>Device ID: MeetingRoom-04"]

        %% Kết nối vật lý tới Wi-Fi
        WIFI --- LAPTOP
        WIFI --- P1
        WIFI --- P2
        WIFI --- P3
        WIFI --- P4

        %% Discovery Traffic (Multicast)
        P1 -. "SPDP Multicast<br/>(239.255.0.1:7400)" .-> LAPTOP
        P2 -. "SPDP Multicast" .-> LAPTOP
        P3 -. "SPDP Multicast" .-> LAPTOP
        P4 -. "SPDP Multicast" .-> LAPTOP
        
        %% Data Traffic (Unicast P2P)
        P1 == "Data (UDP Unicast)<br/>Topic: DeviceLogTopic" ==> LAPTOP
        P2 == "Data (UDP Unicast)" ==> LAPTOP
        P3 == "Data (UDP Unicast)" ==> LAPTOP
        P4 == "Data (UDP Unicast)" ==> LAPTOP
    end

    %% Định dạng màu sắc
    style WIFI fill:#f9f0ff,stroke:#9c27b0,stroke-width:2px
    style LAPTOP fill:#e3f2fd,stroke:#1976d2,stroke-width:2px
    style P1 fill:#e8f5e9,stroke:#388e3c,stroke-width:2px
    style P2 fill:#e8f5e9,stroke:#388e3c,stroke-width:2px
    style P3 fill:#e8f5e9,stroke:#388e3c,stroke-width:2px
    style P4 fill:#e8f5e9,stroke:#388e3c,stroke-width:2px
```

## Giải thích chi tiết vai trò (Roles)

1. **📱 Phone 1, 2, 3, 4 (Android 14)**
   - **Vai trò DDS:** `Publisher` (DataWriter)
   - **Nhiệm vụ:** Hoạt động như các trạm cảm biến môi trường độc lập. Liên tục sinh ra dữ liệu giả lập (Nhiệt độ, Độ ẩm, CO2, v.v.) và đẩy (publish) vào Topic `DeviceLogTopic`.
   - **Định danh (Device ID):** Lần lượt là `MeetingRoom-01`, `MeetingRoom-02`, `MeetingRoom-03`, `MeetingRoom-04`.
   - **Đặc tả Mạng:** Đã xin cấp quyền `MulticastLock` để có thể nhận và gửi bản tin Discovery đa hướng. Khi mới kết nối mạng, chúng sẽ rải bản tin ra toàn bộ mạng LAN để tìm kiếm Subscriber.

2. **💻 Laptop (WSL2 Ubuntu 24.04)**
   - **Vai trò DDS:** `Subscriber` (DataReader)
   - **Nhiệm vụ:** Khởi chạy Fast DDS Backend (C++) để lắng nghe thụ động trên Topic `DeviceLogTopic`. Bất cứ gói tin nào từ 4 phòng (Phone) bay tới sẽ được bắt lấy, chuyển thành JSON và đẩy lên màn hình Streamlit Dashboard.
   - **Đặc tả Mạng:** Nhờ chế độ `Mirrored Networking`, WSL2 nhận chung IP `192.168.0.114` của máy chủ Windows và có thể "bắt sóng" (sniff) được các bản tin Multicast từ điện thoại gửi vào mạng LAN.

> [!TIP]
> **Đặc quyền của mạng Mesh (DDS):** Không có thiết bị nào đóng vai trò là "Máy chủ trung tâm" (Broker/Server). Nếu Laptop (Dashboard) bị tắt, 4 chiếc điện thoại vẫn hoạt động và duy trì kết nối mạng bình thường. Ngay khi Laptop mở lại, chúng sẽ tự động khám phá (Auto-Discovery) ra nhau và tiếp tục luồng dữ liệu ngay lập tức (Plug & Play).
