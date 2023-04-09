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
#pragma once

#include <Arduino.h>
#include <HTTPClient.h>
#include "AudioFileSource.h"
#include <UrlEncode.h>

class AudioFileSourceVoicevox : public AudioFileSource
{
  friend class AudioFileSourceICYStream;

  public:
    AudioFileSourceVoicevox();
    AudioFileSourceVoicevox(const char* endpoint);
    virtual ~AudioFileSourceVoicevox() override;
    
    virtual bool open(const char *url) override;
    virtual uint32_t read(void *data, uint32_t len) override;
    virtual bool seek(int32_t pos, int dir) override;
    virtual bool close() override;
    virtual bool isOpen() override;
    virtual uint32_t getSize() override;
    virtual uint32_t getPos() override;
    bool SetReconnect(int tries, int delayms) { reconnectTries = tries; reconnectDelayMs = delayms; return true; }

    enum { STATUS_HTTPFAIL=2, STATUS_DISCONNECTED, STATUS_RECONNECTING, STATUS_RECONNECTED, STATUS_NODATA };

    void textToSpeech(const char * text, int speaker);

  private:
    void httpInit();
    int postAudioQuery(const char * text, int speaker);
    int postSynthesis(int speaker);

  private:
    virtual uint32_t readInternal(void *data, uint32_t len, bool nonBlock);
    HTTPClient http;

    int pos;
    int size;
    int reconnectTries;
    int reconnectDelayMs;

    String endpoint = "";
    String audioQueryStr = "";
};


#endif

