import streamlit as st
import time
from services.dds_service import DDSSubscriberThread
from services.statistics_service import StatisticsService
from views.dds_runtime_view import render_dds_runtime_view
from views.qos_analytics_view import render_qos_analytics_view
from views.sensor_data_view import render_sensor_data_view

st.set_page_config(page_title="DDS Demonstration Platform", page_icon="📡", layout="wide")
st.title("📡 DDS Demonstration Platform (Phase 5)")
st.markdown("Nền tảng trực quan hoá kiến trúc mạng, khả năng tự động khám phá (Discovery) và chất lượng dịch vụ (QoS) của giao thức Data Distribution Service.")

@st.cache_resource
def get_dds_system():
    dds_thread = DDSSubscriberThread()
    dds_thread.start()
    return dds_thread

@st.cache_resource
def get_statistics_service():
    return StatisticsService("dds_demo.db")

dds_thread = get_dds_system()
stats_service = get_statistics_service()

# Fetch data
df_data = stats_service.fetch_recent_data()
df_participants = stats_service.fetch_participants()
df_events = stats_service.fetch_discovery_events()

current_time = time.time()
total_msgs, throughput, loss_rate, lost_packets = stats_service.get_qos_metrics(df_data, current_time)

# TABS
tab1, tab2, tab3 = st.tabs(["🌐 DDS Runtime & Discovery", "📊 QoS & Analytics", "🌡️ Sensor Data"])

with tab1:
    render_dds_runtime_view(df_participants, df_events, current_time)

with tab2:
    render_qos_analytics_view(df_data, total_msgs, throughput, loss_rate, lost_packets)

with tab3:
    render_sensor_data_view(df_data)

time.sleep(1.0)
st.rerun()
