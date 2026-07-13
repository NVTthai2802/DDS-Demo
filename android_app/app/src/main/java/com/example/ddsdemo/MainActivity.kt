package com.example.ddsdemo

import androidx.appcompat.app.AppCompatActivity
import android.os.Bundle
import android.widget.Button
import android.widget.TextView
import android.widget.Toast
import android.content.Context
import android.net.wifi.WifiManager
import java.util.Timer
import kotlin.concurrent.timerTask

class MainActivity : AppCompatActivity() {

    private lateinit var btnConnect: Button
    private lateinit var btnStartPublish: Button
    private lateinit var tvStatus: TextView
    private lateinit var multicastLock: WifiManager.MulticastLock

    private var isConnected = false
    private var isPublishing = false
    private var publishTimer: Timer? = null

    override fun onCreate(savedInstanceState: Bundle?) {
        super.onCreate(savedInstanceState)
        setContentView(R.layout.activity_main)

        // Acquire MulticastLock to allow UDP Multicast packets
        val wifiManager = applicationContext.getSystemService(Context.WIFI_SERVICE) as WifiManager
        multicastLock = wifiManager.createMulticastLock("DDS_Multicast_Lock")
        multicastLock.setReferenceCounted(true)
        multicastLock.acquire()

        btnConnect = findViewById(R.id.btnConnect)
        btnStartPublish = findViewById(R.id.btnStartPublish)
        tvStatus = findViewById(R.id.tvStatus)

        btnConnect.setOnClickListener {
            val success = startDDS("") // IP no longer needed for Multicast
            if (success) {
                isConnected = true
                tvStatus.text = "Trang thai: Da khoi tao DDS (Auto-Discovery)"
                btnConnect.isEnabled = false
                btnStartPublish.isEnabled = true
                Toast.makeText(this, "Khoi tao Multicast thanh cong!", Toast.LENGTH_SHORT).show()
            } else {
                Toast.makeText(this, "Loi khoi tao DDS!", Toast.LENGTH_SHORT).show()
            }
        }

        btnStartPublish.setOnClickListener {
            if (!isPublishing) {
                isPublishing = true
                btnStartPublish.text = "Stop Publishing"
                tvStatus.text = "Trang thai: Dang gui du lieu..."
                
                // Send data every 1000ms (1Hz for sensors)
                publishTimer = Timer()
                publishTimer?.scheduleAtFixedRate(timerTask {
                    // Simulate random environmental data
                    val temp = 26.0 + Math.random() * 2.0
                    val hum = 55.0 + Math.random() * 5.0
                    val co2 = 800 + (Math.random() * 50).toInt()
                    val light = 450 + (Math.random() * 100).toInt()
                    val occ = Math.random() > 0.5
                    val batt = 100 - (Math.random() * 20).toInt()
                    val sig = -50 - (Math.random() * 10).toInt()
                    
                    publishData("MeetingRoom-01", "Environmental Sensor", temp, hum, co2, light, occ, batt, sig)
                }, 0, 1000)
                
            } else {
                isPublishing = false
                btnStartPublish.text = "Start Publishing"
                tvStatus.text = "Trang thai: Da dung gui du lieu."
                publishTimer?.cancel()
            }
        }
    }

    override fun onDestroy() {
        super.onDestroy()
        publishTimer?.cancel()
        if (::multicastLock.isInitialized && multicastLock.isHeld) {
            multicastLock.release()
        }
        stopDDS()
    }

    /**
     * Native C++ functions (JNI)
     */
    external fun startDDS(laptopIp: String): Boolean
    external fun stopDDS()
    external fun publishData(
        deviceId: String, deviceType: String, temp: Double, hum: Double, 
        co2: Int, light: Int, occ: Boolean, batt: Int, sig: Int
    )

    companion object {
        // Load C++ library on App startup
        init {
            System.loadLibrary("ddsdemo")
        }
    }
}