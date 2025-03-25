// Headers
#include<ESP8266WiFi.h>
#include <ESP8266WebServer.h>

// WiFi Definitions
const char* ssid = "CANDICONNECT";
const char* password = "C67PQOPASS";
IPAddress staticIP(192, 168, 4, 1); // Static IP address for the ESP8266
IPAddress gateway(0, 0, 0, 0);  // Gateway IP address
IPAddress subnet(255, 255, 255, 0);  // Subnet mask
ESP8266WebServer server(80);

//DEMUX variables
int variable1 = 5; 
int variable2 = 5;
int variable3 = 5;
int variable4 = 5;
int variable5 = 5;
int DMX1 = 0; 
int DMX2 = 0;
int DMX3 = 0;
int DMX4 = 0;
int DMX5 = 0;
bool OC = true;
bool OCtrig = true;

String can_id_1="";
String can_id_2="";
String can_id_3="";
String can_id_4="";
String can_id_5="";

String can_value_1="";
String can_value_2="";
String can_value_3="";
String can_value_4="";
String can_value_5="";

String time_1="";
String time_2="";
String time_3="";
String time_4="";
String time_5="";

String targets[] = {"V+","GND","SHLD","HI","LO","EMPTY"};

//local page build
const char index_html[] PROGMEM = R"rawliteral(
<!DOCTYPE HTML>
<html>
<head>
  <title>CAN Data Viewer</title>
  <div id="updateStatus" style="color: green;">Update On</div>
  <style>
    body {
      text-align: center;
    }
    table {
      margin: 0 auto;
      border-collapse: collapse;
      width: 80%;
    }
    th, td {
      border: 1px solid black;
      padding: 8px;
      text-align: center;
    }
    .input-column {
      width: 20px; /* Adjust the width as needed */
    }
    .user-input {
      text-align: center;
    }
  </style>
</head>
<body>
  <h1>CAN Data Viewer</h1>
  <table>
    <tr>
      <th>CAN ID</th>
      <th>Message Value</th>
      <th>Time of Message</th>
    </tr>
    <tr>
      <td>%CAN_ID_1%</td>
      <td>%MESSAGE_VALUE_1%</td>
      <td>%TIME_1%</td>
    </tr>
    <tr>
      <td>%CAN_ID_2%</td>
      <td>%MESSAGE_VALUE_2%</td>
      <td>%TIME_2%</td>
    </tr>
    <tr>
      <td>%CAN_ID_3%</td>
      <td>%MESSAGE_VALUE_3%</td>
      <td>%TIME_3%</td>
    </tr>
    <tr>
      <td>%CAN_ID_4%</td>
      <td>%MESSAGE_VALUE_4%</td>
      <td>%TIME_4%</td>
    </tr>
    <tr>
      <td>%CAN_ID_5%</td>
      <td>%MESSAGE_VALUE_5%</td>
      <td>%TIME_5%</td>
    </tr>
  </table>
  <h2>Pin combination</h2>
  <div id="pinControls" style="display: none;">
    <p class="user-input">Pin_1 to %VARIABLE1% <input type="text" class="input-column" id="userInput_1" placeholder=" "></p>
    <p class="user-input">Pin_2 to %VARIABLE2% <input type="text" class="input-column" id="userInput_2" placeholder=" "></p>
    <p class="user-input">Pin_3 to %VARIABLE3% <input type="text" class="input-column" id="userInput_3" placeholder=" "></p>
    <p class="user-input">Pin_4 to %VARIABLE4% <input type="text" class="input-column" id="userInput_4" placeholder=" "></p>
    <p class="user-input">Pin_5 to %VARIABLE5% <input type="text" class="input-column" id="userInput_5" placeholder=" "></p>
    <p><button onclick="sendUserInput()">Send</button></p>
  </div>
  <p><button id="toggleButton" onclick="togglePinControls()">Show Pin Controls</button></p>
  <p><button onclick="OpenClosePins()">%STATE%</button></p>
</body>
<script>
  var pinControlsVisible = false;
  var autoUpdateInterval;
  
  function togglePinControls() {
    pinControlsVisible = !pinControlsVisible;
    var pinControls = document.getElementById("pinControls");
    var toggleButton = document.getElementById("toggleButton");
    
    if (pinControlsVisible) {
      pinControls.style.display = "block";
      toggleButton.textContent = "Hide Pin Controls";
      updateStatus.textContent = "Update Off";
      updateStatus.style.color = "red";
      stopAutoUpdate(); // Stop auto-update when controls are visible
    } else {
      pinControls.style.display = "none";
      toggleButton.textContent = "Show Pin Controls";
      updateStatus.textContent = "Update On";
      updateStatus.style.color = "green";
      startAutoUpdate(); // Start auto-update when controls are not visible
    }
  }
  
  function sendUserInput() {
    var userInput1 = document.getElementById("userInput_1").value;
    var userInput2 = document.getElementById("userInput_2").value;
    var userInput3 = document.getElementById("userInput_3").value;
    var userInput4 = document.getElementById("userInput_4").value;
    var userInput5 = document.getElementById("userInput_5").value;
    var xhr = new XMLHttpRequest();
    xhr.open("GET", "/update?value1=" + userInput1 + "&value2=" + userInput2 + "&value3=" + userInput3 + "&value4=" + userInput4 + "&value5=" + userInput5, true);
    xhr.send();
    setTimeout(function() {location.reload();}, 1000);
  }
  
  function OpenClosePins() {
    var xhr = new XMLHttpRequest();
    xhr.open("GET", "/openclose", true);
    xhr.send();
    setTimeout(function() {location.reload();}, 1000);
  }
  
  function startAutoUpdate() {
    autoUpdateInterval = setInterval(function() {
      location.reload();
    }, 2000);
  }
  
  function stopAutoUpdate() {
    clearInterval(autoUpdateInterval);
  }
