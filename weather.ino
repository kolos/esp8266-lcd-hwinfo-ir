#define WEATHER_ID "icon\":"
#define WEATHER_TEMP "feelsLike\":"
#define WEATHER_WIND_SPEED "windSpeed\":"
#define WEATHER_WIND_DIR "indDirDegrees\":"
#define WEATHER_PHRASE "phrase\":\""
#define WATHER_NO_PRECIPITATION "characteristic\":[0]"
#define WEATHER_HUMIDITY "humidity\":"
#define WEATHER_DEWPOINT "dewPoint\":"
#define WEATHER_BAROMETERTREND "barometerTrend\":\""

#define WATHER_RAIN_START "startTime\":["
#define WATHER_RAIN_END "endTime\":["
#define WATHER_RAIN_AMOUNT "forecastedRainAmount\":["

#define FIX_ACCENTS(a) a.replace("é", eacute); \
      a.replace("ő", odoubleacute); \
      a.replace("ó", oacute); \
      a.replace("ö", ouml); \
      a.replace("ü", uuml); \
      a.replace("ű", udoubleacute); \
      a.replace("ú", uacute); \
      a.replace("á", aacute); \
      a.replace("í", iacute); \
      \
      a.replace("É", eacute); \
      a.replace("Ő", odoubleacute); \
      a.replace("Ó", oacute); \
      a.replace("Ö", ouml); \
      a.replace("Ü", uuml); \
      a.replace("Ű", udoubleacute); \
      a.replace("Ú", uacute); \
      a.replace("Á", aacute); \
      a.replace("Í", iacute);

int findJsonTerminalCharFrom(String json, int startIndex) {
  for(int i = startIndex; i < json.length(); i++) {
    if(json.charAt(i) == ','
      || json.charAt(i) == '}'
      || json.charAt(i) == '\"'
      || json.charAt(i) == ']')
      return i;
  }

  return -1;
}

String match_json(String json, const char* pattern) {

  return json.substring(strlen(pattern) + json.indexOf(pattern), findJsonTerminalCharFrom(json, strlen(pattern) + json.indexOf(pattern)));
}

int match_json_int(String json, const char* pattern) {
  return match_json(json, pattern).toInt();
}

struct tm match_json_time(String json, const char* pattern) {
  return convert_string_to_tm(match_json(json, pattern));
}

struct tm convert_string_to_tm(String string) {
  struct tm _tm = { 0 };
  int zh, zm;
  sscanf(string.c_str(), "%4d-%2d-%2dT%2d:%2d:%2d+%2d%2d", &(_tm.tm_year), &(_tm.tm_mon), &(_tm.tm_mday), &(_tm.tm_hour), &(_tm.tm_min), &(_tm.tm_sec), &zh, &zm);
  _tm.tm_year -= 1900;
  _tm.tm_mon -= 1;
  _tm.tm_isdst = 0;

  return _tm;
}

void onClientStateChange(void * arguments, asyncHTTPrequest * req, int readyState) {
  if(readyState == 4 && req->responseHTTPcode() == 200) {
      String response = req->responseText();

      _weather_id = match_json(response, WEATHER_ID).toInt();
      _weather_temp = match_json(response, WEATHER_TEMP).toInt();
      _weather_wind_speed = match_json(response, WEATHER_WIND_SPEED).toInt();
      _weather_wind_dir = match_json(response, WEATHER_WIND_DIR).toInt();
      _weather_humidity = match_json(response, WEATHER_HUMIDITY).toInt();
      _weather_phrase = match_json(response, WEATHER_PHRASE);
      _weather_dewpoint = match_json(response, WEATHER_DEWPOINT).toInt();
      _weather_barometertrend = match_json(response, WEATHER_BAROMETERTREND);

      FIX_ACCENTS(_weather_phrase);
      FIX_ACCENTS(_weather_barometertrend);

      if(_weather_phrase.length() > 6) {
        byte space_char_pos = _weather_phrase.lastIndexOf(" ") + 1;
        _weather_phrase = _weather_phrase.substring(space_char_pos, space_char_pos + 6);
      }

      if(response.indexOf(WATHER_NO_PRECIPITATION) == -1) {
        _weather_has_precipitation = true;

        int rainStartArrStart = response.indexOf(WATHER_RAIN_START) + strlen(WATHER_RAIN_START);
        int rainStartArrEnd = response.indexOf("]", rainStartArrStart);
        byte arrLen = ( rainStartArrEnd - rainStartArrStart + 1 ) / 27;

        int rainEndArrStart = response.indexOf(WATHER_RAIN_END) + strlen(WATHER_RAIN_END);

        int rainAmountIdx = response.indexOf(WATHER_RAIN_AMOUNT) + strlen(WATHER_RAIN_AMOUNT);

        tm startArr[arrLen];
        tm endArr[arrLen];
        float amounts[arrLen];
        
        for(byte i = 0; i < arrLen; i++) {
          startArr[i] = convert_string_to_tm(response.substring(rainStartArrStart + (i * 27) + 1, rainStartArrStart + (i * 27) + 26 - 1 - 1));
          endArr[i] = convert_string_to_tm(response.substring(rainEndArrStart + (i * 27) + 1, rainEndArrStart + (i * 27) + 26 - 1 - 1));

          int floatEnd = findJsonTerminalCharFrom(response, rainAmountIdx);
          amounts[i] = response.substring(rainAmountIdx, floatEnd).toFloat();
          rainAmountIdx = floatEnd + 1;
        }

        _weather_amount_of_rain = 0; _next_rain_idx = 0;
        for(byte i = 0; i < arrLen; i++) {
          if(_weather_amount_of_rain > 0) continue;

          _weather_amount_of_rain = amounts[i];
          _next_rain_idx = i;
        }

        if(_next_rain_idx == 0) { /* its currently raining => should display the end time */
          _weather_rain_time = endArr[_next_rain_idx];
        } else {
          _weather_rain_time = startArr[_next_rain_idx];
        }
      } else {
        _weather_has_precipitation = false;
      }
  }
}


void setupAsyncWeather() {
  httpRequest.setTimeout(5);
  httpRequest.setDebug(false);
  httpRequest.onReadyStateChange(onClientStateChange);
}

void requestWeather() {
  httpRequest.open("GET", WEATHER_API_URL);
  httpRequest.send();
}
