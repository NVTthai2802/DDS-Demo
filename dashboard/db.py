import sqlite3
import pandas as pd
from datetime import datetime
import json

class Database:
    def __init__(self, db_path="history.db", fresh_start=False):
        self.conn = sqlite3.connect(db_path, check_same_thread=False)
        self.guid_to_name = {}  # Map prefix to name
        if fresh_start:
            self.reset_session()
        self.create_tables()

    def reset_session(self):
        c = self.conn.cursor()
        c.execute("DROP TABLE IF EXISTS peers")
        c.execute("DROP TABLE IF EXISTS sensor_data")
        self.conn.commit()

    def create_tables(self):
        c = self.conn.cursor()
        c.execute('''
            CREATE TABLE IF NOT EXISTS peers (
                name TEXT PRIMARY KEY,
                prefix TEXT,
                guid TEXT,
                status TEXT,
                sedp_matched BOOLEAN,
                qos TEXT,
                last_seen DATETIME
            )
        ''')
        c.execute('''
            CREATE TABLE IF NOT EXISTS sensor_data (
                id INTEGER PRIMARY KEY AUTOINCREMENT,
                device_id TEXT,
                sequence_number INTEGER,
                timestamp INTEGER,
                temperature REAL,
                humidity REAL,
                co2 REAL,
                light REAL,
                occupancy BOOLEAN,
                battery REAL,
                signal_strength REAL,
                latency_ms INTEGER,
                receive_time INTEGER
            )
        ''')
        self.conn.commit()

    def get_prefix(self, guid_str):
        return guid_str.split('|')[0] if '|' in guid_str else guid_str

    def update_spdp(self, guid, name, status):
        c = self.conn.cursor()
        now = datetime.now()
        prefix = self.get_prefix(guid)
        if name:
            self.guid_to_name[prefix] = name
            
        if status == "DISCOVERED":
            c.execute('''
                INSERT INTO peers (name, prefix, guid, status, sedp_matched, last_seen)
                VALUES (?, ?, ?, ?, ?, ?)
                ON CONFLICT(name) DO UPDATE SET status=?, last_seen=?, prefix=?, guid=?
            ''', (name, prefix, guid, status, False, now, status, now, prefix, guid))
        elif status == "REMOVED":
            c.execute('UPDATE peers SET status=?, last_seen=?, sedp_matched=0 WHERE name=?', ("OFFLINE", now, name))
        self.conn.commit()

    def update_sedp(self, writer_guid, status, qos):
        c = self.conn.cursor()
        now = datetime.now()
        prefix = self.get_prefix(writer_guid)
        name = self.guid_to_name.get(prefix)
        if not name:
            return
            
        is_matched = (status == "MATCHED")
        c.execute('UPDATE peers SET sedp_matched=?, qos=?, last_seen=? WHERE name=?', 
                  (is_matched, qos, now, name))
        self.conn.commit()
        
    def update_liveliness(self, writer_guid, alive_change, not_alive_change):
        c = self.conn.cursor()
        now = datetime.now()
        prefix = self.get_prefix(writer_guid)
        name = self.guid_to_name.get(prefix)
        if not name:
            return
            
        if not_alive_change > 0:
            c.execute('UPDATE peers SET status=?, last_seen=?, sedp_matched=0 WHERE name=?', ("OFFLINE", now, name))
        elif alive_change > 0:
            c.execute('UPDATE peers SET status=?, last_seen=?, sedp_matched=1 WHERE name=?', ("DISCOVERED", now, name))
        self.conn.commit()

    def insert_data(self, data):
        c = self.conn.cursor()
        latency = data['receive_time'] - data['timestamp']
        c.execute('''
            INSERT INTO sensor_data (device_id, sequence_number, timestamp, temperature, humidity, co2, light, occupancy, battery, signal_strength, latency_ms, receive_time)
            VALUES (?, ?, ?, ?, ?, ?, ?, ?, ?, ?, ?, ?)
        ''', (data['device_id'], data['sequence_number'], data['timestamp'], 
              data['temperature'], data['humidity'], data.get('co2', 0), data.get('light', 0), data.get('occupancy', False), data.get('battery', 0), data.get('signal_strength', 0), latency, data['receive_time']))
        
        if 'writer_guid' in data:
            now = datetime.now()
            prefix = self.get_prefix(data['writer_guid'])
            name = self.guid_to_name.get(prefix)
            if name:
                c.execute("UPDATE peers SET last_seen=?, status='DISCOVERED', sedp_matched=1 WHERE name=?", (now, name))
        self.conn.commit()

    def get_peers_df(self):
        return pd.read_sql_query("SELECT * FROM peers", self.conn)

    def get_latest_data_df(self, limit=100):
        return pd.read_sql_query("SELECT * FROM sensor_data ORDER BY id DESC LIMIT ?", self.conn, params=(limit,))

    def get_packet_loss_df(self):
        query = """
        SELECT 
            device_id,
            COUNT(*) as received_msgs,
            MAX(sequence_number) - MIN(sequence_number) + 1 as expected_msgs,
            CASE 
                WHEN (MAX(sequence_number) - MIN(sequence_number) + 1) > 0 THEN 
                    ROUND(100.0 - (COUNT(*) * 100.0 / (MAX(sequence_number) - MIN(sequence_number) + 1)), 2)
                ELSE 0.0 
            END as loss_percent
        FROM sensor_data
        GROUP BY device_id
        """
        return pd.read_sql_query(query, self.conn)