</script>
</html>
)rawliteral";

// GPIO definitions
#define SCLK      D0    //shiftregister clock
#define EN        D1    //DEMUX enable (inverted)
#define DS2       D2    //Shiftregister B dataline
#define DS1       D3    //Shiftregister A dataline
#define MR        D4    //Shiftregister master reset (trigger)
#define HI_ALERT  D5    //CAN_HI line overvoltage (above 3V)
#define LO_ALERT  D6    //CAN_LO line overvoltagr (below -3V)
#define V_ALERT   D7    //CAN supply overvoltage above (7V)
#define LED       D8    //indicator light

void setup() {
  
  // Setup Serial
  Serial.begin(115200);
  delay(500);
  Serial.println("\nSerial Communication begun");
  // Set ESP8266 to access point mode with a static IP
  WiFi.mode(WIFI_AP);
  WiFi.softAPConfig(staticIP, gateway, subnet);
  WiFi.softAP(ssid, password);

  // Print the IP address of the ESP8266
  Serial.println("Access Point IP address: ");
  Serial.println(WiFi.softAPIP());
  delay(500);

  // Configure web server routes
  // Display values
  server.on("/", HTTP_GET, []() {
    String modifiedHtml = String(index_html);
    modifiedHtml.replace("%VARIABLE1%", targets[variable1]);
    modifiedHtml.replace("%VARIABLE2%", targets[variable2]);
    modifiedHtml.replace("%VARIABLE3%", targets[variable3]);
    modifiedHtml.replace("%VARIABLE4%", targets[variable4]);
    modifiedHtml.replace("%VARIABLE5%", targets[variable5]);
    if (OC) {modifiedHtml.replace("%STATE%", "Connect to bus");}
    else {modifiedHtml.replace("%STATE%", "Disconnect from bus");}
    modifiedHtml.replace("%CAN_ID_1%", can_id_1);
    modifiedHtml.replace("%CAN_ID_2%", can_id_2);
    modifiedHtml.replace("%CAN_ID_3%", can_id_3);
    modifiedHtml.replace("%CAN_ID_4%", can_id_4);
    modifiedHtml.replace("%CAN_ID_5%", can_id_5);
    modifiedHtml.replace("%MESSAGE_VALUE_1%", can_value_1);
    modifiedHtml.replace("%MESSAGE_VALUE_2%", can_value_2);
    modifiedHtml.replace("%MESSAGE_VALUE_3%", can_value_3);
    modifiedHtml.replace("%MESSAGE_VALUE_4%", can_value_4);
    modifiedHtml.replace("%MESSAGE_VALUE_5%", can_value_5);
    modifiedHtml.replace("%TIME_1%", time_1);
    modifiedHtml.replace("%TIME_2%", time_2);
    modifiedHtml.replace("%TIME_3%", time_3);
    modifiedHtml.replace("%TIME_4%", time_4);
    modifiedHtml.replace("%TIME_5%", time_5);

    server.send(200, "text/html", modifiedHtml);
  });

  // Get pin values from user
  server.on("/update", HTTP_GET, []() {
    String value1 = server.arg("value1");
    variable1 = value1.toInt();
    String value2 = server.arg("value2");
    variable2 = value2.toInt();
    String value3 = server.arg("value3");
    variable3 = value3.toInt();
    String value4 = server.arg("value4");
    variable4 = value4.toInt();
    String value5 = server.arg("value5");
    variable5 = value5.toInt();
    server.send(200, "text/plain", "Values recieved");
  });

  //Get enable state from user
  server.on("/openclose", HTTP_GET, []() {
    OC = !OC;
    server.send(200, "text/plain", "Pin open-close variable flipped");
  });
  
  server.begin();
  Serial.println("server begun");
  delay(500);
  
  // Setup GPIOs
  pinMode(SCLK,OUTPUT);digitalWrite(SCLK,LOW);
  pinMode(EN,OUTPUT);digitalWrite(EN,HIGH);
  pinMode(DS1,OUTPUT);digitalWrite(DS1,LOW);
  pinMode(DS2,OUTPUT);digitalWrite(DS2,LOW);
  pinMode(MR,OUTPUT);digitalWrite(MR,HIGH);
  pinMode(LED,OUTPUT);digitalWrite(LED,HIGH);
  pinMode(HI_ALERT,INPUT);
  pinMode(LO_ALERT,INPUT);
  pinMode(V_ALERT,INPUT);
  Serial.println("GPIOS Configured");
  delay(500);
}

void ProcessSerial(String raw){
  can_id_1=raw;
  can_id_2="";
  can_id_3="";
  can_id_4="";
  can_id_5="";
  can_value_1="";
  can_value_2="";
  can_value_3="";
  can_value_4="";
  can_value_5="";
  time_1="";
  time_2="";
  time_3="";
  time_4="";
  time_5="";
}

void ConfigurePINS(){
  
}

void loop() {
  server.handleClient();
  if (Serial.available()>0){
    ProcessSerial(Serial.readString());
  }
  if (!OC and OCtrig) {
    
  }
}
