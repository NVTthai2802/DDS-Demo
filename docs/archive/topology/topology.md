# DDS Mesh Network Topology

Sơ đồ mạng thể hiện cấu trúc ngang hàng (P2P), không sử dụng Broker trung tâm. Hệ thống tự động khám phá các thiết bị qua giao thức SPDP (Simple Participant Discovery Protocol) sử dụng Multicast UDP, sau đó trao đổi dữ liệu qua SEDP (Simple Endpoint Discovery Protocol).

```mermaid
graph TD
    subgraph "DDS Domain (ID: 0, Topic: SensorData)"
        direction BT
        B[Backend Laptop\nSubscriber (Dashboard)\nIP: 192.168.x.x] 
        P1[Android Phone 1\nPub/Sub\nIP: 192.168.x.101]
        P2[Android Phone 2\nPub/Sub\nIP: 192.168.x.102]
        P3[Android Phone 3\nPub/Sub\nIP: 192.168.x.103]
        P4[Android Phone 4\nPub/Sub\nIP: 192.168.x.104]
        
        P1 <--> |"SPDP / SEDP\n(Multicast/Unicast)"| P2
        P2 <--> |"SPDP / SEDP\n(Multicast/Unicast)"| P3
        P3 <--> |"SPDP / SEDP\n(Multicast/Unicast)"| P4
        P4 <--> |"SPDP / SEDP\n(Multicast/Unicast)"| P1
        P1 <--> |"SPDP / SEDP"| P3
        P2 <--> |"SPDP / SEDP"| P4
        
        P1 --> |"SensorData\n(Reliable / Best Effort)"| B
        P2 --> |"SensorData\n(Reliable / Best Effort)"| B
        P3 --> |"SensorData\n(Reliable / Best Effort)"| B
        P4 --> |"SensorData\n(Reliable / Best Effort)"| B
    end
```
