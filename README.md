# Smart IoT Dashboard with ESP32 Hardware Control

A complete IoT solution featuring a modern web-based dashboard interface and ESP32 firmware for real-time sensor monitoring and device control.

## 📦 Components

### 1. **Web Dashboard** (`index.html`)
- Modern neon-themed UI with real-time sensor visualization
- Live temperature, humidity, and light level monitoring
- Device control via interactive toggles (relays)
- Historical data visualization with Chart.js
- Timer system for scheduled automation
- Smart Auto mode with threshold-based control
- Firebase Realtime Database integration

### 2. **Arduino ESP32 Firmware** (`arduino_firmware.ino`)
- DHT22 temperature & humidity sensor support
- LDR (Light Dependent Resistor) sensor for brightness detection
- Dual relay control for appliances
- WiFi connectivity with WiFiManager
- Firebase Realtime Database synchronization
- NTP time synchronization for accurate timestamps
- Smart automation: Auto fan control based on temperature
- Real-time stream listener for remote commands

## 🎯 Features

### Dashboard Features
- 🎨 Neon cyan/purple glowing UI theme
- 📊 Real-time sensor data display
- 📈 Historical data charts (60-point history)
- 🎛️ Manual device control via switches
- ⏰ Timer management (daily schedules)
- 🤖 Smart Auto mode with custom thresholds
- 🔐 Firebase Authentication (email/password)
- 📱 Fully responsive design

### Hardware Features
- 🌡️ Temperature monitoring (DHT22)
- 💧 Humidity sensing
- 💡 Light level detection
- 🔌 Dual relay control outputs
- 📡 WiFi connectivity
- ☁️ Cloud synchronization
- ⚑ Automatic fan control based on temp thresholds

## 🔧 Hardware Requirements

### Components
- ESP32 Development Board
- DHT22 Temperature/Humidity Sensor
- LDR (Light Sensor) + 10kΩ resistor
- 2x Relay Module
- Micro USB Power Supply

### Pinout Configuration
```
GPIO 18 -> Relay 1 (Light)
GPIO 19 -> Relay 2 (Fan)
GPIO 27 -> DHT22 Data
GPIO 34 -> LDR Analog Input
GPIO 2  -> Status LED
GPIO 4  -> WiFi Reset Button (optional)
```

## 📝 Firebase Structure

```
iot_system/
├── sensors/
│   ├── temperature (float)
│   ├── humidity (float)
│   ├── light_percent (int)
│   └── timestamp (epoch ms)
├── controls/
│   ├── relay_1 (boolean)
│   └── relay_2 (boolean)
├── settings/
│   ├── auto_mode (boolean)
│   ├── auto_fan_temp (float)
│   ├── alert_temp (float)
│   └── alert_light (int)
├── timers/ (array)
└── history/ (array with historical data)
```

## ⚙️ Setup Instructions

### Dashboard Setup
1. Open `index.html` in a modern web browser
2. Click "ตั้งค่า Firebase" (Firebase Settings)
3. Enter your Firebase configuration:
   - API Key
   - Auth Domain
   - Database URL
   - Project ID
   - App ID
4. Create an account and sign in
5. Configure automation settings

### Arduino Setup
1. Install required libraries in Arduino IDE:
   - WiFiManager
   - Firebase ESP Client
   - DHT sensor library
   
2. Update firmware credentials:
   - Replace `API_KEY` with your Firebase API key
   - Update `DATABASE_URL` with your Firebase Realtime Database URL

3. Upload firmware to ESP32 board

4. Device will auto-connect to WiFi via WiFiManager on first boot

## 📊 Data Flow

```
ESP32 Hardware
   ↓
(DHT22, LDR sensors)
   ↓
Firebase Realtime DB
   ↓
Web Dashboard (View + Control)
   ↓
(User inputs)
   ↓
Firebase Realtime DB
   ↓
ESP32 Hardware (Relay control)
```

## 🎨 UI Theme

The dashboard features an advanced neon theme with:
- Cyan (#00d4ff) and purple (#bb66ff) color scheme
- Glowing effects on interactive elements
- Smooth cubic-bezier animations
- Responsive backdrop blur effects
- Real-time status indicators with pulse animations

## 📱 Responsive Design

- Desktop (1400px+)
- Tablet (860px - 1100px)
- Mobile (< 560px)

## 🔐 Security Notes

- Store Firebase credentials securely
- Use strong passwords for authentication
- Consider implementing rate limiting
- Monitor API usage in Firebase Console

## 📚 Dependencies

### Frontend
- Chart.js (data visualization)
- Firebase Web SDK
- Google Fonts (Sarabun Thai font)

### Hardware
- WiFiManager
- Firebase ESP Client
- DHT sensor library
- Arduino core for ESP32

## 🐛 Troubleshooting

**Dashboard doesn't connect to Firebase:**
- Verify Firebase credentials are correct
- Check internet connection
- Ensure Firebase Realtime Database rules allow read/write

**Sensor readings show incorrect values:**
- Verify DHT22/LDR wiring
- Check sensor power supply
- Reinstall sensor libraries
- Try replacing sensors

**ESP32 won't upload:**
- Install CH340 drivers
- Select correct board (ESP32 Dev Module)
- Choose correct COM port and baud rate (115200)

## 📄 License

This project is provided as-is for educational and development purposes.

## 👤 Author

**bfirstkok**

---

**Last Updated:** March 28, 2026
**Repository:** https://github.com/bfirstkok/DashBoradIOTESP32
