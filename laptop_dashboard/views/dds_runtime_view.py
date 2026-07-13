import streamlit as st
import time
from datetime import datetime

def render_dds_runtime_view(df_participants, df_events, current_time):
    st.header("🌐 Mạng DDS hiện tại (Domain 0)")
    
    col_domain, col_participants = st.columns([1, 2])
    
    with col_domain:
        st.subheader("Domain Monitor")
        st.info("**Domain ID:** 0\n\n**Transport:** UDPv4\n\n**Middleware:** Fast DDS\n\n**Discovery:** SPDP (Simple Participant Discovery)")
        
        st.subheader("Topic Explorer")
        st.success("**Topic Name:** DeviceLogTopic\n\n**Type Name:** DeviceLog\n\n**Key:** None")

    with col_participants:
        st.subheader("Participant Monitor")
        if not df_participants.empty:
            df_participants['is_online'] = df_participants.apply(
                lambda x: True if (x['status'] == 'joined' and (current_time - x['last_seen'] < 5.0)) else False, axis=1
            )
            
            online_count = df_participants['is_online'].sum()
            st.metric("Total Online Participants", int(online_count))
            
            for idx, row in df_participants.iterrows():
                status_icon = "🟢" if row['is_online'] else "🔴"
                status_text = "Online" if row['is_online'] else "Offline"
                st.markdown(f"**{status_icon} {row['name']}** - `{row['guid']}` - {status_text}")
        else:
            st.info("Chưa phát hiện Participant nào tham gia mạng.")
            
    st.markdown("---")
    st.subheader("Discovery Timeline")
    
    if not df_events.empty:
        df_events['time_str'] = df_events['timestamp'].apply(lambda x: datetime.fromtimestamp(x).strftime('%H:%M:%S'))
        
        for idx, row in df_events.head(15).iterrows():
            icon = "🔌" if row['event'] == 'joined' else "❌" if row['event'] == 'left' else "🔗"
            st.text(f"[{row['time_str']}] {icon} {row['entity'].capitalize()} '{row['name']}' ({row['guid']}) {row['event']}.")
    else:
        st.info("Chưa có sự kiện Discovery nào được ghi nhận.")
