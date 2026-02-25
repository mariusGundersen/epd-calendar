#include <HTTPClient.h>
#include <ArduinoJson.h>
#include <NetworkClientSecure.h>
#include "Yr.h"

const char *yrRootCACert = "-----BEGIN CERTIFICATE-----\n"
                           "MIIFpDCCA4ygAwIBAgIQOcqTHO9D88aOk8f0ZIk4fjANBgkqhkiG9w0BAQsFADBs\n"
                           "MQswCQYDVQQGEwJHUjE3MDUGA1UECgwuSGVsbGVuaWMgQWNhZGVtaWMgYW5kIFJl\n"
                           "c2VhcmNoIEluc3RpdHV0aW9ucyBDQTEkMCIGA1UEAwwbSEFSSUNBIFRMUyBSU0Eg\n"
                           "Um9vdCBDQSAyMDIxMB4XDTIxMDIxOTEwNTUzOFoXDTQ1MDIxMzEwNTUzN1owbDEL\n"
                           "MAkGA1UEBhMCR1IxNzA1BgNVBAoMLkhlbGxlbmljIEFjYWRlbWljIGFuZCBSZXNl\n"
                           "YXJjaCBJbnN0aXR1dGlvbnMgQ0ExJDAiBgNVBAMMG0hBUklDQSBUTFMgUlNBIFJv\n"
                           "b3QgQ0EgMjAyMTCCAiIwDQYJKoZIhvcNAQEBBQADggIPADCCAgoCggIBAIvC569l\n"
                           "mwVnlskNJLnQDmT8zuIkGCyEf3dRywQRNrhe7Wlxp57kJQmXZ8FHws+RFjZiPTgE\n"
                           "4VGC/6zStGndLuwRo0Xua2s7TL+MjaQenRG56Tj5eg4MmOIjHdFOY9TnuEFE+2uv\n"
                           "a9of08WRiFukiZLRgeaMOVig1mlDqa2YUlhu2wr7a89o+uOkXjpFc5gH6l8Cct4M\n"
                           "pbOfrqkdtx2z/IpZ525yZa31MJQjB/OCFks1mJxTuy/K5FrZx40d/JiZ+yykgmvw\n"
                           "Kh+OC19xXFyuQnspiYHLA6OZyoieC0AJQTPb5lh6/a6ZcMBaD9YThnEvdmn8kN3b\n"
                           "LW7R8pv1GmuebxWMevBLKKAiOIAkbDakO/IwkfN4E8/BPzWr8R0RI7VDIp4BkrcY\n"
                           "AuUR0YLbFQDMYTfBKnya4dC6s1BG7oKsnTH4+yPiAwBIcKMJJnkVU2DzOFytOOqB\n"
                           "AGMUuTNe3QvboEUHGjMJ+E20pwKmafTCWQWIZYVWrkvL4N48fS0ayOn7H6NhStYq\n"
                           "E613TBoYm5EPWNgGVMWX+Ko/IIqmhaZ39qb8HOLubpQzKoNQhArlT4b4UEV4AIHr\n"
                           "W2jjJo3Me1xR9BQsQL4aYB16cmEdH2MtiKrOokWQCPxrvrNQKlr9qEgYRtaQQJKQ\n"
                           "CoReaDH46+0N0x3GfZkYVVYnZS6NRcUk7M7jAgMBAAGjQjBAMA8GA1UdEwEB/wQF\n"
                           "MAMBAf8wHQYDVR0OBBYEFApII6ZgpJIKM+qTW8VX6iVNvRLuMA4GA1UdDwEB/wQE\n"
                           "AwIBhjANBgkqhkiG9w0BAQsFAAOCAgEAPpBIqm5iFSVmewzVjIuJndftTgfvnNAU\n"
                           "X15QvWiWkKQUEapobQk1OUAJ2vQJLDSle1mESSmXdMgHHkdt8s4cUCbjnj1AUz/3\n"
                           "f5Z2EMVGpdAgS1D0NTsY9FVqQRtHBmg8uwkIYtlfVUKqrFOFrJVWNlar5AWMxaja\n"
                           "H6NpvVMPxP/cyuN+8kyIhkdGGvMA9YCRotxDQpSbIPDRzbLrLFPCU3hKTwSUQZqP\n"
                           "JzLB5UkZv/HywouoCjkxKLR9YjYsTewfM7Z+d21+UPCfDtcRj88YxeMn/ibvBZ3P\n"
                           "zzfF0HvaO7AWhAw6k9a+F9sPPg4ZeAnHqQJyIkv3N3a6dcSFA1pj1bF1BcK5vZSt\n"
                           "jBWZp5N99sXzqnTPBIWUmAD04vnKJGW/4GKvyMX6ssmeVkjaef2WdhW+o45WxLM0\n"
                           "/L5H9MG0qPzVMIho7suuyWPEdr6sOBjhXlzPrjoiUevRi7PzKzMHVIf6tLITe7pT\n"
                           "BGIBnfHAT+7hOtSLIBD6Alfm78ELt5BGnBkpjNxvoEppaZS3JGWg/6w/zgH7IS79\n"
                           "aPib8qXPMThcFarmlwDB31qlpzmq6YR/PFGoOtmUW4y/Twhx5duoXNTSpv4Ao8YW\n"
                           "xw/ogM4cKGR0GQjTQuPOAF1/sdwTsOEFy9EgqoZ0njnnkf3/W9b3raYvAwtt41dU\n"
                           "63ZTGI0RmLo=\n"
                           "-----END CERTIFICATE-----";

