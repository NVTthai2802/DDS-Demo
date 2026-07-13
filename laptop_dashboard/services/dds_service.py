import threading
import time
import json
import subprocess
import os
import sys

# Ensure utils can be imported when running app.py
sys.path.append(os.path.join(os.path.dirname(__file__), ".."))
from utils.db_helper import init_db, insert_sensor_data, update_participant, insert_discovery_event

class DDSSubscriberThread(threading.Thread):
    def __init__(self):
        super().__init__(daemon=True)
        init_db()
        self.running = True
        self.process = None

    def run(self):
        print("DDS Subscriber đang lắng nghe thông qua C++ Fast DDS Backend...")
        
        backend_path = os.path.join(os.path.dirname(__file__), "..", "..", "wsl_backend", "backend", "build", "subscriber")
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
                        insert_sensor_data(sample, current_time)
                    elif msg.get("type") == "discovery":
                        entity = msg.get("entity")
                        event = msg.get("event")
                        guid = msg.get("guid")
                        name = msg.get("name")
                        
                        if entity == "participant":
                            update_participant(guid, name, event)
                            insert_discovery_event(entity, event, guid, name)
                        elif entity == "publisher":
                            insert_discovery_event(entity, event, guid, name)
                except json.JSONDecodeError:
                    pass
            else:
                print(line)
            
            time.sleep(0.001)

    def stop(self):
        self.running = False
        if self.process:
            self.process.terminate()
