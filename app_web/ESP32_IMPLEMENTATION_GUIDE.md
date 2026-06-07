# ESP32 Wi-Fi Robot Control Implementation Guide

This document explains how to use the web interface we just built, how to host it so you can access it from your phone, and provides an exact example of the code you need to flash onto your ESP32.

---

## 1. Hosting the Web App on Your Computer

To use this application from your mobile phone, both your computer and your phone must be connected to the **same Wi-Fi network**.

1. Open **VS Code**.
2. Install the extension called **"Live Server"** (by Ritwick Dey) if you don't have it already.
3. Open the `index.html` file in VS Code.
4. Click **"Go Live"** in the bottom right corner of VS Code (or right-click the HTML file and select "Open with Live Server").
5. Your computer will start a local web server (usually at `http://127.0.0.1:5500/index.html`).

---

## 2. Accessing the Web App from Your Phone

1. Find your computer's local IP address on your Wi-Fi network. 
   - On Windows, open the Command Prompt and type `ipconfig`. Look for the "IPv4 Address" (e.g., `192.168.1.50`).
2. Open the web browser on your smartphone.
3. Type in your computer's IP address and the Live Server port. For example:
   `http://192.168.1.50:5500/index.html`
4. You should now see the premium robot control interface on your phone!

---

## 3. Programming the ESP32 (Using the Custom Library)

The web interface sends HTTP requests to the ESP32. To make the code clean and easy to manage, we have created a custom C++ library called `RobotWebControl` inside the `esp32_library` folder.

This library hides the complex Wi-Fi setup and CORS (Cross-Origin Resource Sharing) security requirements so you can just focus on programming your robot's motors.

### How the Library Works

The library consists of two main files (`RobotWebControl.h` and `RobotWebControl.cpp`) that wrap the standard `WebServer` and `WiFi` libraries. It provides simple "callbacks" (functions that run when an event happens). You just tell it what to do when a "START", "STOP", "PAUSE", or "RESUME" command arrives!

### Arduino IDE Example

Open the `esp32_library.ino` file inside the `esp32_library` folder in your Arduino IDE. It looks like this:

```cpp
#include "RobotWebControl.h"

// Initialize the library
RobotWebControl robotControl;

// Replace with your Wi-Fi credentials
const char* ssid = "YOUR_WIFI_SSID";
const char* password = "YOUR_WIFI_PASSWORD";

void setup() {
  Serial.begin(115200);

  // 1. Define what happens when the web app sends a "START" command
  robotControl.onStart([]() {
    Serial.println("Action: Starting Robot Routine...");
    // TODO: Add your motor start code here
  });

  // 2. Define what happens when the web app sends a "STOP" command
  robotControl.onStop([]() {
    Serial.println("Action: Emergency Stop!");
    // TODO: Add your motor stop code here
  });

  // 3. Define what happens when the web app sends a "PAUSE" command
  robotControl.onPause([]() {
    Serial.println("Action: Pausing Robot...");
    // TODO: Add your motor pause code here
  });

  // 4. Define what happens when the web app sends a "RESUME" command
  robotControl.onResume([]() {
    Serial.println("Action: Resuming Robot...");
    // TODO: Add your motor resume code here
  });

  // Start the Wi-Fi and Web Server
  robotControl.begin(ssid, password);
}

void loop() {
  // Listen for incoming web requests (Required to keep the server responsive)
  robotControl.handleClient();
  
  // Other robot logic (sensors, movement, etc.) can go here!
}
```

### Steps to implement this:
1. Open the `esp32_library.ino` file in the Arduino IDE. Ensure the `RobotWebControl.h` and `RobotWebControl.cpp` files are in the same folder.
2. Replace `YOUR_WIFI_SSID` and `YOUR_WIFI_PASSWORD` with your actual Wi-Fi network details (the same one your computer and phone are connected to).
3. Add your specific motor control code inside the callback blocks (e.g., inside the `robotControl.onStart([]() { ... });` block).
4. Upload the code to your ESP32.
5. Open the **Serial Monitor** (at 115200 baud rate).
6. Wait for it to connect and copy the **ESP32 IP Address** printed in the Serial Monitor.
7. Open the Web App on your phone, paste that IP address at the top, and click **"Set"**.

The connection indicator should turn green and start pulsing, and clicking the buttons will trigger your custom code and print the commands in your Serial Monitor!
