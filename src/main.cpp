#include <Arduino.h>
#include <WiFiMulti.h>
#include <InfluxDbClient.h>
#include <InfluxDbCloud.h>

WiFiMulti wifiMulti;

// Configuración WiFi
#define WIFI_SSID "Orozco 2.4G"
#define WIFI_PASSWORD "orozco02"

// Configuración InfluxDB
#define INFLUXDB_URL "https://us-east-1-1.aws.cloud2.influxdata.com"
#define INFLUXDB_TOKEN "s9UNwU8G1UyEyMfcdrD0hOz9-gRIzMMcM3480IaLkIJme5RsjrmFoF9ghjD5_zZCfl0FO1hqx877dlKmPqiGXA=="
#define INFLUXDB_ORG "6bbc90e6b769e4fb"
#define INFLUXDB_BUCKET "Sensores"

#define TZ_INFO "UTC-5"
#define INFLUXDB_SEND_INTERVAL 5000  // Enviar datos cada 5 segundos

InfluxDBClient client(INFLUXDB_URL, INFLUXDB_ORG, INFLUXDB_BUCKET, INFLUXDB_TOKEN, InfluxDbCloud2CACert);
Point sensor("Sensor_data");

static void Wifi_setup(void);
static void influxDB_TaskInit(void);
static void influxDB_TaskMng(void);

int temp1, hum1, temp2, hum2,hum_suelo1, hum_suelo2;
float clorofila, vel_viento, flujo_agua, cantidad_agua;
char dir_viento[3] = "NE";
float latitud = 10.388738361119273;
float longitud = -73.19838900029197;
static uint32_t lastSendTime = 0;

void setup() {
    Serial.begin(9600);
    Wifi_setup();
    influxDB_TaskInit();
}

void loop() {
    temp1 = random(20, 25);
    hum1 = random(50, 60);
    temp2 = random(25, 30);
    hum2 = random(60, 70);

    hum_suelo1 = random(30, 40);
    hum_suelo2 = random(40, 50);    
    
    clorofila = random(600, 675);
    vel_viento = random(10, 16);
    flujo_agua = random(7, 14);
    cantidad_agua = random(100, 200);

    influxDB_TaskMng();
}

static void influxDB_TaskInit(void) {
    timeSync(TZ_INFO, "pool.ntp.org", "time.nist.gov");
    
    // *Agregar un tag único basado en la dirección MAC del ESP32*
    uint8_t mac[6];
    WiFi.macAddress(mac);
    char macStr[18];
    snprintf(macStr, sizeof(macStr), "%02X:%02X:%02X:%02X:%02X:%02X", mac[0], mac[1], mac[2], mac[3], mac[4], mac[5]);
    
    sensor.addTag("device_id", macStr);
    
    if (client.validateConnection()) {
        Serial.println("InfluxDB connection validated");
    } else {
        Serial.println("InfluxDB connection failed");
    }
}

static void influxDB_TaskMng(void) {
    uint32_t now = millis();
    
    if (now - lastSendTime >= INFLUXDB_SEND_INTERVAL) {
        lastSendTime = now;  // Actualizar tiempo de último envío

        sensor.clearFields();
        sensor.addField("temperature1", temp1);
        sensor.addField("humidity1", hum1);
        sensor.addField("temperature2", temp2);
        sensor.addField("humidity2", hum2);
        sensor.addField("soil_humidity1", hum_suelo1);
        sensor.addField("soil_humidity2", hum_suelo2);
        sensor.addField("chlorophyll", clorofila);
        sensor.addField("wind_speed", vel_viento);
        sensor.addField("wind_direction", dir_viento);
        sensor.addField("water_flow", flujo_agua);
        sensor.addField("water_amount", cantidad_agua);
        sensor.addField("latitude", latitud);
        sensor.addField("longitude", longitud);

        Serial.print("Writing: ");
        Serial.println(sensor.toLineProtocol());

        if (WiFi.status() != WL_CONNECTED) {
            Serial.println("Wifi disconnected, trying to reconnect");
            Wifi_setup();
        }

        if (!client.writePoint(sensor)) {
            Serial.print("InfluxDB write failed: ");
            Serial.println(client.getLastErrorMessage());
        } else {
            Serial.println("InfluxDB write success");
        }
    }
}

static void Wifi_setup(void) {
    Serial.println("Connecting Wifi...");
    WiFi.mode(WIFI_STA);
    wifiMulti.addAP(WIFI_SSID, WIFI_PASSWORD);
    while (wifiMulti.run() != WL_CONNECTED) {
        Serial.print(".");
        delay(150);
    }
    Serial.println("WiFi connected");
}