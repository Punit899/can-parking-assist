# Distributed Parking Assist System with CAN

Two-ECU parking assist system on STM32F407 (sensor node and HMI node)
communicating over CAN at 500 kbit/s.

## Status (work in progress)
- [x] CAN controller bring-up, loopback at 500 kbit/s with interrupt-driven receive and SWO debug output
- [ ] Ultrasonic distance measurement (timer input capture)
- [ ] Two-node bus with DBC-defined message set
- [ ] FreeRTOS application, buzzer and display on HMI node
- [ ] Diagnostics (UDS over ISO-TP) and bus-off recovery

## Tools
STM32CubeMX, STM32CubeIDE, STM32 HAL

## Debugging notes
- **HAL_CAN_Start timeout (error 0x20000) in loopback mode:** the CAN RX pin (PD0) was floating
  with no transceiver attached, so the controller could not detect an idle bus and stayed in
  initialisation mode. Fixed by enabling the internal pull-up on PD0.
