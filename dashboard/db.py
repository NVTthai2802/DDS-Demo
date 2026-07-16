import sqlite3
import pandas as pd
from datetime import datetime
import json

class Database:
    def __init__(self, db_path="history.db"):
        self.conn = sqlite3.connect(db_path, check_same_thread=False)
        self.create_tables()

    def create_tables(self):
        c = self.conn.cursor()
        c.execute('''
            CREATE TABLE IF NOT EXISTS peers (
                prefix TEXT PRIMARY KEY,
                guid TEXT,
                name TEXT,
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
        if status == "DISCOVERED":
            c.execute('''
                INSERT INTO peers (prefix, guid, name, status, sedp_matched, last_seen)
                VALUES (?, ?, ?, ?, ?, ?)
                ON CONFLICT(prefix) DO UPDATE SET status=?, last_seen=?
            ''', (prefix, guid, name, status, False, now, status, now))
        elif status == "REMOVED":
            c.execute('UPDATE peers SET status=?, last_seen=?, sedp_matched=0 WHERE prefix=?', ("OFFLINE", now, prefix))
        self.conn.commit()

    def update_sedp(self, writer_guid, status, qos):
        c = self.conn.cursor()
        now = datetime.now()
        prefix = self.get_prefix(writer_guid)
        is_matched = (status == "MATCHED")
        c.execute('UPDATE peers SET sedp_matched=?, qos=?, last_seen=? WHERE prefix=?', 
                  (is_matched, qos, now, prefix))
        self.conn.commit()
        
    def update_liveliness(self, writer_guid, alive_change, not_alive_change):
        c = self.conn.cursor()
        now = datetime.now()
        prefix = self.get_prefix(writer_guid)
        if not_alive_change > 0:
            c.execute('UPDATE peers SET status=?, last_seen=?, sedp_matched=0 WHERE prefix=?', ("OFFLINE", now, prefix))
        elif alive_change > 0:
            c.execute('UPDATE peers SET status=?, last_seen=?, sedp_matched=1 WHERE prefix=?', ("DISCOVERED", now, prefix))
        self.conn.commit()

    def insert_data(self, data):
        c = self.conn.cursor()
        latency = data['receive_time'] - data['timestamp']
        c.execute('''
            INSERT INTO sensor_data (device_id, sequence_number, timestamp, temperature, humidity, latency_ms, receive_time)
            VALUES (?, ?, ?, ?, ?, ?, ?)
        ''', (data['device_id'], data['sequence_number'], data['timestamp'], 
              data['temperature'], data['humidity'], latency, data['receive_time']))
        
        if 'writer_guid' in data:
            now = datetime.now()
            prefix = self.get_prefix(data['writer_guid'])
            c.execute("UPDATE peers SET last_seen=?, status='DISCOVERED', sedp_matched=1 WHERE prefix=?", (now, prefix))
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
