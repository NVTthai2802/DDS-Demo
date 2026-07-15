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
                guid TEXT PRIMARY KEY,
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

    def update_spdp(self, guid, name, status):
        c = self.conn.cursor()
        now = datetime.now()
        if status == "DISCOVERED":
            c.execute('''
                INSERT INTO peers (guid, name, status, sedp_matched, last_seen)
                VALUES (?, ?, ?, ?, ?)
                ON CONFLICT(guid) DO UPDATE SET status=?, last_seen=?
            ''', (guid, name, status, False, now, status, now))
        elif status == "REMOVED":
            c.execute('UPDATE peers SET status=?, last_seen=?, sedp_matched=0 WHERE guid=?', (status, now, guid))
        self.conn.commit()

    def update_sedp(self, writer_guid, status, qos):
        c = self.conn.cursor()
        now = datetime.now()
        is_matched = (status == "MATCHED")
        c.execute('UPDATE peers SET sedp_matched=?, qos=?, last_seen=? WHERE guid=?', 
                  (is_matched, qos, now, writer_guid))
        self.conn.commit()

    def insert_data(self, data):
        c = self.conn.cursor()
        # Tính latency tương đối (receive_time - timestamp do C++ backend ghi nhận)
        latency = data['receive_time'] - data['timestamp']
        c.execute('''
            INSERT INTO sensor_data (device_id, sequence_number, timestamp, temperature, humidity, latency_ms, receive_time)
            VALUES (?, ?, ?, ?, ?, ?, ?)
        ''', (data['device_id'], data['sequence_number'], data['timestamp'], 
              data['temperature'], data['humidity'], latency, data['receive_time']))
        
        # Cập nhật SEDP Matched và Last Seen cho GUID này
        if 'writer_guid' in data:
            now = datetime.now()
            c.execute("UPDATE peers SET last_seen=?, status='DISCOVERED', sedp_matched=1 WHERE guid=?", (now, data['writer_guid']))
        self.conn.commit()

    def get_peers_df(self):
        return pd.read_sql_query("SELECT * FROM peers", self.conn)

    def get_latest_data_df(self, limit=100):
        return pd.read_sql_query("SELECT * FROM sensor_data ORDER BY id DESC LIMIT ?", self.conn, params=(limit,))
