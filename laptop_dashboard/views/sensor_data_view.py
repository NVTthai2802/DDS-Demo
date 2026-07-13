import streamlit as st
import plotly.express as px

def render_sensor_data_view(df_data):
    st.header("🌡️ Dữ liệu Cảm biến (Tải trọng)")
    st.markdown("Phần này minh họa cho giá trị mang tải thực tế (payload) đang được truyền tải qua DDS.")
    
    if not df_data.empty:
        col_env1, col_env2 = st.columns(2)
        with col_env1:
            fig_temp = px.line(df_data, x='receive_time', y='temperature', color='device_id', title='Biến thiên Nhiệt độ (°C)')
            st.plotly_chart(fig_temp, use_container_width=True)
            
        with col_env2:
            fig_seq = px.scatter(df_data, x='receive_time', y='sequence_number', color='device_id', title='Sequence Tracking')
            st.plotly_chart(fig_seq, use_container_width=True)
            
        st.subheader("Bản tin Raw (10 mẫu mới nhất)")
        st.dataframe(df_data.head(10), use_container_width=True)
    else:
        st.info("Chưa nhận được Sensor Data nào qua DDS.")
