package com.bee.meshdds

import android.content.Context
import android.content.SharedPreferences
import android.net.wifi.WifiManager
import android.os.Build
import android.os.Bundle
import android.widget.Button
import android.widget.EditText
import android.widget.TextView
import androidx.appcompat.app.AppCompatActivity
import kotlin.random.Random

class MainActivity : AppCompatActivity() {

    private lateinit var multicastLock: WifiManager.MulticastLock
    private var isDdsRunning = false
    private lateinit var prefs: SharedPreferences
    private lateinit var tvLog: TextView

    override fun onCreate(savedInstanceState: Bundle?) {
        super.onCreate(savedInstanceState)
        setContentView(R.layout.activity_main)

        // Bắt buộc trên Android: Cấp quyền Multicast Lock để giao thức SPDP chạy được qua Wi-Fi
        val wifi = applicationContext.getSystemService(Context.WIFI_SERVICE) as WifiManager
        multicastLock = wifi.createMulticastLock("MeshDdsLock")
        multicastLock.setReferenceCounted(true)
        multicastLock.acquire()

        prefs = getSharedPreferences("DDS_PREFS", Context.MODE_PRIVATE)

        val etDeviceId = findViewById<EditText>(R.id.etDeviceId)
        val tvStatus = findViewById<TextView>(R.id.tvStatus)
        val btnStart = findViewById<Button>(R.id.btnStart)
        tvLog = findViewById(R.id.tvLog)

        // Generate default device ID if not saved
        val defaultId = Build.MODEL + "-" + Random.nextInt(1000, 9999).toString()
        val savedId = prefs.getString("DEVICE_ID", defaultId)
        etDeviceId.setText(savedId)

        btnStart.setOnClickListener {
            if (!isDdsRunning) {
                val deviceId = etDeviceId.text.toString().trim().ifEmpty { defaultId }
                prefs.edit().putString("DEVICE_ID", deviceId).apply()
                
                // Gọi JNI C++
                startDds(deviceId)
                
                isDdsRunning = true
                tvStatus.text = "DDS Running as $deviceId"
                etDeviceId.isEnabled = false
                btnStart.isEnabled = false
            }
        }
    }

    override fun onDestroy() {
        super.onDestroy()
        if (multicastLock.isHeld) {
            multicastLock.release()
        }
        if (isDdsRunning) {
            stopDds()
        }
    }

    // Called from C++ background thread
    fun onSensorDataReceived(deviceId: String, seqNum: Int, temp: Float, hum: Float, timestamp: Long, latency: Long) {
        val msg = "[$deviceId] seq:$seqNum T:$temp H:$hum lat:${latency}ms\n"
        runOnUiThread {
            tvLog.append(msg)
        }
    }

    private external fun startDds(deviceId: String)
    private external fun stopDds()

    companion object {
        init {
            System.loadLibrary("meshdds")
        }
    }
}
