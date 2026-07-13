import streamlit as st
import pandas as pd
import time
import sqlite3
import plotly.express as px
from dds_subscriber import DDSSubscriberThread

st.set_page_config(page_title="DDS Demonstration Platform", page_icon="📡", layout="wide")
st.title("📡 Nền tảng Học và Trình diễn DDS (Phase 2)")
st.markdown("Giám sát cấu trúc mạng phân tán, Discovery và hiệu năng QoS của giao thức DDS.")

@st.cache_resource
def get_dds_system():
    dds_thread = DDSSubscriberThread("dds_demo.db")
    dds_thread.start()
    return dds_thread

dds_thread = get_dds_system()

# --- Load SQLite Data ---
conn = sqlite3.connect("dds_demo.db", timeout=5)
try:
    df_data = pd.read_sql_query("SELECT * FROM sensor_data ORDER BY receive_time DESC LIMIT 2000", conn)
    df_participants = pd.read_sql_query("SELECT * FROM participants", conn)
except:
    df_data = pd.DataFrame()
    df_participants = pd.DataFrame()
conn.close()

# --- TABS ---
tab1, tab2, tab3 = st.tabs(["🌐 Topology & Discovery", "📊 QoS & Analytics", "🌡️ Sensor Data"])

current_time = time.time()

with tab1:
    st.header("Mạng DDS hiện tại (Domain 0)")
    
    if not df_participants.empty:
        # Check liveliness (3 seconds lease duration + 2s grace)
        df_participants['is_online'] = df_participants.apply(
            lambda x: True if (x['status'] == 'joined' and (current_time - x['last_seen'] < 5.0)) else False, axis=1
        )
        
        online_count = df_participants['is_online'].sum()
        st.metric("Total Online Participants", int(online_count))
        
        st.subheader("Danh sách Participant")
        for idx, row in df_participants.iterrows():
            status_icon = "🟢" if row['is_online'] else "🔴"
            status_text = "Online" if row['is_online'] else "Offline (Liveliness Lost/Left)"
            st.info(f"{status_icon} **{row['name']}**\n\nGUID: `{row['guid']}`\n\nTrạng thái: {status_text}")
    else:
        st.info("Chưa phát hiện Participant nào tham gia mạng.")

with tab2:
    st.header("Hiệu năng và QoS")
    col1, col2, col3, col4 = st.columns(4)
    
    if not df_data.empty:
        total_msgs = len(df_data)
        
        recent_msgs = df_data[df_data['receive_time'] > (current_time - 5.0)]
        throughput = len(recent_msgs) / 5.0 if len(recent_msgs) > 0 else 0.0
        
        df_sorted = df_data.sort_values(by=["device_id", "sequence_number"])
        df_sorted['seq_diff'] = df_sorted.groupby('device_id')['sequence_number'].diff()
        lost_packets = df_sorted[df_sorted['seq_diff'] > 1]['seq_diff'].sum() - len(df_sorted[df_sorted['seq_diff'] > 1])
        loss_rate = (lost_packets / (total_msgs + lost_packets)) * 100 if total_msgs > 0 else 0
        
        col1.metric("Tổng bản tin nhận", f"{total_msgs}")
        col2.metric("Throughput (msg/s)", f"{throughput:.1f}")
        col3.metric("Tỉ lệ Packet Loss", f"{loss_rate:.2f} %")
        col4.metric("QoS Config", "Reliable (History=10)")
        
        st.subheader("Phân tích Latency (Độ trễ truyền dẫn)")
        fig_lat = px.box(df_data, x="device_id", y="latency_ms", title="Độ phân tán Latency theo Node")
        st.plotly_chart(fig_lat, use_container_width=True)
    else:
        st.info("Chưa có dữ liệu để phân tích QoS.")

with tab3:
    st.header("Luồng Dữ liệu (User Data)")
    if not df_data.empty:
        col_env1, col_env2 = st.columns(2)
        with col_env1:
            fig_temp = px.line(df_data, x='receive_time', y='temperature', color='device_id', title='Nhiệt độ (°C)')
            st.plotly_chart(fig_temp, use_container_width=True)
            
        with col_env2:
            fig_seq = px.scatter(df_data, x='receive_time', y='sequence_number', color='device_id', title='Sequence Tracking')
            st.plotly_chart(fig_seq, use_container_width=True)
            
        st.subheader("Bản tin Raw")
        st.dataframe(df_data.head(10), use_container_width=True)
    else:
        st.info("Chưa nhận được Sensor Data nào qua DDS.")

time.sleep(1.0)
st.rerun()
