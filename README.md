# \# Distributed Parking Assist System with CAN

# 

# Two-ECU parking assist system on STM32F407 (sensor node and HMI node)

# communicating over CAN at 500 kbit/s.

# 

# \## Status (work in progress)

# \- \[x] CAN controller bring-up, loopback at 500 kbit/s with interrupt-driven receive and SWO debug output

# \- \[x] Ultrasonic distance measurement (HC-SR04, TIM2 PWM trigger and input capture)

# \- \[ ] Two-node bus with DBC-defined message set

# \- \[ ] FreeRTOS application, buzzer and display on HMI node

# \- \[ ] Diagnostics (UDS over ISO-TP) and bus-off recovery

# 

# \## Hardware

# \- STM32F407G-DISC1 as sensor node

# \- HC-SR04 ultrasonic sensor, echo signal reduced to 3.3 V with a 1 kOhm / 2 kOhm voltage divider

# 

# \## Pin assignment (sensor node)

# | Signal | Pin | Function |

# |---|---|---|

# | CAN1 RX | PD0 | CAN receive (internal pull-up) |

# | CAN1 TX | PD1 | CAN transmit |

# | Trigger | PA2 | TIM2\_CH3, 10 µs pulse every 60 ms |

# | Echo | PA1 | TIM2\_CH2, input capture on both edges |

# | Debug output | PB3 | SWO (printf via ITM) |

# 

# \## Tools

# STM32CubeMX, STM32CubeIDE, STM32 HAL

# 

# \## Debugging notes

# \- \*\*HAL\_CAN\_Start timeout (error 0x20000) in loopback mode:\*\* the CAN RX pin (PD0) was floating

# &#x20; with no transceiver attached, so the controller could not detect an idle bus and stayed in

# &#x20; initialisation mode. Fixed by enabling the internal pull-up on PD0.

