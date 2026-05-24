# 🌿 Hybrid Smart Farm System (ESP32)

An integrated Internet of Things (IoT) system designed to automate and manage smart farms and greenhouses using the **ESP32** microcontroller. The project integrates three parallel and independent communication protocols to provide maximum flexibility, local control, and cloud monitoring.

### ⚙️ Developed By
* **Engineer:** Saad Ahmed Mouith Al-bohan
* **المهندس:** سعد أحمد معيض علبوهان
---

## ✨ Key Features

* **Triple Connectivity Hybrid System:**
  * **Wi-Fi Web Server:** Built-in Async Web Server hosting a highly responsive and fast user interface.
  * **Bluetooth Serial:** Local wireless control and real-time sensor data streaming via a mobile app.
  * **MQTT Client:** Real-time cloud data broadcasting to a broker for remote tracking and telemetry data streaming.
* **Professional Dual-Language Dashboard:** Embedded web interface (HTML5, CSS3, JS) supporting both **Arabic and English** with live readings, an advanced simulation system, and smart weather indicators.
* **Dual Control Modes:**
  * **Auto Mode:** Instant decision-making based on pre-programmed automation algorithms to protect crops.
  * **Manual Mode:** Full control over actuators via the web interface or Bluetooth.
* **Anti-Crash Guard:** Smart initialization separating radio resource periods (Wi-Fi & Bluetooth) to prevent system crashes or data loss.

---

## 📊 Supported Sensors & Actuators

| Component | System Function | Pin |
| :--- | :--- | :--- |
| **Temperature Sensor** | Frost detection and heater/alarm activation | Software / Simulated |
| **Soil Moisture Sensor** | Determines soil irrigation needs | Software / Simulated |
| **Rain Sensor** | Detects storms and rainfall intensity | Software / Simulated |
| **Water Pump** | Automates soil irrigation during droughts | `GPIO 2` |
| **Heater** | Activates heating units when frost is detected | `GPIO 4` |
| **Alarm LED** | Visual alert during environmental hazards | `GPIO 12` |
| **Servo Motor** | Opens/Closes the smart roof to protect crops from heavy rain | `GPIO 13` |

---

## 🛠️ Dependencies

Ensure the following libraries are installed in your Arduino IDE before flashing the code:
1. `WiFi.h` & `WiFiClient.h` (Built-in)
2. `ESPAsyncWebServer.h` (For high-performance web serving)
3. `PubSubClient.h` (For MQTT cloud connectivity)
4. `BluetoothSerial.h` (For Bluetooth mobile app control)
5. `ESP32Servo.h` (For precise servo motor control)

---

## 🚀 Automation Logic Workflow

1. **Frost Protection:** If the temperature drops below **3°C**, the heater and danger alarm activate immediately. They turn off automatically when the temperature rises above **5°C**.
2. **Smart Irrigation:** When soil moisture drops below **50%**, the water pump starts running instantly until the soil is well-hydrated.
3. **Smart Roof:** Upon detecting heavy rain or storms (exceeding **65%** intensity), the servo motor rotates to **180°** to close the roof and protect the crops.
