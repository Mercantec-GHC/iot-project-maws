#ifndef CARRIER_H
#define CARRIER_H
#include <Arduino_MKRIoTCarrier.h>
#include <WiFiNINA.h>
#include <ArduinoHttpClient.h>

MKRIoTCarrier carrier;
int center_x;
int center_y;

// WiFi Configuration
const char* ssid = "NSFW-Wifi";
const char* password = "Keshava108";
const char* server_address = "192.168.64.105"; // local ip
const int server_port = 5021;
WiFiClient wifi_client;
HttpClient http_client(wifi_client, server_address, server_port);

#endif