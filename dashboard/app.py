import streamlit as st
import subprocess
import threading
import json
import time
from datetime import datetime
import pandas as pd
import plotly.express as px
import os
import sys
from dataclasses import dataclass, field
from collections import deque

st.set_page_config(page_title="Mesh DDS Dashboard", layout="wide")

@dataclass
class MeshState:
    peers: dict = field(default_factory=dict)
    sensor_data: deque = field(default_factory=lambda: deque(maxlen=2000))
    guid_to_name: dict = field(default_factory=dict)

@st.cache_resource
def get_state():
    return MeshState()

state = get_state()

def get_prefix(guid_str):
    return guid_str.split('|')[0] if '|' in guid_str else guid_str

@st.cache_resource
def start_backend():
    # Use native Windows executable directly since we dropped WSL2
    backend_exe = os.path.join(os.path.dirname(__file__), '..', 'backend', 'build_app', 'backend_node.exe')
    cmd = [backend_exe]

    try:
        process = subprocess.Popen(cmd, stdout=subprocess.PIPE, stderr=subprocess.STDOUT, text=True, encoding='utf-8', errors='replace')
    except Exception as e:
        st.error(f"Failed to start backend: {e}")
        return None

    def read_output(proc):
        with open("dashboard_error.log", "a", encoding="utf-8") as log_file:
            for line in iter(proc.stdout.readline, ''):
                if line:
                    try:
                        log = json.loads(line)
                        event = log.get('event')
                        now = datetime.now()
                        
                        if event == 'spdp':
                            guid = log.get('guid')
                            name = log.get('name')
                            status = log.get('status')
                            
                            if guid:
                                prefix = get_prefix(guid)
                                if name:
                                    state.guid_to_name[prefix] = name
                                
                                if status == "DISCOVERED" and name:
                                    if name not in state.peers:
                                        state.peers[name] = {
                                            'name': name, 'prefix': prefix, 'guid': guid, 
                                            'status': status, 'sedp_matched': False, 
                                            'qos': None, 'last_seen': now
                                        }
                                    else:
                                        state.peers[name]['status'] = status
                                        state.peers[name]['last_seen'] = now
                                        state.peers[name]['prefix'] = prefix
                                        state.peers[name]['guid'] = guid
                                elif status == "REMOVED" and name in state.peers:
                                    state.peers[name]['status'] = "OFFLINE"
                                    state.peers[name]['last_seen'] = now
                                    state.peers[name]['sedp_matched'] = False
                                    
                        elif event == 'sedp':
                            writer_guid = log.get('writer_guid')
                            status = log.get('status')
                            qos = log.get('qos_reliability')
                            if writer_guid:
                                prefix = get_prefix(writer_guid)
                                name = state.guid_to_name.get(prefix)
                                if name and name in state.peers:
                                    is_matched = (status == "MATCHED")
                                    state.peers[name]['sedp_matched'] = is_matched
                                    state.peers[name]['qos'] = qos
                                    state.peers[name]['last_seen'] = now
                                    
                        elif event == 'liveliness':
                            writer_guid = log.get('writer_guid')
                            alive = log.get('alive_count_change', 0)
                            not_alive = log.get('not_alive_count_change', 0)
                            if writer_guid:
                                prefix = get_prefix(writer_guid)
                                name = state.guid_to_name.get(prefix)
                                if name and name in state.peers:
                                    if not_alive > 0:
                                        state.peers[name]['status'] = "OFFLINE"
                                        state.peers[name]['last_seen'] = now
                                        state.peers[name]['sedp_matched'] = False
                                    elif alive > 0:
                                        state.peers[name]['status'] = "DISCOVERED"
                                        state.peers[name]['last_seen'] = now
                                        state.peers[name]['sedp_matched'] = True
                                        
                        elif event == 'data':
                            latency = log['receive_time'] - log['timestamp']
                            data_entry = {
                                'id': len(state.sensor_data) + 1,
                                'device_id': log['device_id'],
                                'sequence_number': log['sequence_number'],
                                'timestamp': log['timestamp'],
                                'temperature': log['temperature'],
                                'humidity': log['humidity'],
                                'co2': log.get('co2', 0),
                                'light': log.get('light', 0),
                                'occupancy': log.get('occupancy', False),
                                'battery': log.get('battery', 0),
                                'signal_strength': log.get('signal_strength', 0),
                                'latency_ms': latency,
                                'receive_time': log['receive_time']
                            }
                            state.sensor_data.append(data_entry)
                            
                            writer_guid = log.get('writer_guid')
                            if writer_guid:
                                prefix = get_prefix(writer_guid)
                                name = state.guid_to_name.get(prefix)
                                if name and name in state.peers:
                                    state.peers[name]['last_seen'] = now
                                    state.peers[name]['status'] = 'DISCOVERED'
                                    state.peers[name]['sedp_matched'] = True
                                    
                    except json.JSONDecodeError:
                        pass
                    except Exception as e:
                        # Ghi log ra file để dễ debug thay vì print ngầm
                        log_file.write(f"[{time.strftime('%Y-%m-%d %H:%M:%S')}] Error handling data line: {e}\n")
                        log_file.write(f"Line content: {line.strip()}\n")
                        log_file.flush()

    t_stdout = threading.Thread(target=read_output, args=(process,), daemon=True)
    t_stdout.start()
    
    return process

