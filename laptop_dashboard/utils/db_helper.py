import sqlite3
import time

DB_PATH = "dds_demo.db"

def init_db():
    conn = sqlite3.connect(DB_PATH)
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
    c.execute('''CREATE TABLE IF NOT EXISTS discovery_events (
        id INTEGER PRIMARY KEY AUTOINCREMENT,
        timestamp REAL,
        entity TEXT,
        event TEXT,
        guid TEXT,
        name TEXT
    )''')
    # Clear old data for demo
    c.execute('DELETE FROM sensor_data')
    c.execute('DELETE FROM participants')
    c.execute('DELETE FROM discovery_events')
    conn.commit()
    conn.close()

def insert_sensor_data(sample, current_time):
    # Trích xuất timestamp và tự động tính toán latency trong db_helper (hoặc service)
    # Tuy nhiên, tách trách nhiệm tính latency ra statistics_service là lý tưởng, nhưng để gọn,
    # ta cho statistics_service tính latency rồi truyền vào đây cũng được.
    latency = (current_time - sample["timestamp"]) * 1000
    conn = sqlite3.connect(DB_PATH, timeout=10)
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

def update_participant(guid, name, status):
    conn = sqlite3.connect(DB_PATH, timeout=10)
    c = conn.cursor()
    c.execute('''INSERT INTO participants (guid, name, status, last_seen)
                 VALUES (?, ?, ?, ?)
                 ON CONFLICT(guid) DO UPDATE SET status=excluded.status, last_seen=excluded.last_seen, name=excluded.name
              ''', (guid, name, status, time.time()))
    conn.commit()
    conn.close()

def insert_discovery_event(entity, event, guid, name):
    conn = sqlite3.connect(DB_PATH, timeout=10)
    c = conn.cursor()
    c.execute('''INSERT INTO discovery_events (timestamp, entity, event, guid, name)
                 VALUES (?, ?, ?, ?, ?)''', (time.time(), entity, event, guid, name))
    conn.commit()
    conn.close()
