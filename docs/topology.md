# DDS Mesh Demo - Topology
This diagram describes the network and application topology for the DDS Mesh Demo system.

```mermaid
graph TD
    subgraph "Wi-Fi Network (Multicast-enabled)"
        subgraph "Laptop (Dashboard)"
            DashboardUI[Streamlit Dashboard]
            BackendNode[C++ Backend Subscriber]
            DashboardUI -- "Runs" --> BackendNode
            BackendNode -- "Logs via stdout" --> DashboardUI
        end
        
        subgraph "Android Devices"
            Phone1[Phone 1: Publisher & Subscriber]
            Phone2[Phone 2: Publisher & Subscriber]
            Phone3[Phone 3: Publisher & Subscriber]
        end
        
        BackendNode -. "SPDP / SEDP (UDP)" .-> Phone1
        BackendNode -. "SPDP / SEDP (UDP)" .-> Phone2
        Phone1 -. "SensorData (RTPS/UDP)" .-> BackendNode
        Phone2 -. "SensorData (RTPS/UDP)" .-> BackendNode
        Phone3 -. "SensorData (RTPS/UDP)" .-> BackendNode
        
        Phone1 -. "SensorData" .-> Phone2
        Phone2 -. "SensorData" .-> Phone3
        Phone3 -. "SensorData" .-> Phone1
    end
```

## Details
- **Domain ID:** 0 (Default)
- **Topic:** SensorData
- **Discovery Mechanism:** Simple Participant Discovery Protocol (SPDP) over UDPv4 Multicast
- **Data Exchange:** Simple Endpoint Discovery Protocol (SEDP) over RTPS
- **Reliability QoS:**
  - Android Publisher: RELIABLE
  - Android Subscriber: RELIABLE
  - Backend Subscriber: BEST_EFFORT (or RELIABLE depending on config)
- **Liveliness QoS:** AUTOMATIC (Lease Duration 3s, Announcement Period 1s)
