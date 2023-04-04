#include <Arduino.h>

#include <WiFi.h>
#include "AudioFileSourceVoicevox.h"
#include "AudioGeneratorWAW.h"
#include "AudioOutputI2SNoDAC.h"

const char* ssid = "your-ssid";
const char* password = "your-password";

const char *ENDPOINT = "https://example.com";

AudioGeneratorWAV *audioGenerator;
AudioFileSourceVoicevox *buff;
AudioOutputI2SNoDAC *out;

void setup()
{
  Serial.begin(115200);
  delay(1000);
  Serial.println("Connecting to WiFi");

  WiFi.disconnect();
  WiFi.softAPdisconnect(true);
  WiFi.mode(WIFI_STA);
  
  WiFi.begin(ssid, password);

  // Try forever
  while (WiFi.status() != WL_CONNECTED) {
    Serial.println("...Connecting to WiFi");
    delay(1000);
  }
  Serial.println("Connected");

  audioLogger = &Serial;

  buff = new AudioFileSourceVoicevox(ENDPOINT, "こんにちは", 10);
  out = new AudioOutputI2SNoDAC();
  audioGenerator = new AudioGeneratorWAV();
  audioGenerator->begin(buff, out);
}


void loop()
{
  static int lastms = 0;

  if (audioGenerator->isRunning()) {
    if (millis()-lastms > 1000) {
      lastms = millis();
      Serial.printf("Running for %d ms...\n", lastms);
      Serial.flush();
     }
    if (!audioGenerator->loop()) audioGenerator->stop();
  } else {
    Serial.printf("done\n");
    delay(1000);
  }
}
#endif
