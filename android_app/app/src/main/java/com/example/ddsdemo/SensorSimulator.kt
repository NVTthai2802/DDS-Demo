package com.example.ddsdemo

import kotlinx.coroutines.*
import kotlin.math.sin
import kotlin.math.cos

class SensorSimulator(
    private val onDataGenerated: (temp: Double, hum: Double, co2: Int, light: Int, occ: Boolean, batt: Int, sig: Int) -> Unit
) {
    
    private var job: Job? = null
    private var timeStep = 0.0

    // State variables for smooth random walk
    private var currentTemp = 26.0
    private var currentHum = 55.0
    private var currentCo2 = 800.0

    fun start() {
        if (job?.isActive == true) return

        job = CoroutineScope(Dispatchers.Default).launch {
            while (isActive) {
                timeStep += 0.1

                // Temperature: Random Walk with constraints [20, 35]
                val tempNoise = (Math.random() - 0.5) * 0.4
                currentTemp += tempNoise
                currentTemp = currentTemp.coerceIn(20.0, 35.0)

                // Humidity: Random Walk + Sine wave oscillation
                val humNoise = (Math.random() - 0.5) * 0.5
                currentHum += humNoise
                currentHum = currentHum.coerceIn(30.0, 80.0)
                val humFinal = currentHum + sin(timeStep) * 2.0
                
                // CO2: Random Walk
                val co2Noise = (Math.random() - 0.5) * 15.0
                currentCo2 += co2Noise
                currentCo2 = currentCo2.coerceIn(400.0, 2000.0)
                val co2Final = currentCo2.toInt()

                // Light: Cosine wave + small noise
                val lightFinal = 400 + (cos(timeStep * 0.5) * 50).toInt() + (Math.random() * 10).toInt()
                
                // Occupancy: Occasional flip
                val occ = Math.random() > 0.8
                
                // Battery: Slowly drops over time
                val batt = 100 - (timeStep / 5).toInt().coerceAtMost(100)
                
                // Signal: small fluctuations
                val sig = -50 - (Math.random() * 5).toInt()

                // Dispatch to callback
                withContext(Dispatchers.Main) {
                    onDataGenerated(currentTemp, humFinal, co2Final, lightFinal, occ, batt, sig)
                }
                
                delay(1000) // 1Hz data rate
            }
        }
    }

    fun stop() {
        job?.cancel()
        job = null
    }
}
