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
int last_hour = 0;
struct tm ntp_time;

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
      Serial1.print(Item::name);
      Serial1.print(F(": "));
      Serial1.print(i.val());
      Serial1.print(Item::unit());
      Serial1.println();
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
        updateChart(i.val(), energy_delivered_tariff1_chart, energy_delivered_tariff1_axis, energy_delivered_tariff1_last);
        energy_delivered_tariff1_card.update(i.val());
      }
      if((String)Item::name == "energy_delivered_tariff2") {
        updateChart(i.val(), energy_delivered_tariff2_chart, energy_delivered_tariff2_axis, energy_delivered_tariff2_last);
        energy_delivered_tariff2_card.update(i.val());
      }
      if((String)Item::name == "energy_returned_tariff1") {
        updateChart(i.val(), energy_returned_tariff1_chart, energy_returned_tariff1_axis, energy_returned_tariff1_last);
        energy_returned_tariff1_card.update(i.val());
      }
      if((String)Item::name == "energy_returned_tariff2") {
        updateChart(i.val(), energy_returned_tariff2_chart, energy_returned_tariff2_axis, energy_returned_tariff2_last);
        energy_returned_tariff2_card.update(i.val());
      }
      if((String)Item::name == "power_delivered")
        power_delivered_card.update(i.val());
      if((String)Item::name == "power_returned")
        power_returned_card.update(i.val());
      if((String)Item::name == "gas_delivered") {
        updateChart(i.val(), gas_delivered_chart, gas_delivered_axis, gas_delivered_last);
        gas_delivered_card.update(i.val());
      }
    }
  }
};

void setup() {
  Serial.begin(115200, SerialConfig::SERIAL_8N1, SerialMode::SERIAL_RX_ONLY, -1, true);
  Serial.swap();

  Serial1.println("Setup...");

  Serial1.print("WiFi Connecting...");
  WiFi.setHostname("SmartMeter");
  WiFi.mode(WIFI_STA);
  WiFi.config(ipaddr, gateway, subnet, dns1, dns2);
  WiFi.begin(ssid, pass);
  while (WiFi.waitForConnectResult() != WL_CONNECTED) {
    Serial1.print('.');
    delay(1000);
  }
  Serial1.println("Done");

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
      last_hour = ntp_time.tm_hour;
    } else {
      Serial1.println(err);
    }
  }

  unsigned long now = millis();
  if (now - last > 1000) {
    ntp_time.tm_year = 0;
    if(getLocalTime(&ntp_time, 5000)) {
      Serial1.printf("\nNow is : %d-%02d-%02d %02d:%02d:%02d\n", (ntp_time.tm_year) + 1900, (ntp_time.tm_mon) + 1, ntp_time.tm_mday, ntp_time.tm_hour, ntp_time.tm_min, ntp_time.tm_sec);
      p1reader.enable(true);
    }
    last = now;
  }
}

void updateChart(auto value, Chart &chart, auto *axis, auto &last) {
  if (last_hour != ntp_time.tm_hour) axis[ntp_time.tm_hour] = 0;
  if (last == 0) last = value;
  axis[ntp_time.tm_hour] += value - last;
  last = value;
  chart.updateX(time_axis, ntp_time.tm_hour+1);
  chart.updateY(axis, ntp_time.tm_hour+1);
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