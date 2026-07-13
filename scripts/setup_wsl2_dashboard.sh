#!/bin/bash
# Setup Python environment for Dashboard on WSL2

echo "[INFO] Creating Python virtual environment..."
cd "/mnt/c/Project TTS/dds_demo/windows_dashboard"
python3 -m venv venv
source venv/bin/activate

echo "[INFO] Installing requirements..."
pip install streamlit pandas plotly

echo "[INFO] Dashboard environment setup complete!"
echo "[INFO] To run the dashboard, execute:"
echo "       source venv/bin/activate"
echo "       streamlit run app.py"
