import streamlit as st
import plotly.express as px

def render_qos_analytics_view(df_data, total_msgs, throughput, loss_rate, lost_packets):
    st.header("📊 Hiệu năng và QoS")
    
    st.subheader("QoS Profile")
    col_q1, col_q2, col_q3 = st.columns(3)
    col_q1.info("**Reliability:** RELIABLE\n\n*(Đảm bảo không mất gói, tự động gửi lại)*")
    col_q2.info("**History:** KEEP_LAST\n\n**Depth:** 10\n*(Giữ lại 10 mẫu tin gần nhất)*")
    col_q3.info("**Liveliness:** AUTOMATIC\n\n**Lease Duration:** 3s\n*(Báo mất kết nối sau 3s)*")
    
    st.markdown("---")
    
    st.subheader("Mạng & Chuyển mạch")
    col1, col2, col3, col4 = st.columns(4)
    col1.metric("Tổng bản tin nhận", f"{total_msgs}")
    col2.metric("Throughput (msg/s)", f"{throughput:.1f}")
    col3.metric("Số gói mất (Drop)", f"{lost_packets}")
    col4.metric("Tỉ lệ Packet Loss", f"{loss_rate:.2f} %")
    
    st.subheader("Phân tích Latency (Độ trễ truyền dẫn)")
    if not df_data.empty:
        fig_lat = px.box(df_data, x="device_id", y="latency_ms", title="Độ phân tán Latency theo Node")
        st.plotly_chart(fig_lat, use_container_width=True)
    else:
        st.info("Chưa có dữ liệu để phân tích QoS.")
