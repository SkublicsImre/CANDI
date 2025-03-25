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

// DEMUX variables
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

// CAN variables
  bool NewID=true;
  bool FoundID=false;
  String can_id[20];
  String can_vals[20];
  uint8_t catchtime[20];
  uint8_t catchmils[20];
  int ModulCount = 0;

  String targets[6] = {"V+","GND","SHLD","HI","LO","EMPTY"};
  String can_raw = "\n";
  String can_raw_holder[10] = {"","","","","","","","","",""};
  int can_raw_counter = 0;

  String Serialmessage="";
  String vcolor="green";
  String hcolor="green";
  String lcolor="green";

// Local page build
  const char index_html[] PROGMEM = R"rawliteral(
      <!DOCTYPE HTML>
  <html>
  <head>
      <title>CAN Data Viewer</title>
      <div id="updateStatus" style="color: green; font-size: 30px;">Update On</div>
      <style>
          body {
              text-align: center;
              overflow-x: hidden;
          }
          table {
              margin: 0 auto;
              border-collapse: collapse;
              width: 90%;
              font-size: 30px;
          }
          @media (max-width: 930px) {
            body {
                font-size: 20px;
            }
          }
          th, td {
              border: 1px solid black;
              padding: 10px;
              text-align: center;
          }

          .alert {
              color: rgb(0, 0, 0);
              padding: 10px;
              text-align: center;
              width: 100px;
              margin: 0 auto;
          }

          .input-column {
              width: 45px; 
              font-size: 40px;
          }
          .user-input {
              text-align: center;
              font-size: 30px;
          }
          .Sinputbar{
              width: 80%;
              font-size: 40px;
              text-align: center;
          }
          .terminal {
              background-color: rgb(0, 0, 0);
              color: white;
              padding: 20px;
              text-align: left;
              width: 90%;
              margin: 0 auto;
          }
          .button {
            font-size: 30px; 
            padding: 15px 150px; 
          }
          .button2 {
            font-size: 30px;
            padding: 15px 30px; 
          }
      </style>
  </head>
  <body>
      <h1>CAN Data Viewer</h1>
      <a class="alert" style="background-color: %VCOLOR%">VOLTAGE</a>  <a class="alert" style="background-color: %HCOLOR%">CAN HI</a> <a class="alert" style="background-color: %LCOLOR%">CAN LO</a>
      <p></p>
      <table>
          <tr>
              <th>CAN ID</th>
              <th>Message Value</th>
              <th>Time of Message</th>
          </tr>
          <!--GhostRow0-->
          <!--GhostRow1-->
          <!--GhostRow2-->
          <!--GhostRow3-->
          <!--GhostRow4-->
          <!--GhostRow5-->
          <!--GhostRow6-->
          <!--GhostRow7-->
          <!--GhostRow8-->
          <!--GhostRow9-->
          <!--GhostRow10-->
          <!--GhostRow11-->
          <!--GhostRow12-->
          <!--GhostRow13-->
          <!--GhostRow14-->
          <!--GhostRow15-->
          <!--GhostRow16-->
          <!--GhostRow17-->
          <!--GhostRow18-->
          <!--GhostRow19-->
      </table>
      <div id="pinControls" style="display: none;">
          <p class="user-input">Pin_1 to %VARIABLE1% <input type="text" class="input-column" id="userInput_1" placeholder=" "></p>
          <p class="user-input">Pin_2 to %VARIABLE2% <input type="text" class="input-column" id="userInput_2" placeholder=" "></p>
          <p class="user-input">Pin_3 to %VARIABLE3% <input type="text" class="input-column" id="userInput_3" placeholder=" "></p>
          <p class="user-input">Pin_4 to %VARIABLE4% <input type="text" class="input-column" id="userInput_4" placeholder=" "></p>
          <p class="user-input">Pin_5 to %VARIABLE5% <input type="text" class="input-column" id="userInput_5" placeholder=" "></p>
          <p><input type="text" class="Sinputbar" id="SeruserInput" placeholder=" "></p>
          <a><button class="button2" onclick="sendUserInput()">Send pin data</button></a> <a><button class="button2" onclick="sendSerUserInput()">Send serial data</button></a>
      </div>
      <p><button class="button" id="toggleButton" onclick="togglePinControls()">Show Controls</button></p>
      <p><button class="button" onclick="OpenClosePins()">%STATE%</button></p>
      <div class="terminal">
          <pre id="canTerminal">Raw CAN data: %RAWCONTENTS%
          </pre>
      </div>
  </body>
  <script>
      var pinControlsVisible = false;
      var autoUpdateInterval = setInterval(function () {
          location.reload();
      }, 2000);

      function togglePinControls() {
          pinControlsVisible = !pinControlsVisible;
          var pinControls = document.getElementById("pinControls");
          var toggleButton = document.getElementById("toggleButton");

          if (pinControlsVisible) {
              pinControls.style.display = "block";
              toggleButton.textContent = "Hide Controls";
              updateStatus.textContent = "Update Off";
              updateStatus.style.color = "red";
              stopAutoUpdate();
          } else {
              pinControls.style.display = "none";
              toggleButton.textContent = "Show Controls";
              updateStatus.textContent = "Update On";
              updateStatus.style.color = "green";
              startAutoUpdate();
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
          setTimeout(function () {
              location.reload();
          }, 200);
      }

      function sendSerUserInput() {
          var SeruserInput = document.getElementById("SeruserInput").value;
          var xhr = new XMLHttpRequest();
          xhr.open("GET", "/pushserial?serialmessage="+SeruserInput, true);
          xhr.send();
          setTimeout(function () {
              location.reload();
          }, 200);
      }

      function OpenClosePins() {
          var xhr = new XMLHttpRequest();
          xhr.open("GET", "/openclose", true);
          xhr.send();
          setTimeout(function () {
              location.reload();
          }, 200);
      }

      function startAutoUpdate() {
          autoUpdateInterval = setInterval(function () {
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

// Debug setup
  bool debug=false;
  void dss(String debugMessage){
    if (debug){Serial.println(debugMessage);}
  }
  void dsip(IPAddress debugMessage){
    if (debug){Serial.println(debugMessage);}
  }

void setup() {
  // Setup Serial
    Serial.begin(115200);
    delay(50);
    dss("\nSerial Communication begun");
  // Set ESP8266 to access point mode with a static IP
    WiFi.mode(WIFI_AP);
    WiFi.softAPConfig(staticIP, gateway, subnet);
    WiFi.softAP(ssid, password);

  // Print the IP address of the ESP8266
    dss("Access Point IP address: ");
    dsip(WiFi.softAPIP());
    delay(50);

  // Configure web server routes
    // Display values
    server.on("/", HTTP_GET, []() {
      String modifiedHtml = String(index_html);
      modifiedHtml.replace("%VARIABLE1%", targets[variable1]);
      modifiedHtml.replace("%VARIABLE2%", targets[variable2]);
      modifiedHtml.replace("%VARIABLE3%", targets[variable3]);
      modifiedHtml.replace("%VARIABLE4%", targets[variable4]);
      modifiedHtml.replace("%VARIABLE5%", targets[variable5]);
      modifiedHtml.replace("%VCOLOR%", vcolor);
      modifiedHtml.replace("%HCOLOR%", hcolor);
      modifiedHtml.replace("%LCOLOR%", lcolor);
      for (int i=0;i<=ModulCount;i++){
        modifiedHtml.replace("<!--GhostRow"+String(i)+"-->","<tr><th>"+can_id[i]+"</th><th>"+can_vals[i]+"</th><th>"+String(catchtime[i])+"</th></tr>");
      }
      if (OC) {modifiedHtml.replace("%STATE%", "Connect to bus");}
      else {modifiedHtml.replace("%STATE%", "Disconnect from bus");}
      modifiedHtml.replace("%RAWCONTENTS%", can_raw);
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
      Serialmessage = server.arg("serialmessage");
      server.send(200, "text/plain", "Values recieved");
      dss("Updated Pin values");
    });
    server.on("/pushserial", HTTP_GET, [](){
      Serialmessage = server.arg("serialmessage");
      Serial.print(Serialmessage);
      server.send(200, "text/plain", "Values printed");
    });
    //Get enable state from user
    server.on("/openclose", HTTP_GET, []() {
      OC = !OC;
      dss("Enable pin written to: ");
      dss(String(OC));
      digitalWrite(EN,OC);
      server.send(200, "text/plain", "Pin open-close variable flipped");
      dss("Pin connection status changed");
    });
  // Begin server
    server.begin();
    dss("Server begun");
    delay(50);
  
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
    dss("GPIOS Configured");
    delay(50);
}
void ProcessSerial(String raw){
  // Raw data rolling display
    if (can_raw_counter>9){
      can_raw_counter=8;
      can_raw_holder[9]="";
      for (int i=0;i<8;i++){
        can_raw_holder[i]=can_raw_holder[i+1];
      }
    }
      can_raw_holder[can_raw_counter]=raw;
      can_raw="\n";
    for (int i=0;i<=can_raw_counter;i++){
        can_raw += can_raw_holder[i]+"\n";
    }
    can_raw_counter++;
  // Identify Modules by ID tag
    String IDtag = "ID";
    int IDlenght = 3;
    int datalenght = 8;
    int IDpos=raw.indexOf(IDtag);
  // Check with allready saved IDs 
    if(IDpos != -1){
      uint8_t currentt=millis();
      String ID = raw.substring(IDtag.length(),IDpos + IDtag.length()+IDlenght);
      String val = raw.substring(IDtag.length()+IDpos+IDlenght,IDpos+IDtag.length()+IDlenght+datalenght);
      NewID=true;
      FoundID=true;
      for(int i=0;i<=ModulCount;i++){
        if(ID==can_id[i]){
          can_vals[i]=val;
          catchtime[i]=millis()-catchtime[ModulCount];
          NewID = false;
        }
      }
  // If New ID found create placeholder (up to 20 consecutive module)
      if(NewID){
        can_id[ModulCount]=ID;
        can_vals[ModulCount]=val;
        catchtime[ModulCount]=currentt-catchmils[ModulCount];
        catchmils[ModulCount]=currentt;
        ModulCount++;
        if(ModulCount>=20){ModulCount=19;dss("Alert: memory overflow");}
      }
      
    }else{FoundID=false;}
  
}
void ConfigurePINS(){
  // disconect from bus
    OC=true;
    digitalWrite(EN,OC);
    dss("Enable pin written to: ");
    dss(String(OC));
    dss("\n");
    delay(10);
  
  // set variables to the requested values
    DMX1=variable1;
    DMX2=variable2;
    DMX3=variable3;
    DMX4=variable4;
    DMX5=variable5;
  // Distribute pin numbers into two 8 bit arrays
    int DMX[5]={DMX1,DMX2,DMX3,DMX4,DMX5};
    bool SRG1[8]={0,0,0,0,0,0,0,0};
    bool SRG2[8]={0,0,0,0,0,0,0,0};
    for (int nums=0;nums<5;nums++){ //iterate trough nums
      for (int bits=0;bits<3;bits++){ //iterate by bit
        int val = DMX[nums]; //define cureently digested value
        if (nums<2) {SRG2[7-(nums*3+bits)]=bitRead(val,bits);} //if its DMX1 or DMX2
        else {if (nums==2&&bits==0) {SRG2[1]=bitRead(val,bits);} //if its the first bit of DMX3
        else {if (nums>=2) {SRG1[(8-((nums-2)*3+bits))]=bitRead(val,bits);}} //if its DMX3's second and third bit or DMX4 or DMX5
        }
      }
    }
  // Shift out data
    digitalWrite(MR,LOW); //reset shiftreg
    digitalWrite(SCLK,LOW); //prep clock for rising trigger
    delay(5);
    digitalWrite(MR,HIGH);//enable write
    delay(5);
    for(int i=0;i<8;i++){
      digitalWrite(DS1,SRG1[i]);//set data
      digitalWrite(DS2,SRG2[i]);
      delay(1);
      digitalWrite(SCLK,HIGH);//trigger
      delay(1);
      digitalWrite(SCLK,LOW);//reset clock
      delay(1);
    }
    digitalWrite(SCLK,LOW);//set data and clock to low after finishing shift out
    digitalWrite(DS1,LOW);
    digitalWrite(DS2,LOW);
}
void loop() {
  // Update server info
    server.handleClient();
    if (Serial.available()>0){
      ProcessSerial(Serial.readString());
    }
  
  // Check pinconfig request 
    //if pinnumber change detected
    if (DMX1!=variable1||DMX2!=variable2||DMX3!=variable3||DMX4!=variable4||DMX5!=variable5){
        OCtrig=true;//set trigger variable
    if (OCtrig){
        OCtrig=false;//reset trigger variable
        ConfigurePINS();//set up new pin configuration
      }
    }
  // Check alerts
    if(digitalRead(V_ALERT)){digitalWrite(EN,HIGH);OC=true;dss("Overvoltage Alert");vcolor="red";}else{vcolor="green";}
    if(digitalRead(HI_ALERT)){digitalWrite(EN,HIGH);OC=true;dss("CAN_HI spike Alert");hcolor="red";}else{hcolor="green";}
    if(digitalRead(LO_ALERT)){digitalWrite(EN,HIGH);OC=true;dss("CAN_LO spike Alert");lcolor="red";}else{lcolor="green";}
}
