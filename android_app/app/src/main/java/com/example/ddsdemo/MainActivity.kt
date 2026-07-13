package com.example.ddsdemo

import androidx.appcompat.app.AppCompatActivity
import android.os.Bundle
import android.widget.Button
import android.widget.TextView
import android.widget.Toast
import android.content.Context
import android.net.wifi.WifiManager
import android.widget.CheckBox

class MainActivity : AppCompatActivity() {

    private lateinit var btnConnect: Button
    private lateinit var btnStartPublish: Button
    private lateinit var tvStatus: TextView
    private lateinit var cbReliable: CheckBox
    private lateinit var multicastLock: WifiManager.MulticastLock

    private var isConnected = false
    private var isPublishing = false
    
    private lateinit var sensorSimulator: SensorSimulator

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
        cbReliable = findViewById(R.id.cbReliable)

        // Initialize Simulator
        sensorSimulator = SensorSimulator { temp, hum, co2, light, occ, batt, sig ->
            if (isPublishing) {
                DDSManager.publishDataNative(
                    "MeetingRoom-01", "Environmental Sensor", 
                    temp, hum, co2, light, occ, batt, sig
                )
            }
        }

        btnConnect.setOnClickListener {
            val success = DDSManager.startDDSNative(cbReliable.isChecked)
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
                sensorSimulator.start()
            } else {
                isPublishing = false
                btnStartPublish.text = "Start Publishing"
                tvStatus.text = "Trang thai: Da dung gui du lieu."
                sensorSimulator.stop()
            }
        }
    }

    override fun onDestroy() {
        super.onDestroy()
        sensorSimulator.stop()
        if (::multicastLock.isInitialized && multicastLock.isHeld) {
            multicastLock.release()
        }
        DDSManager.stopDDSNative()
    }
}