# Distributed Parking Assist System with CAN

Two-ECU parking assist system: a sensor node measures the distance to an obstacle and sends it
over CAN at 500 kbit/s to an HMI node, which displays it and warns the driver.

## Status (work in progress)
- [x] Sensor node: CAN bring-up in loopback at 500 kbit/s, interrupt-driven receive, SWO debug output
- [x] Sensor node: ultrasonic distance measurement (HC-SR04, TIM2 PWM trigger and input capture)
- [x] CAN message `PA_Distance` (ID 0x200) with status and alive counter, specified in a DBC file
- [x] HMI node: CAN bring-up in loopback, receives and decodes `PA_Distance`
- [ ] Two-node bus over SN65HVD230 transceivers
- [ ] HMI node: 1602 LCD and buzzer warning
- [ ] FreeRTOS application structure
- [ ] Diagnostics (UDS over ISO-TP) and bus-off recovery

## System overview
| Node | Board | MCU | Role |
|---|---|---|---|
| Sensor node | STM32F407G-DISC1 | STM32F407VG (168 MHz) | Measures distance, sends `PA_Distance` every 100 ms |
| HMI node | NUCLEO-F103RB | STM32F103RB (64 MHz) | Receives `PA_Distance`, shows and signals it |

Using two different microcontroller families on one bus mirrors real vehicles, where ECUs come
from different suppliers.

## CAN communication
The full message set is defined in [`can_parking_assist.dbc`](can_parking_assist.dbc).

**PA_Distance** (ID 0x200, 4 bytes, cycle 100 ms, sender: sensor node)
| Byte | Signal | Description |
|---|---|---|
| 0 to 1 | Distance | Distance in mm, unsigned 16-bit, little-endian |
| 2 | Status | 0 = valid, 1 = no echo or out of range |
| 3 | AliveCounter | Increments with every frame, lets the receiver detect a frozen sender |

**Bit timing (500 kbit/s on both nodes)**
| Node | CAN clock | Prescaler | Segment 1 | Segment 2 | Sample point |
|---|---|---|---|---|---|
| Sensor node | 42 MHz | 6 | 11 tq | 2 tq | 85.7 % |
| HMI node | 32 MHz | 4 | 13 tq | 2 tq | 87.5 % |

## Pin assignment
**Sensor node (STM32F407)**
| Signal | Pin | Function |
|---|---|---|
| CAN1 RX | PD0 | CAN receive (internal pull-up) |
| CAN1 TX | PD1 | CAN transmit |
| Trigger | PA2 | TIM2_CH3, 10 µs pulse every 60 ms |
| Echo | PA1 | TIM2_CH2, input capture on both edges (via 1 kOhm / 2 kOhm divider) |
| Debug output | PB3 | SWO (printf via ITM) |

**HMI node (STM32F103)**
| Signal | Pin | Function |
|---|---|---|
| CAN RX | PA11 | CAN receive (internal pull-up) |
| CAN TX | PA12 | CAN transmit |
| LCD RS, E | PA0, PA1 | 1602 LCD control (4-bit mode) |
| LCD D4 to D7 | PA4, PB0, PC1, PC0 | 1602 LCD data |
| Debug output | PB3 | SWO (printf via ITM) |

## Repository structure
| Path | Content |
|---|---|
| `sensor_node/` | STM32CubeIDE project for the sensor node |
| `hmi_node/` | STM32CubeIDE project for the HMI node |
| `can_parking_assist.dbc` | CAN message specification |

## Tools
STM32CubeMX, STM32CubeIDE, STM32 HAL, Git

## Debugging notes
- **HAL_CAN_Start timeout (error 0x20000) in loopback mode:** the CAN RX pin (PD0) was floating
  with no transceiver attached, so the controller could not detect an idle bus and stayed in
  initialisation mode. Fixed by enabling the internal pull-up on PD0. The same pull-up was applied
  to PA11 on the HMI node from the start.
