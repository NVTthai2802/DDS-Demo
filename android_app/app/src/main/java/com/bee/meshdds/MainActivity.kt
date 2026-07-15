package com.bee.meshdds

import android.content.Context
import android.net.wifi.WifiManager
import android.os.Bundle
import android.widget.Button
import android.widget.TextView
import androidx.appcompat.app.AppCompatActivity

class MainActivity : AppCompatActivity() {

    private lateinit var multicastLock: WifiManager.MulticastLock
    private var isDdsRunning = false

    override fun onCreate(savedInstanceState: Bundle?) {
        super.onCreate(savedInstanceState)
        setContentView(R.layout.activity_main)

        // Bắt buộc trên Android: Cấp quyền Multicast Lock để giao thức SPDP chạy được qua Wi-Fi
        val wifi = applicationContext.getSystemService(Context.WIFI_SERVICE) as WifiManager
        multicastLock = wifi.createMulticastLock("MeshDdsLock")
        multicastLock.setReferenceCounted(true)
        multicastLock.acquire()

        val tvStatus = findViewById<TextView>(R.id.tvStatus)
        val btnStart = findViewById<Button>(R.id.btnStart)

        btnStart.setOnClickListener {
            if (!isDdsRunning) {
                // Tự sinh device_id động không hardcode
                val deviceId = android.provider.Settings.Secure.getString(
                    contentResolver,
                    android.provider.Settings.Secure.ANDROID_ID
                ) + "-Phone"
                
                // Gọi JNI C++
                startDds(deviceId)
                
                isDdsRunning = true
                tvStatus.text = "DDS Running as $deviceId"
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

    private external fun startDds(deviceId: String)
    private external fun stopDds()

    companion object {
        init {
            System.loadLibrary("meshdds")
        }
    }
}
