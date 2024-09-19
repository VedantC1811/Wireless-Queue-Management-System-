# Wireless-Queue-Management-System-

/*
 * Wireless Queue Management System
 *
 * Overview:
 * The Wireless Queue Management System uses an ESP32 as the server and ESP8266 as the clients to manage and display queue numbers. 
 * Clients input queue numbers via IR remote controls, and the server processes and displays the current queue number on a web page.
 *
 * Components:
 * 1. ESP32 Server:
 *    - Hosts the web page for real-time display of queue numbers.
 *    - Manages queue number data from clients and updates the web page.
 *    - Stores and retrieves queue numbers using EEPROM.
 *    - Handles AJAX requests from clients for real-time updates.
 *
 * 2. ESP8266 Clients:
 *    - Equipped with IR receivers to detect input from remote controls.
 *    - Sends queue numbers to the ESP32 server via HTTP requests.
 *    - Uses EEPROM to store queue numbers and display status on an LCD.
 *    - Displays real-time information on an I2C LCD.
 *
 * 3. IR Remote Control:
 *    - Allows users to input queue numbers and control token navigation (next, previous, send, clear).
 *    - Sends specific IR codes that are interpreted by the ESP8266 clients.
 *
 * 4. Web Interface:
 *    - Hosted on the ESP32 server, showing the current queue token number.
 *    - Accessible from any device connected to the server’s WiFi network.
 *
 * Key Features:
 * - Real-time queue updates displayed on a web page.
 * - Remote input via IR remote control.
 * - LCD feedback on clients showing current token number and system status.
 * - EEPROM data persistence for queue numbers.
 * - Wireless communication over WiFi.
 * - Token navigation using IR remote buttons.
 *
 * System Workflow:
 * 1. Client Side (ESP8266):
 *    - Connects to ESP32 server's WiFi network.
 *    - Reads IR remote input and stores the queue number in EEPROM.
 *    - Sends the queue number to the ESP32 server via HTTP request.
 *    - Updates the LCD display with token number and connection status.
 *
 * 2. Server Side (ESP32):
 *    - Hosts a WiFi network for clients to connect.
 *    - Processes incoming HTTP requests from clients and stores queue numbers in EEPROM.
 *    - Updates and displays the queue number on the web page in real-time using AJAX.
 *    - Provides feedback to clients and manages the queue system.
 *
 * Required Components:
 * - ESP32 (Server)
 * - ESP8266 (Clients)
 * - IR Remote Control
 * - IR Receiver for ESP8266 Clients
 * - LCD I2C Display for Clients
 * - WiFi Network for Communication
 * - Power supply for ESP32 and ESP8266 modules
 *
 * Libraries Used:
 * - ESP8266WiFi.h: For WiFi communication on the ESP8266 clients.
 * - ESPAsyncWebServer.h: To host the web page on the ESP32 server.
 * - EEPROM.h: To store and retrieve queue numbers in EEPROM.
 * - IRremoteESP8266.h: To receive and decode IR signals on the ESP8266 clients.
 * - LiquidCrystal_I2C.h: To display queue numbers and system status on the LCD display.
 * - ESP8266HTTPClient.h: To send HTTP requests from the clients to the server.
 */
