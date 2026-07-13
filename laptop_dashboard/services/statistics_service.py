import pandas as pd
import sqlite3

class StatisticsService:
    def __init__(self, db_path="dds_demo.db"):
        self.db_path = db_path

    def get_qos_metrics(self, df_data, current_time):
        if df_data.empty:
            return 0, 0.0, 0.0, 0
            
        total_msgs = len(df_data)
        
        recent_msgs = df_data[df_data['receive_time'] > (current_time - 5.0)]
        throughput = len(recent_msgs) / 5.0 if len(recent_msgs) > 0 else 0.0
        
        df_sorted = df_data.sort_values(by=["device_id", "sequence_number"])
        df_sorted['seq_diff'] = df_sorted.groupby('device_id')['sequence_number'].diff()
        
        lost_packets_series = df_sorted[df_sorted['seq_diff'] > 1]['seq_diff']
        lost_packets = lost_packets_series.sum() - len(lost_packets_series) if not lost_packets_series.empty else 0
        
        loss_rate = (lost_packets / (total_msgs + lost_packets)) * 100 if total_msgs > 0 else 0
        
        return total_msgs, throughput, loss_rate, int(lost_packets)

    def fetch_recent_data(self):
        conn = sqlite3.connect(self.db_path, timeout=5)
        try:
            df_data = pd.read_sql_query("SELECT * FROM sensor_data ORDER BY receive_time DESC LIMIT 2000", conn)
        except:
            df_data = pd.DataFrame()
        conn.close()
        return df_data

    def fetch_participants(self):
        conn = sqlite3.connect(self.db_path, timeout=5)
        try:
            df_participants = pd.read_sql_query("SELECT * FROM participants", conn)
        except:
            df_participants = pd.DataFrame()
        conn.close()
        return df_participants

    def fetch_discovery_events(self):
        conn = sqlite3.connect(self.db_path, timeout=5)
        try:
            df_events = pd.read_sql_query("SELECT * FROM discovery_events ORDER BY timestamp DESC LIMIT 100", conn)
        except:
            df_events = pd.DataFrame()
        conn.close()
        return df_events
