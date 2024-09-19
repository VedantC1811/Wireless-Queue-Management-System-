
#include <WiFi.h>
#include <ESPAsyncWebServer.h>
#include <EEPROM.h>
#include <ESPmDNS.h>

int counter1 = 0;
int counter2 = 0;
int counter3 = 0;
int counter4 = 0;
int counter5 = 0;

const char *wifiSSID = "TP-Link_91F4";
const char *wifiPassword = "27296936";
IPAddress serverIp(192, 168, 0, 150); // Set the desired static IP address


// Define structure for EEPROM data
struct EEPROMData {
    int counter1;
    int counter2;
    int counter3;
    int counter4;
    int counter5;
    // Add more fields as needed
};
AsyncWebServer server(80);

// EEPROM addresses for storing counter values
#define COUNTER1_EEPROM_ADDRESS 0
#define COUNTER2_EEPROM_ADDRESS (COUNTER1_EEPROM_ADDRESS + sizeof(int))
#define COUNTER3_EEPROM_ADDRESS (COUNTER2_EEPROM_ADDRESS + sizeof(int))
#define COUNTER4_EEPROM_ADDRESS (COUNTER3_EEPROM_ADDRESS + sizeof(int))
#define COUNTER5_EEPROM_ADDRESS (COUNTER4_EEPROM_ADDRESS + sizeof(int))

const char *html = R"(
<!DOCTYPE html>
<html lang='en'>
<head>
    <meta charset='UTF-8'>
    <meta name='viewport' content='width=device-width, initial-scale=1.0'>
    <title>HPC TOKEN MACHINE</title>
    <style>
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

        button {
            margin-bottom: 10px;
            background-color: rgb(228, 92, 92);
            border: none;
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
        function updateReceivedValue1(value) {
            var receivedValue1Element = document.getElementById('receivedValue1');
            receivedValue1Element.textContent = value;
        }

        function updateReceivedValue2(value) {
            var receivedValue2Element = document.getElementById('receivedValue2');
            receivedValue2Element.textContent = value;
        }

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

        setInterval(fetchData1, 1000);
        setInterval(fetchData2, 1000);
    </script>
</body>
</html>

)";

void setup() {
    Serial.begin(115200);

   

    // Connect to Wi-Fi
    WiFi.begin(wifiSSID, wifiPassword);
    while (WiFi.status() != WL_CONNECTED) {
        Serial.println("Connecting to WiFi...");
        delay(100);
    }

    Serial.println("Connected to WiFi");
    Serial.print("IP Address: ");
    Serial.println(WiFi.localIP());

    WiFi.config(serverIp, IPAddress(192, 168, 0, 150), IPAddress(255, 255, 255, 0));  // Set gateway and subnet mask

    if (!MDNS.begin("HPC")) {
        Serial.println("MDNS responder started");
    }

    // Read initial values from EEPROM
    EEPROMData eepromData;
    EEPROM.begin(sizeof(eepromData));
    EEPROM.get(0, eepromData);
    counter1 = eepromData.counter1;
    counter2 = eepromData.counter2;
    counter3 = eepromData.counter3;
    counter4 = eepromData.counter4;
    counter5 = eepromData.counter5;
    EEPROM.end();

    // Handle the root URL request
    server.on("/", HTTP_GET, [](AsyncWebServerRequest *request) {
        // Send the HTML content as the response
        request->send(200, "text/html", html);
    });

    // Handle the /getCounter1 request for AJAX
    server.on("/getCounter1", HTTP_GET, [](AsyncWebServerRequest *request) {
        if (request->hasParam("value")) {
            String valueStr = request->getParam("value")->value();
            int receivedValue1 = valueStr.toInt();
            counter1 = receivedValue1;

            // Save the updated counter to EEPROM
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
     // Handle the /getCounter1 request for AJAX
    server.on("/getCounter2", HTTP_GET, [](AsyncWebServerRequest *request) {
        if (request->hasParam("value")) {
            String valueStr = request->getParam("value")->value();
            int receivedValue2 = valueStr.toInt();
            counter2 = receivedValue2;

            // Save the updated counter to EEPROM
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

    // Repeat the above pattern for /getCounter2, /getCounter3, /getCounter4, /getCounter5

    server.begin();
}

void loop() {
    // Your loop code goes here
}