const String url = "https://api.met.no/weatherapi/locationforecast/2.0/compact?lat=59.91757&lon=10.82044";

class HttpStream : public Stream
{
private:
    HTTPClient &_client;
    Stream &_stream;

public:
    HttpStream(HTTPClient &client) : _client(client), _stream(client.getStream()) {}

    int available() override
    {
        int available = _stream.available();
        if (available)
        {
            return available;
        }

        while (_client.connected())
        {
            available = _stream.available();
            if (available)
            {
                return available;
            }
        }
        return available;
    }

    int read() override
    {
        int read = _stream.read();
        while (read == -1 && _client.connected())
        {
            read = _stream.read();
        }
        return read;
    }

    int peek() override
    {
        return _stream.peek();
    }

    size_t write(uint8_t data) override
    {
        return _stream.write(data);
    }
};

void getWeather(std::vector<Day> &days)
{
    NetworkClientSecure client;
    client.setCACert(yrRootCACert);

    HTTPClient http;
    http.useHTTP10(true);
    http.begin(client, url);
    http.GET();
    JsonDocument filter;

    filter["time"] = true;
    filter["data"]["instant"]["details"]["air_temperature"] = true;
    filter["data"]["next_12_hours"]["summary"]["symbol_code"] = true;

    JsonDocument doc;

    HttpStream responseStream(http);

    responseStream.find("\"timeseries\":[");

    Day currentDayData;

    do
    {
        DeserializationError error = deserializeJson(doc, responseStream, DeserializationOption::Filter(filter));

        if (error)
        {
            Serial.print("deserializeJson() failed: ");
            Serial.println(error.c_str());
            return;
        }

        String time = doc["time"].as<String>();
        String symbol_code = doc["data"]["next_12_hours"]["summary"]["symbol_code"].as<String>();
        float temp = doc["data"]["instant"]["details"]["air_temperature"].as<float>();

        if (time.substring(0, 10) != currentDayData.date)
        {
            if (currentDayData.date != "")
            {
                days.push_back(currentDayData);
            }
            String currentDay = time.substring(0, 10);
            currentDayData = Day{currentDay, 99, -99, symbol_code};
        }

        if (temp < currentDayData.minTemp)
        {
            currentDayData.minTemp = temp;
        }
        if (temp > currentDayData.maxTemp)
        {
            currentDayData.maxTemp = temp;
        }
    } while (responseStream.findUntil(",", "]"));

    // Consume remaining stream data to ensure connection is properly closed
    while (responseStream.available())
    {
        responseStream.read();
    }

    for (const auto &day : days)
    {
        Serial.printf("%s: %f - %f - %s\n", day.date.c_str(), day.minTemp, day.maxTemp, day.symbol_code.c_str());
    }

    http.end();
}