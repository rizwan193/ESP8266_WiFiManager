#include <Config.h>

#define button 0
#define output 2

String state, val, data;
bool mustSaveConfig = false;

void saveConfigCallback()
{
  Serial.println("Should save config");
  mustSaveConfig = true;
}

void writeString(char address, String data)
{
  Serial.println("Cleaning EEPROM");
  
  for (int k = 0; k < 96; k++)
  {
    EEPROM.write(k, 0);
  }
  EEPROM.commit();
  Serial.println("EEPROM cleared");
  Serial.println("Writing Data...");
  int _size = data.length();
  for (int i = 0; i < _size; i++)
  {
    EEPROM.write(address+_size, data[i]);
  }
  EEPROM.write(address+_size, '\0');
  EEPROM.commit();
}

String readData(char address1)
{
  char data[1200];
  int len = 0;
  unsigned char j;
  j = EEPROM.read(address1);

  while (j != '\0' && len < 1200)
  {
    j = EEPROM.read(address1+len);
    data[len] = j;
    len++;
  }
  data[len] = '\0';
  Serial.println("\n");
  Serial.println("Reading Data from EEPROM");
  Serial.println(data);
  return String(data);
}

void setup()
{
  Serial.begin(115200);
  EEPROM.begin(512);
  pinMode(button, INPUT_PULLUP);
  pinMode(output, OUTPUT);
  digitalWrite(output, HIGH);
  SPIFFS.format();

  Serial.println("Mountung FS...");
  data = readData(15);
  data.toCharArray(API, 33);

  if (SPIFFS.begin())
  {
    Serial.println("Mounted file system");
    if (SPIFFS.exists("/config.json"))
    {
      Serial.println("Reading config file");
      File configFile = SPIFFS.open("/config.json", "r");
      if (configFile)
      {
        Serial.println("Opened config file");
        size_t size = configFile.size();
        
        std::unique_ptr<char[]> buf(new char(size));

        configFile.readBytes(buf.get(), size);
        DynamicJsonBuffer jsonBuffer;

        JsonObject& json = jsonBuffer.parseObject(buf.get());
        json.printTo(Serial);
        if(json.success())
        {
          Serial.println("\nparsed json");
          strcpy(API, json["API"]);
        }
        else
        {
          Serial.println("Failed to load json.config");
        }
      }
    }
  }
  else
  {
    Serial.println("Failed to mount FS");
  }
  WiFiManagerParameter custom_API("API", "API", API, 31);
  WiFiManager wifimanager;
  wifimanager.setSaveConfigCallback(saveConfigCallback);
  wifimanager.addParameter(&custom_API);
  wifimanager.resetSettings();

  wifimanager.autoConnect(ssid, password);
  Serial.println("Connected");

  strcpy (API, custom_API.getValue());

  if(strcmp(savedAPI, API) != 0)
  {
    Serial.println("API key invalid");
    Serial.println("Please enter correct API");
    delay(60000);
    wifimanager.resetSettings();
    ESP.reset();
  }
  else if (strcmp(savedAPI, API) == 0)
  {
    writeString(15, API);
    Serial.println("Entered API is correct");
    Serial.println ("Access granted");
    delay(60000);
  }

  if (mustSaveConfig)
  {
    Serial.println("saving config");
    DynamicJsonBuffer jsonBuffer;
    JsonObject& json = jsonBuffer.createObject();
    json["API"] = API;

    File configFile = SPIFFS.open("/config.json", "w");
    if(!configFile)
    {
      Serial.println("Failed to open config file for writing");
    }
    json.printTo(Serial);
    json.printTo(configFile);
    configFile.close();
  }
  server.begin();
  Serial.println(" ");
  Serial.println("Server connected");
  
  Serial.println("Local IP: ");
  Serial.println(WiFi.localIP());
  Serial.println("Subnet mask: ");
  Serial.println(WiFi.subnetMask());
  Serial.println("Gateway IP: ");
  Serial.println(WiFi.gatewayIP());
  Serial.println("MAC Address: ");
  Serial.println(WiFi.macAddress());
}


void loop()
{
  WiFiClient client = server.available();
  WiFiManager wifimanager;

  if(!client)
  {
    return;
  }

  state = client.readStringUntil('\r');
  Serial.println(state);
  client.flush();

  if(state.indexOf("/on") != -1)
  {
    digitalWrite(output, LOW);
    val = "on";
  }
  else if (state.indexOf("/off") != -1)
  {
    digitalWrite(output, HIGH);
    val = "off";
  }
  else if (state.indexOf("/reset") != -1)
  {
    wifimanager.resetSettings();
    ESP.reset();
  }

  String s = "HTTP/1.1.200 OK\r\n";
  s += "Conent-Type: application/json\r\n\r\n";
  s += "{\"data\":{\"message\":\"success\",\"value\":\"";
  s += val;
  s += "\"}}\r\n";
  s += "\n";

  client.println(s);
  delay(1);
  Serial.println("Client disconnected");
}