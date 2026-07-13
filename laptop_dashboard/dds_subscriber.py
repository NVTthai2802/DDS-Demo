import threading
import time
import json
import subprocess
import os

class DDSSubscriberThread(threading.Thread):
    def __init__(self, data_queue):
        super().__init__(daemon=True)
        self.data_queue = data_queue
        self.running = True
        self.process = None

    def run(self):
        print("DDS Subscriber đang lắng nghe thông qua C++ Fast DDS Backend...")
        
        # Chạy C++ backend
        backend_path = os.path.join(os.path.dirname(__file__), "..", "wsl_dashboard", "backend", "build", "subscriber")
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
                    sample = json.loads(line)
                    current_time = time.time()
                    latency = (current_time - sample["timestamp"]) * 1000 # milliseconds
                    
                    data_point = {
                        "device_id": sample["device_id"],
                        "device_type": sample["device_type"],
                        "temperature": sample["temperature"],
                        "humidity": sample["humidity"],
                        "co2": sample["co2"],
                        "light": sample["light"],
                        "occupancy": sample["occupancy"],
                        "battery": sample["battery"],
                        "signal_strength": sample["signal_strength"],
                        "sequence_number": sample["sequence_number"],
                        "timestamp": sample["timestamp"],
                        "receive_time": current_time,
                        "latency_ms": latency
                    }
                    self.data_queue.append(data_point)
                except json.JSONDecodeError:
                    print(f"Failed to parse JSON: {line}")
            else:
                # In các log khác (INFO, ERROR từ C++)
                print(line)
            
            time.sleep(0.001)

    def stop(self):
        self.running = False
        if self.process:
            self.process.terminate()