start_backend()

st.title("Bee Labs: Mesh DDS P2P Dashboard")

col1, col2 = st.columns([1, 2])

with col1:
    st.subheader("Runtime & Discovery (SPDP vs SEDP)")
    peers_df = pd.DataFrame(list(state.peers.values()))
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
    data_df = pd.DataFrame(list(state.sensor_data))
    if not data_df.empty:
        # Get the latest 500 rows and sort descending by id like the SQL query did
        data_df_limit = data_df.tail(500).sort_values('id', ascending=False)
        
        st.markdown("**Relative Latency (ms) (Receive Time - Sample Time)**")
        fig_lat = px.line(data_df_limit, x='id', y='latency_ms', color='device_id', markers=True)
        st.plotly_chart(fig_lat, use_container_width=True)
        
        col_t, col_h = st.columns(2)
        with col_t:
            st.markdown("**Temperature (°C)**")
            fig_temp = px.line(data_df_limit, x='id', y='temperature', color='device_id')
            st.plotly_chart(fig_temp, use_container_width=True)
        with col_h:
            st.markdown("**Humidity (%)**")
            fig_hum = px.line(data_df_limit, x='id', y='humidity', color='device_id')
            st.plotly_chart(fig_hum, use_container_width=True)
            
        col_c, col_l = st.columns(2)
        with col_c:
            st.markdown("**CO2 Levels (ppm)**")
            fig_co2 = px.line(data_df_limit, x='id', y='co2', color='device_id')
            st.plotly_chart(fig_co2, use_container_width=True)
        with col_l:
            st.markdown("**Ambient Light (lux)**")
            fig_light = px.line(data_df_limit, x='id', y='light', color='device_id')
            st.plotly_chart(fig_light, use_container_width=True)
            
        col_b, col_s = st.columns(2)
        with col_b:
            st.markdown("**Battery Level (%)**")
            fig_bat = px.line(data_df_limit, x='id', y='battery', color='device_id')
            st.plotly_chart(fig_bat, use_container_width=True)
        with col_s:
            st.markdown("**Signal Strength (dBm)**")
            fig_sig = px.line(data_df_limit, x='id', y='signal_strength', color='device_id')
            st.plotly_chart(fig_sig, use_container_width=True)
            
        st.markdown("**Packet Loss & Reliability**")
        loss_data = []
        for device_id, df_group in data_df.groupby('device_id'):
            if 'sequence_number' in df_group.columns:
                expected = int(df_group['sequence_number'].max() - df_group['sequence_number'].min() + 1)
                received = len(df_group)
                loss = max(0, expected - received)
                loss_percent = round((loss * 100.0 / expected), 2) if expected > 0 else 0.0
                loss_data.append({
                    'device_id': device_id,
                    'received_msgs': received,
                    'expected_msgs': expected,
                    'loss_percent': loss_percent
                })
        loss_df = pd.DataFrame(loss_data)
        if not loss_df.empty:
            st.dataframe(loss_df, use_container_width=True)
            
    else:
        st.info("No data received yet.")

time.sleep(2)
st.rerun()
