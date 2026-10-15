# Distributed Parking Assist System with CAN

Two-ECU parking assist system: a sensor node measures the distance to an obstacle and sends it
over a CAN bus at 500 kbit/s to an HMI node, which shows it on a display.

## Status (work in progress)
- [x] Sensor node: CAN bring-up in loopback at 500 kbit/s, interrupt-driven receive, SWO debug output
- [x] Sensor node: ultrasonic distance measurement (HC-SR04, TIM2 PWM trigger and input capture)
- [x] CAN message `PA_Distance` (ID 0x200) with status and alive counter, specified in a DBC file
- [x] HMI node: CAN bring-up, receives and decodes `PA_Distance`
- [x] HMI node: own HD44780 driver for a 1602 LCD in 4-bit mode, shows the received distance
- [x] Two-node bus over SN65HVD230 transceivers: live distance from the sensor node shown on the HMI's LCD
- [x] Bus diagnostics on the sensor node: CAN error status register shown on LEDs and debug console
- [ ] Bus timing and load measured with a logic analyser
- [ ] HMI node: buzzer and LED warning by distance zone, timeout when messages stop
- [ ] FreeRTOS application structure
- [ ] Diagnostics (UDS over ISO-TP) and bus-off recovery

## System overview
| Node | Board | MCU | Role |
|---|---|---|---|
| Sensor node | STM32F407G-DISC1 | STM32F407VG (168 MHz) | Measures distance, sends `PA_Distance` every 100 ms |
| HMI node | NUCLEO-F103RB | STM32F103RB (64 MHz) | Receives `PA_Distance`, shows it on a 1602 LCD |

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

Both nodes run in normal mode with automatic retransmission enabled.

## Hardware and wiring

**Transceivers (one SN65HVD230 module per node, powered from 3.3 V)**
| Module pin | Sensor node (STM32F407) | HMI node (STM32F103) |
|---|---|---|
| 3.3V | 3V pin | 3V3 (CN6, socket 4) |
| GND | GND | GND |
| CAN TX | PD1 | PA12 (CN10 outer row, level with D13) |
| CAN RX | PD0 | PA11 (CN10 outer row, level with D12) |

CAN TX on the module goes to CAN TX on the MCU, and RX to RX; a CAN transceiver does not cross
them the way a UART connection does.

**Bus**
| Connection | Purpose |
|---|---|
| CANH to CANH, CANL to CANL, as a twisted pair | differential bus signal |
| GND of both boards connected | common reference for both transceivers |
| 120 Ω termination on each module | two terminations in parallel give 60 Ω between CANH and CANL |

The 5 V supplies of the two boards are never connected; each board is powered from its own USB port.

**Sensor node (STM32F407)**
| Signal | Pin | Function |
|---|---|---|
| CAN1 RX / TX | PD0 / PD1 | CAN (internal pull-up on RX) |
| Trigger | PA2 | TIM2_CH3, 10 µs pulse every 60 ms |
| Echo | PA1 | TIM2_CH2, input capture on both edges (via 1 kOhm / 2 kOhm divider from 5 V to 3.3 V) |
| Diagnostic LEDs | PD12, PD14, PD15 | green = bus OK, red = transmit errors, blue = no acknowledgement |
| Debug output | PB3 | SWO (printf via ITM) |

**HMI node (STM32F103)**
| Signal | Pin | Function |
|---|---|---|
| CAN RX / TX | PA11 / PA12 | CAN (internal pull-up on RX) |
| LCD RS, E | PA0, PA1 | 1602 LCD control (4-bit mode, RW tied to GND) |
| LCD D4 to D7 | PA4, PB0, PC1, PC0 | 1602 LCD data |
| Receive indicator | PA5 (LD2) | toggles with every received `PA_Distance` frame |
| Debug output | PB3 | SWO (printf via ITM) |

LCD supply: 5 V from the Nucleo, contrast set with 1 kOhm from V0 to GND (no potentiometer needed),
backlight through 330 Ω.

