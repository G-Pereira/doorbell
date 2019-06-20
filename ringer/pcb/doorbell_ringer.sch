EESchema Schematic File Version 4
LIBS:doorbell_ringer-cache
EELAYER 29 0
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
Text Notes 1600 650  0    50   ~ 0
Microphone
Text Notes 6000 7400 0    50   ~ 0
TODO\nDecoupling capacitors
$Comp
L RF_Module:ESP32-WROOM-32 U?
U 1 1 5D04A5B5
P 6000 3600
F 0 "U?" H 6000 5181 50  0000 C CNN
F 1 "ESP32-WROOM-32" H 6000 5090 50  0000 C CNN
F 2 "RF_Module:ESP32-WROOM-32" H 6000 2100 50  0001 C CNN
F 3 "https://www.espressif.com/sites/default/files/documentation/esp32-wroom-32_datasheet_en.pdf" H 5700 3650 50  0001 C CNN
	1    6000 3600
	1    0    0    -1  
$EndComp
Text Label 6600 4600 0    50   ~ 0
mic
$Comp
L Transistor_BJT:2N2219 Q?
U 1 1 5D0C287C
P 1850 1650
F 0 "Q?" H 2040 1696 50  0000 L CNN
F 1 "2N2219" H 2040 1605 50  0000 L CNN
F 2 "Package_TO_SOT_THT:TO-39-3" H 2050 1575 50  0001 L CIN
F 3 "http://www.onsemi.com/pub_link/Collateral/2N2219-D.PDF" H 1850 1650 50  0001 L CNN
	1    1850 1650
	1    0    0    -1  
$EndComp
$Comp
L power:+3.3V #PWR?
U 1 1 5D0C3661
P 1950 950
F 0 "#PWR?" H 1950 800 50  0001 C CNN
F 1 "+3.3V" H 1965 1123 50  0000 C CNN
F 2 "" H 1950 950 50  0001 C CNN
F 3 "" H 1950 950 50  0001 C CNN
	1    1950 950 
	1    0    0    -1  
$EndComp
$Comp
L Device:R R?
U 1 1 5D0C3A49
P 1950 1200
F 0 "R?" H 2020 1246 50  0000 L CNN
F 1 "10k" H 2020 1155 50  0000 L CNN
F 2 "" V 1880 1200 50  0001 C CNN
F 3 "~" H 1950 1200 50  0001 C CNN
	1    1950 1200
	1    0    0    -1  
$EndComp
Wire Wire Line
	1950 1050 1950 1000
Wire Wire Line
	1950 1450 1950 1400
$Comp
L Device:R R?
U 1 1 5D0C7748
P 1700 1400
F 0 "R?" V 1493 1400 50  0000 C CNN
F 1 "100k" V 1584 1400 50  0000 C CNN
F 2 "" V 1630 1400 50  0001 C CNN
F 3 "~" H 1700 1400 50  0001 C CNN
	1    1700 1400
	0    1    1    0   
$EndComp
$Comp
L Device:R R?
U 1 1 5D0C7C59
P 850 1250
F 0 "R?" H 780 1204 50  0000 R CNN
F 1 "10k" H 780 1295 50  0000 R CNN
F 2 "" V 780 1250 50  0001 C CNN
F 3 "~" H 850 1250 50  0001 C CNN
	1    850  1250
	-1   0    0    1   
$EndComp
$Comp
L Device:C C?
U 1 1 5D0C88D8
P 1250 1650
F 0 "C?" V 998 1650 50  0000 C CNN
F 1 "100n" V 1089 1650 50  0000 C CNN
F 2 "" H 1288 1500 50  0001 C CNN
F 3 "~" H 1250 1650 50  0001 C CNN
	1    1250 1650
	0    1    1    0   
$EndComp
Wire Wire Line
	1850 1400 1950 1400
Connection ~ 1950 1400
Wire Wire Line
	1950 1400 1950 1350
$Comp
L Device:Microphone_Condenser MK?
U 1 1 5D0C96FD
P 850 1900
F 0 "MK?" H 980 1946 50  0000 L CNN
F 1 "Microphone_Condenser" H 980 1855 50  0000 L CNN
F 2 "" V 850 2000 50  0001 C CNN
F 3 "~" V 850 2000 50  0001 C CNN
	1    850  1900
	1    0    0    -1  
$EndComp
Wire Wire Line
	850  1700 850  1650
Wire Wire Line
	850  1650 1100 1650
Wire Wire Line
	850  1100 850  1000
Wire Wire Line
	1950 950  1950 1000
Wire Wire Line
	850  1400 850  1650
Connection ~ 850  1650
$Comp
L power:GND #PWR?
U 1 1 5D0D0805
P 1950 2150
F 0 "#PWR?" H 1950 1900 50  0001 C CNN
F 1 "GND" H 1955 1977 50  0000 C CNN
F 2 "" H 1950 2150 50  0001 C CNN
F 3 "" H 1950 2150 50  0001 C CNN
	1    1950 2150
	1    0    0    -1  
$EndComp
Wire Wire Line
	850  2150 850  2100
Wire Wire Line
	1950 1850 1950 2150
Wire Wire Line
	1400 1650 1500 1650
Wire Wire Line
	1550 1400 1500 1400
Wire Wire Line
	1500 1400 1500 1650
Connection ~ 1500 1650
Wire Wire Line
	1500 1650 1650 1650
$Comp
L Device:C C?
U 1 1 5D0D5E44
P 2300 1400
F 0 "C?" V 2048 1400 50  0000 C CNN
F 1 "100n" V 2139 1400 50  0000 C CNN
F 2 "" H 2338 1250 50  0001 C CNN
F 3 "~" H 2300 1400 50  0001 C CNN
	1    2300 1400
	0    1    1    0   
$EndComp
Wire Wire Line
	2150 1400 1950 1400
$Comp
L Device:R R?
U 1 1 5D0D7FB2
P 2650 1200
F 0 "R?" H 2720 1246 50  0000 L CNN
F 1 "1k" H 2720 1155 50  0000 L CNN
F 2 "" V 2580 1200 50  0001 C CNN
F 3 "~" H 2650 1200 50  0001 C CNN
	1    2650 1200
	1    0    0    -1  
$EndComp
$Comp
L Device:R R?
U 1 1 5D0D84BB
P 2650 1600
F 0 "R?" H 2720 1646 50  0000 L CNN
F 1 "1k" H 2720 1555 50  0000 L CNN
F 2 "" V 2580 1600 50  0001 C CNN
F 3 "~" H 2650 1600 50  0001 C CNN
	1    2650 1600
	1    0    0    -1  
$EndComp
Wire Wire Line
	2450 1400 2650 1400
Wire Wire Line
	2650 1450 2650 1400
Connection ~ 2650 1400
Wire Wire Line
	2650 1400 2900 1400
Wire Wire Line
	2650 1350 2650 1400
Wire Wire Line
	2650 1750 2650 2150
Wire Wire Line
	2650 2150 1950 2150
Connection ~ 1950 2150
Wire Wire Line
	2650 1050 2650 1000
Wire Wire Line
	2650 1000 1950 1000
Connection ~ 1950 1000
Wire Wire Line
	850  1000 1950 1000
Wire Wire Line
	850  2150 1950 2150
Text Label 2900 1400 0    50   ~ 0
mic
$EndSCHEMATC
