import threading
import time
import json
import subprocess
import os
import sqlite3

class DDSSubscriberThread(threading.Thread):
    def __init__(self, db_path="dds_demo.db"):
        super().__init__(daemon=True)
        self.db_path = db_path
        self._init_db()
        self.running = True
        self.process = None

    def _init_db(self):
        conn = sqlite3.connect(self.db_path)
        c = conn.cursor()
        c.execute('''CREATE TABLE IF NOT EXISTS sensor_data (
            id INTEGER PRIMARY KEY AUTOINCREMENT,
            device_id TEXT, device_type TEXT, temperature REAL,
            humidity REAL, co2 INTEGER, light INTEGER,
            occupancy BOOLEAN, battery INTEGER, signal_strength INTEGER,
            sequence_number INTEGER, timestamp REAL, receive_time REAL,
            latency_ms REAL
        )''')
        c.execute('''CREATE TABLE IF NOT EXISTS participants (
            guid TEXT PRIMARY KEY,
            name TEXT,
            status TEXT,
            last_seen REAL
        )''')
        # Clear old data for demo
        c.execute('DELETE FROM sensor_data')
        c.execute('DELETE FROM participants')
        conn.commit()
        conn.close()

    def _insert_data(self, sample, latency, current_time):
        conn = sqlite3.connect(self.db_path, timeout=10)
        c = conn.cursor()
        c.execute('''INSERT INTO sensor_data 
            (device_id, device_type, temperature, humidity, co2, light, 
             occupancy, battery, signal_strength, sequence_number, timestamp, receive_time, latency_ms)
            VALUES (?, ?, ?, ?, ?, ?, ?, ?, ?, ?, ?, ?, ?)''', 
            (sample["device_id"], sample["device_type"], sample["temperature"], sample["humidity"],
             sample["co2"], sample["light"], sample["occupancy"], sample["battery"],
             sample["signal_strength"], sample["sequence_number"], sample["timestamp"],
             current_time, latency))
        conn.commit()
        conn.close()

    def _update_participant(self, guid, name, status):
        conn = sqlite3.connect(self.db_path, timeout=10)
        c = conn.cursor()
        c.execute('''INSERT INTO participants (guid, name, status, last_seen)
                     VALUES (?, ?, ?, ?)
                     ON CONFLICT(guid) DO UPDATE SET status=excluded.status, last_seen=excluded.last_seen, name=excluded.name
                  ''', (guid, name, status, time.time()))
        conn.commit()
        conn.close()

    def run(self):
        print("DDS Subscriber đang lắng nghe thông qua C++ Fast DDS Backend...")
        
        backend_path = os.path.join(os.path.dirname(__file__), "..", "wsl_backend", "backend", "build", "subscriber")
        if not os.path.exists(backend_path):
            print(f"[ERROR] Không tìm thấy {backend_path}. Vui lòng build C++ backend trước!")
            return
            
        self.process = subprocess.Popen(
            [backend_path],
            stdout=subprocess.PIPE,
            stderr=subprocess.STDOUT,
            text=True
        )

        while self.running and self.process.poll() is None:
            line = self.process.stdout.readline()
            if not line:
                continue
                
            line = line.strip()
            if line.startswith("{") and line.endswith("}"):
                try:
                    msg = json.loads(line)
                    if msg.get("type") == "data":
                        sample = msg["payload"]
                        current_time = time.time()
                        latency = (current_time - sample["timestamp"]) * 1000
                        self._insert_data(sample, latency, current_time)
                    elif msg.get("type") == "discovery":
                        self._update_participant(msg["guid"], msg["name"], msg["event"])
                except json.JSONDecodeError:
                    pass
            else:
                print(line)
            
            time.sleep(0.001)

    def stop(self):
        self.running = False
        if self.process:
            self.process.terminate()
