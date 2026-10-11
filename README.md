# Distributed Parking Assist System with CAN

Two-ECU parking assist system on STM32F407 (sensor node and HMI node)
communicating over CAN at 500 kbit/s.

## Status (work in progress)
- [x] CAN controller bring-up, loopback at 500 kbit/s with interrupt-driven receive
- [ ] Ultrasonic distance measurement (timer input capture)
- [ ] Two-node bus with DBC-defined message set
- [ ] FreeRTOS application, buzzer and display on HMI node
- [ ] Diagnostics (UDS over ISO-TP) and bus-off recovery

## Tools
STM32CubeMX, STM32CubeIDE, STM32 HAL
