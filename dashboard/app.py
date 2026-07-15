import streamlit as st
import subprocess
import threading
import json
import time
import pandas as pd
import plotly.express as px
from db import Database
import os
import sys

st.set_page_config(page_title="Mesh DDS Dashboard", layout="wide")

DB_PATH = os.environ.get("DB_PATH", "history.db")
db = Database(DB_PATH)

@st.cache_resource
def start_backend():
    backend_exe = os.path.join(os.path.dirname(__file__), '..', 'backend', 'build', 'backend_node')
    
    # Try WSL2 Ubuntu execution first
    cmd = ['wsl', '-d', 'Ubuntu-24.04', backend_exe]
    if not os.path.exists(backend_exe) or sys.platform != 'win32':
        # On pure Linux or if WSL path not strictly found, try native execution
        cmd = [backend_exe]

    try:
        process = subprocess.Popen(cmd, stdout=subprocess.PIPE, stderr=subprocess.PIPE, text=True)
    except Exception as e:
        st.error(f"Failed to start backend: {e}")
        return None

    def read_output(proc):
        for line in iter(proc.stdout.readline, ''):
            if line:
                try:
                    log = json.loads(line)
                    if log.get('event') == 'spdp':
                        db.update_spdp(log.get('guid'), log.get('name'), log.get('status'))
                    elif log.get('event') == 'sedp':
                        db.update_sedp(log.get('writer_guid'), log.get('status'), log.get('qos_reliability'))
                    elif log.get('event') == 'data':
                        db.insert_data(log)
                except json.JSONDecodeError:
                    pass
                except Exception as e:
                    # Bắt rộng lỗi xử lý logic song song theo yêu cầu, không để luồng sập ngầm
                    print(f"Error handling data line: {e}")

    t = threading.Thread(target=read_output, args=(process,), daemon=True)
    t.start()
    return process

start_backend()

st.title("Bee Labs: Mesh DDS P2P Dashboard")

col1, col2 = st.columns([1, 2])

with col1:
    st.subheader("Runtime & Discovery (SPDP vs SEDP)")
    peers_df = db.get_peers_df()
    if not peers_df.empty:
        display_df = peers_df[['name', 'guid', 'status', 'sedp_matched', 'qos', 'last_seen']].copy()
        
        def highlight_sedp(row):
            if row['sedp_matched']:
                return ['background-color: rgba(144, 238, 144, 0.2)'] * len(row)
            elif row['status'] == 'DISCOVERED':
                return ['background-color: rgba(255, 255, 224, 0.2)'] * len(row)
            else:
                return ['background-color: rgba(240, 128, 128, 0.2)'] * len(row)
                
        st.dataframe(display_df.style.apply(highlight_sedp, axis=1), use_container_width=True)
        st.caption("🟢 SEDP Matched (Data Flowing) | 🟡 SPDP Joined (Metadata only) | 🔴 Offline")
    else:
        st.info("No peers discovered yet.")

with col2:
    st.subheader("Sensor Data & Analytics")
    data_df = db.get_latest_data_df(500)
    if not data_df.empty:
        st.markdown("**Relative Latency (ms) (Receive Time - Sample Time)**")
        fig_lat = px.line(data_df, x='id', y='latency_ms', color='device_id', markers=True)
        st.plotly_chart(fig_lat, use_container_width=True)
        
        col_t, col_h = st.columns(2)
        with col_t:
            st.markdown("**Temperature (°C)**")
            fig_temp = px.line(data_df, x='id', y='temperature', color='device_id')
            st.plotly_chart(fig_temp, use_container_width=True)
        with col_h:
            st.markdown("**Humidity (%)**")
            fig_hum = px.line(data_df, x='id', y='humidity', color='device_id')
            st.plotly_chart(fig_hum, use_container_width=True)
            
    else:
        st.info("No data received yet.")

time.sleep(2)
st.rerun()
