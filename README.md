# Water Level Controller
A project to control tank water level using HC-SR04 and ESP8266
Functions:
- Read water level of tank using HC-SR04 module
- Export level reading and telemetry to InfluxDB Cloud for remote monitoring
- Control a pump according to water availability and tank water level
- Support for remote control commands
- The monitoring components are run by solar power; Voltage Monitoring using Aduino ADC

## Hardware Used
- HC-SR04 for reading water level from tank
- NodeMCU ESP8266 Module for Wifi Connectivity and control logic
- SLA-05VDC relay SSR for pump control

## Softwares Used
- IDE:Arduino
- Cloud Storage: Influx Cloud
- Visualization: Grafana Cloud


