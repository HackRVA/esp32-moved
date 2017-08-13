EESchema Schematic File Version 2
LIBS:power
LIBS:device
LIBS:transistors
LIBS:conn
LIBS:linear
LIBS:regul
LIBS:74xx
LIBS:cmos4000
LIBS:adc-dac
LIBS:memory
LIBS:xilinx
LIBS:microcontrollers
LIBS:dsp
LIBS:microchip
LIBS:analog_switches
LIBS:motorola
LIBS:texas
LIBS:intel
LIBS:audio
LIBS:interface
LIBS:digital-audio
LIBS:philips
LIBS:display
LIBS:cypress
LIBS:siliconi
LIBS:opto
LIBS:atmel
LIBS:contrib
LIBS:valves
LIBS:ESP32-footprints-Shem-Lib
EELAYER 26 0
EELAYER END
$Descr A4 11693 8268
encoding utf-8
Sheet 1 1
Title ""
Date ""
Rev ""
Comp ""
Comment1 ""
Comment2 ""
Comment3 ""
Comment4 ""
$EndDescr
$Comp
L ESP32-WROOM U?
U 1 1 59897FA2
P 5050 3050
F 0 "U?" H 5025 4437 60  0000 C CNN
F 1 "ESP32-WROOM" H 5025 4331 60  0000 C CNN
F 2 "ESP32-footprints-Lib:ESP32-WROOM" H 5400 4400 60  0001 C CNN
F 3 "" H 4600 3500 60  0001 C CNN
	1    5050 3050
	1    0    0    -1  
$EndComp
Text Label 6150 3400 0    60   ~ 0
LED_R
Text Label 6150 3300 0    60   ~ 0
LED_G
Text Label 6150 3200 0    60   ~ 0
LED_B
Wire Wire Line
	5950 3200 6150 3200
Wire Wire Line
	5950 3300 6150 3300
Wire Wire Line
	5950 3400 6150 3400
$Comp
L GND #PWR?
U 1 1 5989812E
P 6100 3700
F 0 "#PWR?" H 6100 3450 50  0001 C CNN
F 1 "GND" H 6105 3527 50  0000 C CNN
F 2 "" H 6100 3700 50  0001 C CNN
F 3 "" H 6100 3700 50  0001 C CNN
	1    6100 3700
	1    0    0    -1  
$EndComp
Wire Wire Line
	5950 3600 6050 3600
Wire Wire Line
	6050 3600 6050 3700
Wire Wire Line
	5950 3700 6100 3700
Connection ~ 6050 3700
$Comp
L GND #PWR?
U 1 1 59898158
P 4600 4200
F 0 "#PWR?" H 4600 3950 50  0001 C CNN
F 1 "GND" H 4605 4027 50  0000 C CNN
F 2 "" H 4600 4200 50  0001 C CNN
F 3 "" H 4600 4200 50  0001 C CNN
	1    4600 4200
	1    0    0    -1  
$EndComp
Wire Wire Line
	4600 4100 4600 4200
$Comp
L GND #PWR?
U 1 1 59898175
P 4000 3750
F 0 "#PWR?" H 4000 3500 50  0001 C CNN
F 1 "GND" H 4005 3577 50  0000 C CNN
F 2 "" H 4000 3750 50  0001 C CNN
F 3 "" H 4000 3750 50  0001 C CNN
	1    4000 3750
	1    0    0    -1  
$EndComp
Wire Wire Line
	4000 3750 4100 3750
Text Label 6150 3100 0    60   ~ 0
TRIGGER
Wire Wire Line
	5950 3100 6150 3100
$EndSCHEMATC
