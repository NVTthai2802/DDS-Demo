import streamlit as st
import pandas as pd
import time
import plotly.express as px
import plotly.graph_objects as go
from collections import deque
from dds_subscriber import DDSSubscriberThread

# --- Cấu hình trang ---
st.set_page_config(page_title="Smart Environment DDS Dashboard", page_icon="🌿", layout="wide")
st.title("🌿 P2P DDS: Smart Environment Monitor")
st.markdown("Giám sát dữ liệu cảm biến đa phòng thời gian thực qua giao thức phân tán DDS.")

# --- Khởi tạo hệ thống ---
@st.cache_resource
def get_dds_system():
    # Giới hạn 2000 bản tin gần nhất
    data_queue = deque(maxlen=2000)
    dds_thread = DDSSubscriberThread(data_queue)
    dds_thread.start()
    return data_queue, dds_thread

data_queue, dds_thread = get_dds_system()

refresh_rate = 1.0 # Tần số làm mới 1 giây (1 Hz)

# --- Xử lý Dữ liệu ---
if len(data_queue) > 0:
    df = pd.DataFrame(list(data_queue))
    
    # Lấy dữ liệu mới nhất theo từng phòng (device_id)
    latest_df = df.sort_values('sequence_number').groupby('device_id').tail(1)
    
    # --- 1. NETWORK & QoS METRICS ---
    st.header("📡 DDS Network & QoS")
    col1, col2, col3, col4 = st.columns(4)
    
    total_msgs = len(df)
    current_time = time.time()
    
    # Tính Throughput (msg/s) dựa trên 5 giây gần nhất
    recent_msgs = df[df['receive_time'] > (current_time - 5.0)]
    throughput = len(recent_msgs) / 5.0 if len(recent_msgs) > 0 else 0.0
    
    # Tính Packet Loss toàn hệ thống
    df_sorted = df.sort_values(by=["device_id", "sequence_number"])
    df_sorted['seq_diff'] = df_sorted.groupby('device_id')['sequence_number'].diff()
    lost_packets = df_sorted[df_sorted['seq_diff'] > 1]['seq_diff'].sum() - len(df_sorted[df_sorted['seq_diff'] > 1])
    loss_rate = (lost_packets / (total_msgs + lost_packets)) * 100 if total_msgs > 0 else 0
    
    col1.metric("Tổng Message Nhận", f"{total_msgs}")
    col2.metric("Throughput (msg/s)", f"{throughput:.1f}")
    col3.metric("Packet Loss Rate (%)", f"{loss_rate:.2f} %")
    col4.metric("QoS Policy", "Reliable")

    # --- 2. THÔNG TIN THIẾT BỊ (DISCOVERY) ---
    st.header("📱 Quản lý Thiết bị (Discovery)")
    # Giả định nếu không nhận được tin nào trong 5 giây là Offline
    latest_df['status'] = latest_df['receive_time'].apply(lambda t: '🟢 Online' if (current_time - t) < 5.0 else '🔴 Offline')
    
    device_cols = st.columns(len(latest_df))
    for idx, row in latest_df.iterrows():
        with device_cols[idx % len(device_cols)]:
            st.info(f"**{row['device_id']}**\n\n"
                    f"Loại: {row['device_type']}\n\n"
                    f"Trạng thái: {row['status']}\n\n"
                    f"Pin: 🔋 {row['battery']}%\n\n"
                    f"Sóng: 📶 {row['signal_strength']} dBm")

    # --- 3. MÔI TRƯỜNG TỪNG PHÒNG ---
    st.header("🌡️ Thông số Môi trường Real-time")
    
    # Vẽ Line Chart cho Nhiệt độ và Độ ẩm
    col_env1, col_env2 = st.columns(2)
    with col_env1:
        fig_temp = px.line(df, x='receive_time', y='temperature', color='device_id', title='Nhiệt độ (°C)')
        fig_temp.update_layout(xaxis_title="Thời gian", yaxis_title="°C")
        st.plotly_chart(fig_temp, use_container_width=True)
        
    with col_env2:
        fig_hum = px.line(df, x='receive_time', y='humidity', color='device_id', title='Độ ẩm (%)')
        fig_hum.update_layout(xaxis_title="Thời gian", yaxis_title="%")
        st.plotly_chart(fig_hum, use_container_width=True)

    col_env3, col_env4 = st.columns(2)
    with col_env3:
        fig_co2 = px.line(df, x='receive_time', y='co2', color='device_id', title='Nồng độ CO2 (ppm)')
        st.plotly_chart(fig_co2, use_container_width=True)
        
    with col_env4:
        fig_lat = px.line(df, x='receive_time', y='latency_ms', color='device_id', title='Độ trễ truyền dẫn (Latency - ms)')
        st.plotly_chart(fig_lat, use_container_width=True)

    # --- 4. BẢNG DỮ LIỆU THÔ ---
    st.subheader("Dữ liệu thô (Raw Data)")
    st.dataframe(df.tail(15).sort_values(by="sequence_number", ascending=False), use_container_width=True)

else:
    st.info("⏳ Đang chờ thiết bị tham gia Domain và gửi dữ liệu Môi trường...")

time.sleep(refresh_rate)
st.rerun()