## Software components
- **Distance measurement:** TIM2 runs at 1 µs per tick with a 60 ms period. Channel 3 generates the
  trigger pulse in hardware; channel 2 captures both edges of the echo, and the pulse width gives
  the distance (mm = µs × 343 / 2000).
- **CAN message handling:** the sensor node packs distance, status, and alive counter into
  `PA_Distance`; the HMI node checks ID and length in the receive interrupt and decodes the signals.
- **LCD driver:** `hmi_node/Core/Src/lcd1602.c` implements the HD44780 initialisation sequence for
  4-bit mode, with microsecond timing from the Cortex-M3 cycle counter (DWT). API: `LCD_Init`,
  `LCD_Clear`, `LCD_SetCursor`, `LCD_Print`.
- **Bus diagnostics:** the sensor node reads the CAN error status register (ESR) after each frame
  and shows the transmit error counter (TEC) and last error code (LEC) on LEDs and in the console.

## Build and run
1. Open both projects in STM32CubeIDE (`sensor_node`, `hmi_node`); configuration changes are made in
   the standalone STM32CubeMX with the `.ioc` files.
2. With both boards connected, set the ST-LINK serial number in each project's debug configuration
   (Debugger tab, ST-LINK S/N), so each project always connects to its own board.
3. Flash the sensor node and let it run, then debug the HMI node. Enable Serial Wire Viewer
   (core clock 168 MHz for the sensor node, 64 MHz for the HMI node) and ITM port 0 to see the
   debug output.

## Repository structure
| Path | Content |
|---|---|
| `sensor_node/` | STM32CubeIDE project for the sensor node |
| `hmi_node/` | STM32CubeIDE project for the HMI node, including the LCD driver |
| `can_parking_assist.dbc` | CAN message specification |

## Tools
STM32CubeMX, STM32CubeIDE, STM32 HAL, Git

## Debugging log
Problems met during development, how they were found, and how they were fixed.

### Tooling and setup
- **CubeMX project could not be opened inside the IDE:** the `.ioc` file was created with the
  standalone STM32CubeMX 6.18.1, while the IDE's built-in CubeMX was version 6.15.0. Fix: import the
  generated code as an existing project and do all configuration in the standalone CubeMX.
- **HMI project would not build (23 errors in CMSIS RTOS2 and NN files):** the first code generation
  had used the EWARM (IAR) toolchain setting, which copied extra CMSIS libraries into `Drivers`.
  Fix: set the toolchain to STM32CubeIDE, delete the `Drivers` folder, and regenerate with
  "copy only the necessary library files".
- **ST-LINK not detected on the Nucleo, then a firmware prompt:** after reconnecting, the IDE offered
  an ST-LINK firmware update, which was applied (V2J46M33).
- **"Multiple ST-LINKs detected":** with both boards connected, the IDE did not know which board to
  debug. Fix: ST-LINK serial number selected in each project's debug configuration.
- **SWO console stayed empty:** ITM stimulus port 0 was not enabled, and the trace must be started
  before the program runs. Fix: tick port 0 in the trace configuration, start the trace, then resume.

### Sensor node
- **LED did not blink after flashing:** the debugger halts at `main()` by default, so the program had
  not started yet. Fix: resume execution (F8) after the debug session starts.
- **HAL_CAN_Start timeout (error 0x20000) in loopback mode:** found by reading `hcan1.ErrorCode` in the
  debugger after the program stopped in `Error_Handler`. Cause: the CAN RX pin (PD0) was floating with
  no transceiver attached, so the controller could not detect an idle bus and stayed in
  initialisation mode. Fix: internal pull-up on PD0; applied to PA11 on the HMI node from the start.
- **Sensor reported "no echo" after rewiring:** the status signal (`st=1`) and the alive counter showed
  that the CAN path worked while the measurement failed. Cause: an interrupted ground rail on the
  breadboard. Fix: sensor ground connected directly to a GND pin of the board.

### HMI node
- **Compiler warning on `snprintf` truncation:** the distance value is now limited to 4 digits before
  formatting, so the LCD line always fits its 16 characters.
- **Display contrast without a potentiometer:** V0 connected through 1 kOhm to GND; leaving V0
  unconnected would give a blank display even with working code.

###
