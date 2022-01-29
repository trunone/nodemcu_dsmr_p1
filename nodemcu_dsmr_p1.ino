#include <ESP8266WiFi.h>
#include <time.h>

#include "dsmr.h"

#include "ESPDash.h"

#ifndef STASSID
#define STASSID ""
#define STAPSK  ""
#endif

const char *ssid = STASSID;
const char *pass = STAPSK;

unsigned long last = 0;
struct tm tmstruct;

IPAddress ipaddr(192, 168, 100, 19);
IPAddress subnet(255, 255, 255, 0);
IPAddress gateway(192, 168, 100, 1);
IPAddress dns1(8, 8, 8, 8);
IPAddress dns2(9, 9, 9, 9);

P1Reader p1reader(&Serial, D6);

AsyncWebServer webserver(80);
ESPDash dashboard(&webserver);

Card energy_delivered_tariff1_card(&dashboard, GENERIC_CARD, "energy_delivered_tariff1", "kWh");
Card energy_delivered_tariff2_card(&dashboard, GENERIC_CARD, "energy_delivered_tariff2", "kWh");
Card energy_returned_tariff1_card(&dashboard, GENERIC_CARD, "energy_returned_tariff1", "kWh");
Card energy_returned_tariff2_card(&dashboard, GENERIC_CARD, "energy_returned_tariff2", "kWh");
Card power_delivered_card(&dashboard, GENERIC_CARD, "power_delivered", "kW");
Card power_returned_card(&dashboard, GENERIC_CARD, "power_returned", "kW");
Card gas_delivered_card(&dashboard, GENERIC_CARD, "gas_delivered", "m^3");

Chart energy_delivered_tariff1_chart(&dashboard, BAR_CHART, "energy_delivered_tariff1");
Chart energy_delivered_tariff2_chart(&dashboard, BAR_CHART, "energy_delivered_tariff2");
Chart energy_returned_tariff1_chart(&dashboard, BAR_CHART, "energy_returned_tariff1");
Chart energy_returned_tariff2_chart(&dashboard, BAR_CHART, "energy_returned_tariff2");
Chart gas_delivered_chart(&dashboard, BAR_CHART, "gas_delivered");

String time_axis[] = {"0h", "1h", "2h", "3h", "4h", "5h", "6h", "7h", "8h", "9h", "10h", "11h", "12h", "13h", "14h", "15h", "16h", "17h", "18h", "19h", "20h", "21h", "22h", "23h"};
float energy_delivered_tariff1_axis[] = {0.0, 0.0, 0.0, 0.0, 0.0, 0.0, 0.0, 0.0, 0.0, 0.0, 0.0, 0.0, 0.0, 0.0, 0.0, 0.0, 0.0, 0.0, 0.0, 0.0, 0.0, 0.0, 0.0, 0.0,};
float energy_delivered_tariff2_axis[] = {0.0, 0.0, 0.0, 0.0, 0.0, 0.0, 0.0, 0.0, 0.0, 0.0, 0.0, 0.0, 0.0, 0.0, 0.0, 0.0, 0.0, 0.0, 0.0, 0.0, 0.0, 0.0, 0.0, 0.0,};
float energy_returned_tariff1_axis[] = {0.0, 0.0, 0.0, 0.0, 0.0, 0.0, 0.0, 0.0, 0.0, 0.0, 0.0, 0.0, 0.0, 0.0, 0.0, 0.0, 0.0, 0.0, 0.0, 0.0, 0.0, 0.0, 0.0, 0.0,};
float energy_returned_tariff2_axis[] = {0.0, 0.0, 0.0, 0.0, 0.0, 0.0, 0.0, 0.0, 0.0, 0.0, 0.0, 0.0, 0.0, 0.0, 0.0, 0.0, 0.0, 0.0, 0.0, 0.0, 0.0, 0.0, 0.0, 0.0,};
float gas_delivered_axis[] = {0.0, 0.0, 0.0, 0.0, 0.0, 0.0, 0.0, 0.0, 0.0, 0.0, 0.0, 0.0, 0.0, 0.0, 0.0, 0.0, 0.0, 0.0, 0.0, 0.0, 0.0, 0.0, 0.0, 0.0,};

using DsmrData = ParsedData<
    /* FixedValue */ energy_delivered_tariff1,
    /* FixedValue */ energy_delivered_tariff2,
    /* FixedValue */ energy_returned_tariff1,
    /* FixedValue */ energy_returned_tariff2,
    /* FixedValue */ power_delivered,
    /* FixedValue */ power_returned,
    /* TimestampedFixedValue */ gas_delivered
    >;

struct DsmrPrinter {
  template <typename Item>
  void apply(Item &i) {
    if (i.present()) {
      Serial.print(Item::name);
      Serial.print(F(": "));
      Serial.print(i.val());
      Serial.print(Item::unit());
      Serial.println();
    }
  }
};

float energy_delivered_tariff1_last = 0;
float energy_delivered_tariff2_last = 0;
float energy_returned_tariff1_last = 0;
float energy_returned_tariff2_last = 0;
float gas_delivered_last = 0;

