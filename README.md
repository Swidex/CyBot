# CyBot

Code created for CPRE 288 at Iowa State.
##Table of Contents
- [CyBot](#cybot)
  - [CyBot Software (C)](#cybot-software-c)
    - [Information](#information)
    - [UART Information](#uart-information)
    - [Sensor Data:](#sensor-data)
    - [Commands:](#commands)
      - [Drive (w) `w[param]x`](#drive-w-wparamx)
        - [Examples](#examples)
        - [Return Format](#return-format)
      - [Turn (d) `d[param]x`](#turn-d-dparamx)
        - [Examples](#examples-1)
        - [Return Format](#return-format-1)
      - [Scan (d) `s`](#scan-d-s)
        - [Return Format](#return-format-2)
  - [GUI (Python)](#gui-python)
    - [Python Requirements:](#python-requirements)
    - [GUI Information](#gui-information)
      - [CyBot](#cybot-1)
        - [Direction](#direction)
        - [Servo/Scan Angle](#servoscan-angle)
      - [Scan Data](#scan-data)
        - [IR Data](#ir-data)
        - [Detected Objects](#detected-objects)
      - [Bot Information](#bot-information)
        - [Mode Information](#mode-information)
        - [Bump Sensor](#bump-sensor)
        - [Position Data](#position-data)
    - [GUI Commands](#gui-commands)
      - [Drive `w/s`](#drive-ws)
      - [Turn `a/d`](#turn-ad)
      - [Scan `s`](#scan-s)
      - [Calibrate `c`](#calibrate-c)
      - [Clear `k`](#clear-k)
      - [Toggle Mode `t`](#toggle-mode-t)

## CyBot Software (C)
### Information
- Tiva™TM4C123GH6PM Microcontroller
- Iowa State CyBot

### UART Information
- Baudrate is 115200
### Sensor Data:
- IR Sensor
  - ~~Additional IR Sensors~~ (WIP)
- Bump Sensors
- Ping Sensor
- ~~Cliff Sensor~~ (WIP)
- ~~Battery Data~~ (WIP)

### Commands:
#### Drive (w) `w[param]x`
Drives the CyBot forwards and backwards. [*param*] is in centimeters.
> [*param*] > 0 **( foward )**
> [*param*] < 0 **( backward )**
##### Examples
- Drive backwards 10 centimeters: `w-10x`
- Drive forward 20 centimeters: `w20x`
##### Return Format
`TURN,ANGLE`

#### Turn (d) `d[param]x`
Turns the CyBot left and right. [*param*] is in degrees.
> [*param*] > 0 **( left / co-clockwise )**
> [*param*] < 0 **( right / clockwise )**
##### Examples
- Turn clockwise 15 degrees: `w-10x`
- Turn co-clockwise 45 degrees: `w45x`
##### Return Format
`MOV,DIST`

#### Scan (d) `s`
Returns scan data 180 degrees infront of the CyBot, every two degrees.
##### Return Format
`SCN,SCAN_ANGLE,IR_RAW_VAL,PING_VAL`

## GUI (Python)
### Python Requirements:
- `python 3.9`
- `pyserial`
- `pygame`
- `math`

### GUI Information

#### CyBot
The CyBot is identified by the large white dot, starting in the middle of the window.
##### Direction
The direction of the CyBot is indicated by the green arc on the Player sprite
##### Servo/Scan Angle
The direction of the Servo and Scan Angle is indicated by the blue line on the Player sprite

#### Scan Data
##### IR Data
The small red dots indicate IR scan data
##### Detected Objects
Objects detected are indicated by white polygons

#### Bot Information
##### Mode Information
The current mode (auto/manual) is indicated in the top left.
##### Bump Sensor
The bump sensor data is indicated in the top left, below the mode information.
##### Position Data
The position information of the CyBot is indicated in the top left, below the bump sensor

### GUI Commands
#### Drive `w/s`
Drive the bot 20 centimeters forwards or backwards

#### Turn `a/d`
Turn the bot 15 degrees left or right

#### Scan `s`
Take a 180 degree scan and map objects

#### Calibrate `c`
Take measurements to calibrate IR sensor and update approximation formula

#### Clear `k`
Clears the simulation of scan data

#### Toggle Mode `t`
Toggles between Manual and Autonomous mode. On autonomous mode, the bot automatically drives towards the smallest object, while avoiding objects.