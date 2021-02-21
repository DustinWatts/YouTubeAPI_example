#include <WiFi.h>             // Wifi support
#include <HTTPClient.h>       // HTTP support
#include <WiFiClientSecure.h> // TLS Support
#include <ArduinoJson.h>      // Using ArduinoJson to process the API response

HTTPClient https;
WiFiClientSecure *client = new WiFiClientSecure;

// Your WiFi Details
const char *ssid = ""; // Your WiFi SSID
const char *password = ""; // Your WiFi Password

// Use https://developers.google.com/youtube/v3/docs/videos/list to get the liveChatID for the video (stream) you want.
const char *liveChatID = "";

// Use https://console.developers.google.com to get an API key.
const char *apiKey = "";

// We use the Root Certificate from the API.
const char* rootCACertificate = \
"-----BEGIN CERTIFICATE-----\n"
"MIIDujCCAqKgAwIBAgILBAAAAAABD4Ym5g0wDQYJKoZIhvcNAQEFBQAwTDEgMB4G\n"
"A1UECxMXR2xvYmFsU2lnbiBSb290IENBIC0gUjIxEzARBgNVBAoTCkdsb2JhbFNp\n"
"Z24xEzARBgNVBAMTCkdsb2JhbFNpZ24wHhcNMDYxMjE1MDgwMDAwWhcNMjExMjE1\n"
"MDgwMDAwWjBMMSAwHgYDVQQLExdHbG9iYWxTaWduIFJvb3QgQ0EgLSBSMjETMBEG\n"
"A1UEChMKR2xvYmFsU2lnbjETMBEGA1UEAxMKR2xvYmFsU2lnbjCCASIwDQYJKoZI\n"
"hvcNAQEBBQADggEPADCCAQoCggEBAKbPJA6+Lm8omUVCxKs+IVSbC9N/hHD6ErPL\n"
"v4dfxn+G07IwXNb9rfF73OX4YJYJkhD10FPe+3t+c4isUoh7SqbKSaZeqKeMWhG8\n"
"eoLrvozps6yWJQeXSpkqBy+0Hne/ig+1AnwblrjFuTosvNYSuetZfeLQBoZfXklq\n"
"tTleiDTsvHgMCJiEbKjNS7SgfQx5TfC4LcshytVsW33hoCmEofnTlEnLJGKRILzd\n"
"C9XZzPnqJworc5HGnRusyMvo4KD0L5CLTfuwNhv2GXqF4G3yYROIXJ/gkwpRl4pa\n"
"zq+r1feqCapgvdzZX99yqWATXgAByUr6P6TqBwMhAo6CygPCm48CAwEAAaOBnDCB\n"
"mTAOBgNVHQ8BAf8EBAMCAQYwDwYDVR0TAQH/BAUwAwEB/zAdBgNVHQ4EFgQUm+IH\n"
"V2ccHsBqBt5ZtJot39wZhi4wNgYDVR0fBC8wLTAroCmgJ4YlaHR0cDovL2NybC5n\n"
"bG9iYWxzaWduLm5ldC9yb290LXIyLmNybDAfBgNVHSMEGDAWgBSb4gdXZxwewGoG\n"
"3lm0mi3f3BmGLjANBgkqhkiG9w0BAQUFAAOCAQEAmYFThxxol4aR7OBKuEQLq4Gs\n"
"J0/WwbgcQ3izDJr86iw8bmEbTUsp9Z8FHSbBuOmDAGJFtqkIk7mpM0sYmsL4h4hO\n"
"291xNBrBVNpGP+DTKqttVCL1OmLNIG+6KYnX3ZHu01yiPqFbQfXf5WRDLenVOavS\n"
"ot+3i9DAgBkcRcAtjOj4LaR0VknFBbVPFd5uRHg5h6h+u/N5GJG79G+dwfCMNYxd\n"
"AfvDbbnvRG15RjF+Cv6pgsH/76tuIMRQyV+dTZsXjAzlAcmgQWpzU/qlULRuJQ/7\n"
"TBj0/VLZjmmx6BEP3ojY+x1J96relc8geMJgEtslQIxq/H5COEBkEveegeGTLg==\n"
"-----END CERTIFICATE-----";

void setup() {
  
  Serial.begin(9600);

  Serial.printf("Connecting to %s", ssid);
 
  WiFi.mode(WIFI_STA);
  WiFi.begin(ssid, password);
  while (WiFi.status() != WL_CONNECTED)
  {
    delay(500);
    Serial.print(".");
  }
  Serial.println("");
  Serial.print("Connected! IP address: ");
  Serial.println(WiFi.localIP());

  Serial.println("Started.");

}

void loop() {

  if(client) {
    client->setCACert(rootCACertificate);
   {

      // Construct the URL
      String requestURL = "https://youtube.googleapis.com/youtube/v3/liveChat/messages?liveChatId=";
      requestURL += liveChatID;
      requestURL += "&part=authorDetails,snippet&maxResults=20&key="; // Keep the maxResults as small as possbile
      requestURL += apiKey;

      if (https.begin(*client, requestURL)) {

        Serial.print("Getting... ");
          
        int httpCode = https.GET();
  
        if (httpCode > 0) {

          String response = https.getString();

          Serial.println("Response OK.");

          // Do something with the response.
          DynamicJsonDocument doc(24576);

          DeserializationError error = deserializeJson(doc, response);

          for (JsonObject elem : doc["items"].as<JsonArray>()) {

            // The etag is unique for each message, we could use this to keep track of which messages we already processed.
            //const char* etag = elem["etag"];
          
            JsonObject snippet = elem["snippet"];
            const char* snippet_displayMessage = snippet["displayMessage"];
                  
            JsonObject authorDetails = elem["authorDetails"];
            const char* authorDetails_displayName = authorDetails["displayName"];

            // For now it just prints the latest 20 messages to the serial
            Serial.printf("%s wrote : %s\n", authorDetails_displayName, snippet_displayMessage);
            
          }
  
        } else {
          Serial.printf("Request failed, error: %s\n", https.errorToString(httpCode).c_str());
          String payload = https.getString();
          Serial.println(String("Error:") + payload);
        }
  
        https.end();
      } else {
        Serial.printf("Unable to connect\n");
      }
    }
  
    delete client;
  } else {
    Serial.println("Unable to create client");
  }

  delay(4000);

}
