/*
  AudioFileSourceVoicevox
  Connect to a VOICEVOX service
  
  Copyright (C) 2023  t_furu

  This program is free software: you can redistribute it and/or modify
  it under the terms of the GNU General Public License as published by
  the Free Software Foundation, either version 3 of the License, or
  (at your option) any later version.

  This program is distributed in the hope that it will be useful,
  but WITHOUT ANY WARRANTY; without even the implied warranty of
  MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.  See the
  GNU General Public License for more details.

  You should have received a copy of the GNU General Public License
  along with this program.  If not, see <http://www.gnu.org/licenses/>.
*/

#if defined(ESP32) || defined(ESP8266)

#include "AudioFileSourceVoicevox.h"

AudioFileSourceVoicevox::AudioFileSourceVoicevox()
{
  pos = 0;
  reconnectTries = 0;
  saveURL[0] = 0;
}

AudioFileSourceVoicevox::AudioFileSourceVoicevox(const char* endpoint, const char * text, int speaker)
{
  saveURL[0] = 0;
  reconnectTries = 0;
  this->endpoint = String(endpoint);

  httpInit();
  int result = postAudioQuery(text, speaker);
  if (result != 0) {
    return;
  }
  postSynthesis(speaker);
}

void AudioFileSourceVoicevox::httpInit() {
  pos = 0;
  http.setFollowRedirects(HTTPC_STRICT_FOLLOW_REDIRECTS); 
  http.setTimeout( 30 * 1000 ); 
  http.setReuse(true);
}

bool AudioFileSourceVoicevox::open(const char *url)
{
  /*
  pos = 0;
  http.setFollowRedirects(HTTPC_STRICT_FOLLOW_REDIRECTS); 
  http.setTimeout( 30 * 1000 ); 
  http.setReuse(true);
  http.begin(client, url); 
  int code = http.GET();
  if (code != HTTP_CODE_OK) {
    http.end();
    cb.st(STATUS_HTTPFAIL, PSTR("Can't open HTTP request"));
    return false;
  }
  size = http.getSize();
  strncpy(saveURL, url, sizeof(saveURL));
  saveURL[sizeof(saveURL)-1] = 0;
  return true;
  */
  return false;
}

AudioFileSourceVoicevox::~AudioFileSourceVoicevox()
{
  http.end();
}


uint32_t AudioFileSourceVoicevox::read(void *data, uint32_t len)
{
  if (data==NULL) {
    audioLogger->printf_P(PSTR("ERROR! AudioFileSourceVoicevox::read passed NULL data\n"));
    return 0;
  }
  return readInternal(data, len, true);
}

uint32_t AudioFileSourceVoicevox::readInternal(void *data, uint32_t len, bool nonBlock)
{
retry:
  if (!http.connected()) {
    cb.st(STATUS_DISCONNECTED, PSTR("Stream disconnected"));
    http.end();
    for (int i = 0; i < reconnectTries; i++) {
      char buff[64];
      sprintf_P(buff, PSTR("Attempting to reconnect, try %d"), i);
      cb.st(STATUS_RECONNECTING, buff);
      delay(reconnectDelayMs);
      if (open(saveURL)) {
        cb.st(STATUS_RECONNECTED, PSTR("Stream reconnected"));
        break;
      }
    }
    if (!http.connected()) {
      cb.st(STATUS_DISCONNECTED, PSTR("Unable to reconnect"));
      return 0;
    }
  }
  if ((size > 0) && (pos >= size)) return 0;

  WiFiClient *stream = http.getStreamPtr();

  // Can't read past EOF...
  if ( (size > 0) && (len > (uint32_t)(pos - size)) ) len = pos - size;

  if (!nonBlock) {
    int start = millis();
    while ((stream->available() < (int)len) && (millis() - start < 500)) yield();
  }

  size_t avail = stream->available();
  if (!nonBlock && !avail) {
    cb.st(STATUS_NODATA, PSTR("No stream data available"));
    http.end();
    goto retry;
  }
  if (avail == 0) return 0;
  if (avail < len) len = avail;
  
  int read = stream->read(reinterpret_cast<uint8_t*>(data), len);
  pos += read;
  return read;
}

bool AudioFileSourceVoicevox::seek(int32_t pos, int dir)
{
  audioLogger->printf_P(PSTR("ERROR! AudioFileSourceVoicevox::seek not implemented!"));
  (void) pos;
  (void) dir;
  return false;
}

bool AudioFileSourceVoicevox::close()
{
  http.end();
  return true;
}

bool AudioFileSourceVoicevox::isOpen()
{
  return http.connected();
}

uint32_t AudioFileSourceVoicevox::getSize()
{
  return size;
}

uint32_t AudioFileSourceVoicevox::getPos()
{
  return pos;
}

int AudioFileSourceVoicevox::postAudioQuery(const char * text, int speaker) {
  String url = String(endpoint) + "/audio_query";
  String query = "?speaker=" + String(speaker) + "&text=" + urlEncode( String(text) );

  http.begin(url + query);
  int code = http.POST( query );
  if (code != HTTP_CODE_OK) {
    http.end();
    cb.st(STATUS_HTTPFAIL, PSTR("Can't open HTTP request"));
    return -1;
  } 
  audioQueryStr = http.getString();

  http.end();
  return 0;
}

int AudioFileSourceVoicevox::postSynthesis(int speaker) {
  String url = String(endpoint) + "/synthesis?speaker=" + String(speaker);

  http.begin(url);
  http.addHeader("Content-Type", "application/json");
  int code = http.POST(this->audioQueryStr);
  if (code != HTTP_CODE_OK) {
    http.end();
    cb.st(STATUS_HTTPFAIL, PSTR("Can't open HTTP request"));
    return -1;
  }    
  size = http.getSize();
  return 0;
}

#endif
