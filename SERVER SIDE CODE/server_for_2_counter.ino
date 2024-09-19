/*
  Project: Wireless Queue Management System using ESP32
  Description: 
  This project sets up a wireless queue management system using an ESP32 module. 
  The ESP32 serves as a web server, displaying token numbers (counters) on a web page, 
  which is updated dynamically using AJAX. The token counters are persisted using 
  EEPROM, so they are retained even after the device restarts.

  Components:
  1. ESP32: Acts as the web server and handles Wi-Fi connection.
  2. EEPROM: Used to store and persist counter values across reboots.
  3. Web Interface: Displays the token counters and auto-updates using AJAX calls.
  4. AJAX: Fetches and updates the counter values from the ESP32 server in real-time.

  Key Features:
  1. Static IP address configuration for the ESP32.
  2. MDNS service for easy access on local networks.
  3. Use of EEPROM to retain counter states between restarts.
  4. Responsive and dynamically updating web page to display the token numbers.

  How it Works:
  - The ESP32 connects to a Wi-Fi network and hosts a web server.
  - The counters (token numbers) are stored in EEPROM for persistence.
  - A web page is served, displaying the counters and updating them in real-time using JavaScript.
  - AJAX requests fetch the latest counter values from the server every second.
*/


#include <WiFi.h>
#include <ESPAsyncWebServer.h>
#include <EEPROM.h>
#include <ESPmDNS.h>

// Counters for token numbers displayed on the web page
int counter1 = 0;
int counter2 = 0;
int counter3 = 0;
int counter4 = 0;
int counter5 = 0;

// Wi-Fi credentials for connecting to the local network
const char *wifiSSID = "TP-Link_91F4";
const char *wifiPassword = "27296936";

// Static IP address assigned to the ESP32 server
IPAddress serverIp(192, 168, 0, 150);

// Structure to store counter values in EEPROM for persistence
struct EEPROMData {
    int counter1;
    int counter2;
    int counter3;
    int counter4;
    int counter5;
};

// Create a web server object that listens on port 80
AsyncWebServer server(80);

// EEPROM addresses to store each counter value
#define COUNTER1_EEPROM_ADDRESS 0
#define COUNTER2_EEPROM_ADDRESS (COUNTER1_EEPROM_ADDRESS + sizeof(int))
#define COUNTER3_EEPROM_ADDRESS (COUNTER2_EEPROM_ADDRESS + sizeof(int))
#define COUNTER4_EEPROM_ADDRESS (COUNTER3_EEPROM_ADDRESS + sizeof(int))
#define COUNTER5_EEPROM_ADDRESS (COUNTER4_EEPROM_ADDRESS + sizeof(int))

// HTML page template to display counters on the web page
const char *html = R"(
<!DOCTYPE html>
<html lang='en'>
<head>
    <meta charset='UTF-8'>
    <meta name='viewport' content='width=device-width, initial-scale=1.0'>
    <title>HPC TOKEN MACHINE</title>
    <style>
        /* CSS styling for the layout and design of the web page */
        * {
            margin: 0;
            padding: 0;
            box-sizing: border-box;
        }

        body {
            font-family: Arial, sans-serif;
            background-color: white;
        }

        .container {
            display: grid;
            grid-template-columns: repeat(2, 1fr);
            gap: 15px;
            background-color: white;
            border-radius: 10px;
            box-shadow: 0 0 10px rgba(0, 0, 0, 0.1);
            overflow: hidden;
        }

        /* Styles for different sections of the page */
        .item1,
        .heading,
        .item,
        .counter-row {
            height: 70px;
            font-size: 1.5em;
            text-align: center;
            display: flex;
            align-items: center;
            justify-content: center;
        }

        .item1 {
            background-color: rgb(29, 192, 241);
            font-size: 40px;
            position: sticky;
            top: 3px;
            z-index: 100;
            height: 76px;
        }

        .item {
            background-color: rgb(29, 192, 241);
            color: white;
            height: 42vh;
            font-size: 70px;
            position: relative;
        }

        .value1,
        .value2 {
            background-color: rgb(29, 192, 241);
            color: white;
            height: 30px;
            font-size: 150px;
            text-align: center;
            display: flex;
            align-items: center;
            justify-content: center;
        }

        img {
            position: absolute;
            bottom: 0;
            right: 0;
            max-height: 150px;
        }
    </style>