struct DsmrDashboard {
  template <typename Item>
  void apply(Item &i) {
    if (i.present()) {
      if((String)Item::name == "energy_delivered_tariff1") {
        if (energy_delivered_tariff1_last == 0) energy_delivered_tariff1_last = i.val();
        energy_delivered_tariff1_axis[tmstruct.tm_hour] += i.val() - energy_delivered_tariff1_last;
        energy_delivered_tariff1_last = i.val();
        energy_delivered_tariff1_chart.updateX(time_axis, tmstruct.tm_hour+1);
        energy_delivered_tariff1_chart.updateY(energy_delivered_tariff1_axis, tmstruct.tm_hour+1);
        energy_delivered_tariff1_card.update(i.val());
      }
      if((String)Item::name == "energy_delivered_tariff2") {
        if (energy_delivered_tariff2_last == 0) energy_delivered_tariff2_last = i.val();
        energy_delivered_tariff2_axis[tmstruct.tm_hour] += i.val() - energy_delivered_tariff2_last;
        energy_delivered_tariff2_last = i.val();
        energy_delivered_tariff2_chart.updateX(time_axis, tmstruct.tm_hour+1);
        energy_delivered_tariff2_chart.updateY(energy_delivered_tariff2_axis, tmstruct.tm_hour+1);
        energy_delivered_tariff2_card.update(i.val());
      }
      if((String)Item::name == "energy_returned_tariff1") {
        if (energy_returned_tariff1_last == 0) energy_returned_tariff1_last = i.val();
        energy_returned_tariff1_axis[tmstruct.tm_hour] += i.val() - energy_returned_tariff1_last;
        energy_returned_tariff1_last = i.val();
        energy_returned_tariff1_chart.updateX(time_axis, tmstruct.tm_hour+1);
        energy_returned_tariff1_chart.updateY(energy_returned_tariff1_axis, tmstruct.tm_hour+1);
        energy_returned_tariff1_card.update(i.val());
      }
      if((String)Item::name == "energy_returned_tariff2") {
        if (energy_returned_tariff2_last == 0) energy_returned_tariff2_last = i.val();
        energy_returned_tariff2_axis[tmstruct.tm_hour] += i.val() - energy_returned_tariff2_last;
        energy_returned_tariff2_last = i.val();
        energy_returned_tariff2_chart.updateX(time_axis, tmstruct.tm_hour+1);
        energy_returned_tariff2_chart.updateY(energy_returned_tariff2_axis, tmstruct.tm_hour+1);
        energy_returned_tariff2_card.update(i.val());
      }
      if((String)Item::name == "power_delivered")
        power_delivered_card.update(i.val());
      if((String)Item::name == "power_returned")
        power_returned_card.update(i.val());
      if((String)Item::name == "gas_delivered") {
        if (gas_delivered_last == 0) gas_delivered_last = i.val();
        gas_delivered_axis[tmstruct.tm_hour] += i.val() - gas_delivered_last;
        gas_delivered_last = i.val();
        gas_delivered_chart.updateX(time_axis, tmstruct.tm_hour+1);
        gas_delivered_chart.updateY(gas_delivered_axis, tmstruct.tm_hour+1);
        gas_delivered_card.update(i.val());
      }
    }
  }
};

void setup() {
  Serial.begin(115200, SerialConfig::SERIAL_8N1, SerialMode::SERIAL_RX_ONLY, 0, true);
  Serial.swap();
  Serial.println("Setup...");

  Serial.print("WiFi Connecting...");
  WiFi.setHostname("Smart Meter");
  WiFi.mode(WIFI_STA);
  WiFi.config(ipaddr, gateway, subnet, dns1, dns2);
  WiFi.begin(ssid, pass);
  while (WiFi.waitForConnectResult() != WL_CONNECTED) {
    Serial.print('.');
    delay(1000);
  }
  Serial.println("Done");

  configTime(3600, 0, "time.nist.gov", "0.pool.ntp.org", "1.pool.ntp.org");

  p1reader.enable(true);

  webserver.begin();
}

void loop() {
  p1reader.loop();
  if (p1reader.available()) {
    DsmrData data;
    String err;
    if (p1reader.parse(&data, &err)) {
      data.applyEach(DsmrPrinter());
      data.applyEach(DsmrDashboard());
      dashboard.sendUpdates();
    } else {
      Serial.println(err);
    }
  }

  unsigned long now = millis();
  if (now - last > 1000) {
    tmstruct.tm_year = 0;
    if(getLocalTime(&tmstruct, 5000)) {
      Serial.printf("\nNow is : %d-%02d-%02d %02d:%02d:%02d\n", (tmstruct.tm_year) + 1900, (tmstruct.tm_mon) + 1, tmstruct.tm_mday, tmstruct.tm_hour, tmstruct.tm_min, tmstruct.tm_sec);
      p1reader.enable(true);
    }
    last = now;
  }
}

bool getLocalTime(struct tm * info, uint32_t ms) {
  uint32_t count = ms / 10;
  time_t now;

  time(&now);
  localtime_r(&now, info);

  if (info->tm_year > 100) {
    return true;
  }

  while (count--) {
    delay(10);
    time(&now);
    localtime_r(&now, info);
    if (info->tm_year > 100) {
      return true;
    }
  }
  return false;
}