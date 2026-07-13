package com.example.ddsdemo

object DDSManager {
    init {
        System.loadLibrary("ddsdemo")
    }

    external fun startDDSNative(isReliable: Boolean): Boolean
    external fun stopDDSNative()
    external fun publishDataNative(
        deviceId: String, deviceType: String, temp: Double, hum: Double, 
        co2: Int, light: Int, occ: Boolean, batt: Int, sig: Int
    )
}