</head>
<body>
    <div class='container'>
        <!-- Display for counter names and token numbers -->
        <div class='item1'>COUNTER NAME</div>
        <div class='item1'>TOKEN NUMBERS</div>
        <div class='item'>COUNTER 1</div>
        <div class='item'>
            <div class='value1' id='receivedValue1'>0</div>
        </div>
        <div class='item'>COUNTER 2</div>
        <div class='item'>
            <div class='value2' id='receivedValue2'>0</div>
            <img src='https://hpcembedded.com/wp-content/uploads/2020/09/logo-bill.png' alt='image'>
        </div>
    </div>

    <script>
        // Functions to update the displayed token values for counter 1 and 2
        function updateReceivedValue1(value) {
            var receivedValue1Element = document.getElementById('receivedValue1');
            receivedValue1Element.textContent = value;
        }

        function updateReceivedValue2(value) {
            var receivedValue2Element = document.getElementById('receivedValue2');
            receivedValue2Element.textContent = value;
        }

        // Periodically fetch the latest counter values from the server
        function fetchData1() {
            fetch('/getCounter1')
                .then(response => response.text())
                .then(data => {
                    updateReceivedValue1(data);
                });
        }

        function fetchData2() {
            fetch('/getCounter2')
                .then(response => response.text())
                .then(data => {
                    updateReceivedValue2(data);
                });
        }

        // Update values every second
        setInterval(fetchData1, 1000);
        setInterval(fetchData2, 1000);
    </script>
</body>
</html>
)";

void setup() {
    Serial.begin(115200);

    // Connect to the specified Wi-Fi network
    WiFi.begin(wifiSSID, wifiPassword);
    while (WiFi.status() != WL_CONNECTED) {
        Serial.println("Connecting to WiFi...");
        delay(100);
    }

    Serial.println("Connected to WiFi");
    Serial.print("IP Address: ");
    Serial.println(WiFi.localIP());

    // Set the ESP32's static IP address
    WiFi.config(serverIp, IPAddress(192, 168, 0, 150), IPAddress(255, 255, 255, 0));

    // Start MDNS service to allow local network access via "HPC.local"
    if (!MDNS.begin("HPC")) {
        Serial.println("MDNS responder started");
    }

    // Read stored counter values from EEPROM
    EEPROMData eepromData;
    EEPROM.begin(sizeof(eepromData));
    EEPROM.get(0, eepromData);
    counter1 = eepromData.counter1;
    counter2 = eepromData.counter2;
    counter3 = eepromData.counter3;
    counter4 = eepromData.counter4;
    counter5 = eepromData.counter5;
    EEPROM.end();

    // Handle requests to the root URL ("/")
    server.on("/", HTTP_GET, [](AsyncWebServerRequest *request) {
        request->send(200, "text/html", html);
    });

    // Handle AJAX request for counter 1
    server.on("/getCounter1", HTTP_GET, [](AsyncWebServerRequest *request) {
        if (request->hasParam("value")) {
            String valueStr = request->getParam("value")->value();
            int receivedValue1 = valueStr.toInt();
            counter1 = receivedValue1;

            // Save the updated counter value to EEPROM
            EEPROMData eepromData;
            eepromData.counter1 = counter1;
            eepromData.counter2 = counter2;
            eepromData.counter3 = counter3;
            eepromData.counter4 = counter4;
            eepromData.counter5 = counter5;
            EEPROM.begin(sizeof(eepromData));
            EEPROM.put(0, eepromData);
            EEPROM.commit();
            EEPROM.end();
        }
        request->send(200, "text/plain", String(counter1));
    });

    // Handle AJAX request for counter 2
    server.on("/getCounter2", HTTP_GET, [](AsyncWebServerRequest *request) {
        if (request->hasParam("value")) {
            String valueStr = request->getParam("value")->value();
            int receivedValue2 = valueStr.toInt();
            counter2 = receivedValue2;

            // Save the updated counter value to EEPROM
            EEPROMData eepromData;
            eepromData.counter1 = counter1;
            eepromData.counter2 = counter2;
            eepromData.counter3 = counter3;
            eepromData.counter4 = counter4;
            eepromData.counter5 = counter5;
            EEPROM.begin(sizeof(eepromData));
            EEPROM.put(0, eepromData);
            EEPROM.commit();
            EEPROM.end();
        }
        request->send(200, "text/plain", String(counter2));
    });

    // Start the server
    server.begin();
}

void loop() {
    // Main loop remains empty since the server handles everything
}
