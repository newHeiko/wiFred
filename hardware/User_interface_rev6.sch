EESchema Schematic File Version 4
EELAYER 30 0
EELAYER END
$Descr A4 11693 8268
encoding utf-8
Sheet 2 3
Title "Wireless FREDI"
Date "2021-06-28"
Rev "0.62"
Comp "Heiko Rosemann (heiko.rosemann@web.de) CC-BY-SA"
Comment1 ""
Comment2 ""
Comment3 "Keys, LEDs and Potentiometer for throttle"
Comment4 "Flashlight"
$EndDescr
$Comp
L wfred_rev6-rescue:POT-RESCUE-wfred_rev2-wfred_rev2-rescue RV201
U 1 1 59210A4F
P 1000 5250
F 0 "RV201" H 1000 5150 50  0000 C CNN
F 1 "10k lin P160KNPD-4FC20B10K" H 1000 5250 50  0000 C CNN
F 2 "myFootprints:P160KNPD" H 1000 5250 60  0001 C CNN
F 3 "" H 1000 5250 60  0000 C CNN
F 4 "858-P160KNP4FC20B10K" H 1000 5250 60  0001 C CNN "Mouser"
	1    1000 5250
	0    1    1    0   
$EndComp
Wire Wire Line
	1000 5500 1000 6050
$Comp
L wfred_rev6-rescue:R-RESCUE-wfred_rev2-wfred_rev2-rescue R202
U 1 1 59210B02
P 1550 5250
F 0 "R202" V 1630 5250 40  0000 C CNN
F 1 "4k7" V 1557 5251 40  0000 C CNN
F 2 "Resistor_SMD:R_0805_2012Metric_Pad1.15x1.40mm_HandSolder" V 1480 5250 30  0001 C CNN
F 3 "" H 1550 5250 30  0000 C CNN
F 4 "SMD-0805 4,70K" V 1550 5250 60  0001 C CNN "Reichelt"
F 5 "C17673" V 1550 5250 50  0001 C CNN "LCSC"
	1    1550 5250
	0    1    1    0   
$EndComp
Wire Wire Line
	1150 5250 1300 5250
Wire Wire Line
	1800 5250 2000 5250
Wire Wire Line
	2000 5250 2000 5450
Wire Wire Line
	2000 5850 2000 6050
Connection ~ 2000 5250
Text GLabel 2500 5250 2    60   Output ~ 0
SPEED
$Comp
L wfred_rev6-rescue:R-RESCUE-wfred_rev2-wfred_rev2-rescue R207
U 1 1 592110C5
P 7500 1500
F 0 "R207" V 7580 1500 40  0000 C CNN
F 1 "680R" V 7507 1501 40  0000 C CNN
F 2 "Resistor_SMD:R_0805_2012Metric_Pad1.15x1.40mm_HandSolder" V 7430 1500 30  0001 C CNN
F 3 "" H 7500 1500 30  0000 C CNN
F 4 "SMD-0805 680" V 7500 1500 60  0001 C CNN "Reichelt"
F 5 "C17798" V 7500 1500 50  0001 C CNN "LCSC"
	1    7500 1500
	-1   0    0    1   
$EndComp
$Comp
L wfred_rev6-rescue:LED-RESCUE-wfred_rev2-wfred_rev2-rescue D205
U 1 1 59211179
P 7500 2150
F 0 "D205" H 7500 2250 50  0000 C CNN
F 1 "STOP - red" H 7500 2050 50  0000 C CNN
F 2 "LED_THT:LED_D3.0mm" H 7500 2150 60  0001 C CNN
F 3 "" H 7500 2150 60  0000 C CNN
F 4 "LED 3MM RT" H 7500 2150 60  0001 C CNN "Reichelt"
	1    7500 2150
	0    1    1    0   
$EndComp
Wire Wire Line
	7500 1750 7500 1950
$Comp
L wfred_rev6-rescue:LED-RESCUE-wfred_rev2-wfred_rev2-rescue D206
U 1 1 59211323
P 7800 2150
F 0 "D206" H 7800 2250 50  0000 C CNN
F 1 "FORWARD - green" H 7800 2050 50  0000 C CNN
F 2 "LED_THT:LED_D3.0mm" H 7800 2150 60  0001 C CNN
F 3 "" H 7800 2150 60  0000 C CNN
F 4 "LED 3MM GN" H 7800 2150 60  0001 C CNN "Reichelt"
	1    7800 2150
	0    1    1    0   
$EndComp
$Comp
L wfred_rev6-rescue:LED-RESCUE-wfred_rev2-wfred_rev2-rescue D207
U 1 1 592113F4
P 8100 2150
F 0 "D207" H 8100 2250 50  0000 C CNN
F 1 "REVERSE - green" H 8100 2050 50  0000 C CNN
F 2 "LED_THT:LED_D3.0mm" H 8100 2150 60  0001 C CNN
F 3 "" H 8100 2150 60  0000 C CNN
F 4 "LED 3MM GN" H 8100 2150 60  0001 C CNN "Reichelt"
	1    8100 2150
	0    1    1    0   
$EndComp
Wire Wire Line
	7500 2350 7500 2650
Wire Wire Line
	7500 2650 7200 2650
Text GLabel 7200 2650 0    60   Input ~ 0
LED_STOP
Text GLabel 7200 2750 0    60   Input ~ 0
LED_FWD
$Comp
L wfred_rev6-rescue:SW_PUSH-device-wfred_rev2-rescue SW204
U 1 1 59212D1C
P 1500 2500
F 0 "SW204" H 1650 2610 50  0000 C CNN
F 1 "ESTOP" H 1500 2420 50  0000 C CNN
F 2 "Button_Switch_THT:SW_PUSH_6mm_H9.5mm" H 1500 2500 60  0001 C CNN
F 3 "" H 1500 2500 60  0000 C CNN
F 4 "642-ADTS65RV" H 1500 2500 60  0001 C CNN "Mouser1"
F 5 "113-DTS65RV" H 1500 2500 60  0001 C CNN "Mouser2"
	1    1500 2500
	1    0    0    -1  
$EndComp
Wire Wire Line
	1800 2000 2100 2000
Wire Wire Line
	1800 1000 2100 1000
Wire Wire Line
	3400 1000 3700 1000
Wire Wire Line
	3400 1500 3700 1500
Wire Wire Line
	3400 2000 3700 2000
Wire Wire Line
	3400 2500 3700 2500
$Comp
L wfred_rev6-rescue:SW_PUSH-device-wfred_rev2-rescue SW205
U 1 1 592B1B9E
P 3100 1000
F 0 "SW205" H 3250 1110 50  0000 C CNN
F 1 "F0" H 3100 920 50  0000 C CNN
F 2 "Button_Switch_THT:SW_PUSH_6mm_H9.5mm" H 3100 1000 60  0001 C CNN
F 3 "" H 3100 1000 60  0000 C CNN
F 4 "642-ADTS65KV" H 1500 1000 60  0001 C CNN "Mouser1"
F 5 "TASTER 3301B" H 1500 1000 60  0001 C CNN "Reichelt"
F 6 "113-DTS65KV" H 1500 1000 60  0001 C CNN "Mouser2"
	1    3100 1000
	1    0    0    -1  
$EndComp
$Comp
L wfred_rev6-rescue:SW_PUSH-device-wfred_rev2-rescue SW201
U 1 1 592B1C04
P 1500 1000
F 0 "SW201" H 1650 1110 50  0000 C CNN
F 1 "F1" H 1500 920 50  0000 C CNN
F 2 "Button_Switch_THT:SW_PUSH_6mm_H9.5mm" H 1500 1000 60  0001 C CNN
F 3 "" H 1500 1000 60  0000 C CNN
F 4 "642-ADTS65KV" H 1500 1000 60  0001 C CNN "Mouser1"
F 5 "TASTER 3301B" H 1500 1000 60  0001 C CNN "Reichelt"
F 6 "113-DTS65KV" H 1500 1000 60  0001 C CNN "Mouser2"
	1    1500 1000
	1    0    0    -1  
$EndComp
$Comp
L wfred_rev6-rescue:SW_PUSH-device-wfred_rev2-rescue SW206
U 1 1 592B1C64
P 3100 1500
F 0 "SW206" H 3250 1610 50  0000 C CNN
F 1 "F2" H 3100 1420 50  0000 C CNN
F 2 "Button_Switch_THT:SW_PUSH_6mm_H9.5mm" H 3100 1500 60  0001 C CNN
F 3 "" H 3100 1500 60  0000 C CNN
F 4 "642-ADTS65KV" H 1500 1000 60  0001 C CNN "Mouser1"
F 5 "TASTER 3301B" H 1500 1000 60  0001 C CNN "Reichelt"
F 6 "113-DTS65KV" H 1500 1000 60  0001 C CNN "Mouser2"
	1    3100 1500
	1    0    0    -1  
$EndComp
$Comp
L wfred_rev6-rescue:SW_PUSH-device-wfred_rev2-rescue SW213
U 1 1 592B1CC9
P 4700 1000
F 0 "SW213" H 4850 1110 50  0000 C CNN
F 1 "F3" H 4700 920 50  0000 C CNN
F 2 "Button_Switch_THT:SW_PUSH_6mm_H9.5mm" H 4700 1000 60  0001 C CNN
F 3 "" H 4700 1000 60  0000 C CNN
F 4 "642-ADTS65KV" H 1500 1000 60  0001 C CNN "Mouser1"
F 5 "TASTER 3301B" H 1500 1000 60  0001 C CNN "Reichelt"
F 6 "113-DTS65KV" H 1500 1000 60  0001 C CNN "Mouser2"
	1    4700 1000
	1    0    0    -1  
$EndComp
$Comp
L wfred_rev6-rescue:SW_PUSH-device-wfred_rev2-rescue SW202
U 1 1 592B1D39
P 1500 1500
F 0 "SW202" H 1650 1610 50  0000 C CNN
F 1 "F4" H 1500 1420 50  0000 C CNN
F 2 "Button_Switch_THT:SW_PUSH_6mm_H9.5mm" H 1500 1500 60  0001 C CNN
F 3 "" H 1500 1500 60  0000 C CNN
F 4 "642-ADTS65KV" H 1500 1000 60  0001 C CNN "Mouser1"
F 5 "TASTER 3301B" H 1500 1000 60  0001 C CNN "Reichelt"
F 6 "113-DTS65KV" H 1500 1000 60  0001 C CNN "Mouser2"
	1    1500 1500
	1    0    0    -1  
$EndComp
$Comp
L wfred_rev6-rescue:SW_PUSH-device-wfred_rev2-rescue SW215
U 1 1 592B1D94
P 4700 2000
F 0 "SW215" H 4850 2110 50  0000 C CNN
F 1 "SHIFT" H 4700 1920 50  0000 C CNN
F 2 "Button_Switch_THT:SW_PUSH_6mm_H9.5mm" H 4700 2000 60  0001 C CNN
F 3 "" H 4700 2000 60  0000 C CNN
F 4 "642-ADTS65YV" H 4700 2000 60  0001 C CNN "Mouser1"
F 5 "113-DTS65YV" H 4700 2000 60  0001 C CNN "Mouser2"
	1    4700 2000
	1    0    0    -1  
$EndComp
Wire Wire Line
	7200 2750 7800 2750
$Comp
L wfred_rev6-rescue:GND-RESCUE-wfred_rev2-wfred_rev2-rescue #PWR0204
U 1 1 59306CC7
P 2000 6050
AR Path="/59306CC7" Ref="#PWR0204"  Part="1" 
AR Path="/5920DD4A/59306CC7" Ref="#PWR0204"  Part="1" 
F 0 "#PWR0204" H 2000 6050 30  0001 C CNN
F 1 "GND" H 2000 5980 30  0001 C CNN
F 2 "" H 2000 6050 60  0001 C CNN
F 3 "" H 2000 6050 60  0001 C CNN
	1    2000 6050
	1    0    0    -1  
$EndComp
$Comp
L wfred_rev6-rescue:GND-RESCUE-wfred_rev2-wfred_rev2-rescue #PWR0203
U 1 1 59306D0E
P 1000 6050
AR Path="/59306D0E" Ref="#PWR0203"  Part="1" 
AR Path="/5920DD4A/59306D0E" Ref="#PWR0203"  Part="1" 
F 0 "#PWR0203" H 1000 6050 30  0001 C CNN
F 1 "GND" H 1000 5980 30  0001 C CNN
F 2 "" H 1000 6050 60  0001 C CNN
F 3 "" H 1000 6050 60  0001 C CNN
	1    1000 6050
	1    0    0    -1  
$EndComp
Wire Wire Line
	7800 2750 7800 2350
Wire Wire Line
	7800 1750 7800 1950
Wire Wire Line
	8100 1750 8100 1950
$Comp
L wfred_rev6-rescue:SW_PUSH-device-wfred_rev2-rescue SW207
U 1 1 5A125A5B
P 3100 2000
F 0 "SW207" H 3250 2110 50  0000 C CNN
F 1 "F5" H 3100 1920 50  0000 C CNN
F 2 "Button_Switch_THT:SW_PUSH_6mm_H9.5mm" H 3100 2000 60  0001 C CNN
F 3 "" H 3100 2000 60  0000 C CNN
F 4 "642-ADTS65KV" H 1500 1000 60  0001 C CNN "Mouser1"
F 5 "TASTER 3301B" H 1500 1000 60  0001 C CNN "Reichelt"
F 6 "113-DTS65KV" H 1500 1000 60  0001 C CNN "Mouser2"
	1    3100 2000
	1    0    0    -1  
$EndComp
$Comp
L wfred_rev6-rescue:SWITCH_INV-device-wfred_rev2-rescue SW209
U 1 1 5A12804B
P 4250 3950
F 0 "SW209" H 4050 4100 50  0000 C CNN
F 1 "LOCO1" H 4100 3800 50  0000 C CNN
F 2 "myFootprints:OS102011MS2Q" H 4250 3950 60  0001 C CNN
F 3 "" H 4250 3950 60  0000 C CNN
F 4 "611-OS102011MS2QN1" H 4250 3950 60  0001 C CNN "Mouser"
	1    4250 3950
	-1   0    0    -1  
$EndComp
$Comp
L wfred_rev6-rescue:SWITCH_INV-device-wfred_rev2-rescue SW210
U 1 1 5A128104
P 4250 4750
F 0 "SW210" H 4050 4900 50  0000 C CNN
F 1 "LOCO2" H 4100 4600 50  0000 C CNN
F 2 "myFootprints:OS102011MS2Q" H 4250 4750 60  0001 C CNN
F 3 "" H 4250 4750 60  0000 C CNN
F 4 "611-OS102011MS2QN1" H 4250 4750 60  0001 C CNN "Mouser"
	1    4250 4750
	-1   0    0    -1  
$EndComp
$Comp
L wfred_rev6-rescue:SWITCH_INV-device-wfred_rev2-rescue SW211
U 1 1 5A12817D
P 4250 5350
F 0 "SW211" H 4050 5500 50  0000 C CNN
F 1 "LOCO3" H 4100 5200 50  0000 C CNN
F 2 "myFootprints:OS102011MS2Q" H 4250 5350 60  0001 C CNN
F 3 "" H 4250 5350 60  0000 C CNN
F 4 "611-OS102011MS2QN1" H 4250 5350 60  0001 C CNN "Mouser"
	1    4250 5350
	-1   0    0    -1  
$EndComp
$Comp
L wfred_rev6-rescue:SWITCH_INV-device-wfred_rev2-rescue SW216
U 1 1 5A153621
P 5000 2500
F 0 "SW216" H 4800 2650 50  0000 C CNN
F 1 "DIRECTION" H 4850 2350 50  0000 C CNN
F 2 "myFootprints:100SP1T1B1M1QEH" H 5000 2500 60  0001 C CNN
F 3 "" H 5000 2500 60  0000 C CNN
F 4 "612-100-A1111" H 5000 2500 60  0001 C CNN "Mouser"
	1    5000 2500
	1    0    0    -1  
$EndComp
Wire Wire Line
	4200 2500 4500 2500
Wire Wire Line
	5500 2400 5800 2400
Wire Wire Line
	5800 2600 5500 2600
$Comp
L wfred_rev6-rescue:SW_PUSH-device-wfred_rev2-rescue SW214
U 1 1 5BDFCFFA
P 4700 1500
F 0 "SW214" H 4850 1610 50  0000 C CNN
F 1 "F6" H 4700 1420 50  0000 C CNN
F 2 "Button_Switch_THT:SW_PUSH_6mm_H9.5mm" H 4700 1500 60  0001 C CNN
F 3 "" H 4700 1500 60  0000 C CNN
F 4 "642-ADTS65KV" H 1500 1000 60  0001 C CNN "Mouser1"
F 5 "TASTER 3301B" H 1500 1000 60  0001 C CNN "Reichelt"
F 6 "113-DTS65KV" H 1500 1000 60  0001 C CNN "Mouser2"
	1    4700 1500
	1    0    0    -1  
$EndComp
$Comp
L wfred_rev6-rescue:SW_PUSH-device-wfred_rev2-rescue SW203
U 1 1 5BDFD0F5
P 1500 2000
F 0 "SW203" H 1650 2110 50  0000 C CNN
F 1 "F7" H 1500 1920 50  0000 C CNN
F 2 "Button_Switch_THT:SW_PUSH_6mm_H9.5mm" H 1500 2000 60  0001 C CNN
F 3 "" H 1500 2000 60  0000 C CNN
F 4 "642-ADTS65KV" H 1500 1000 60  0001 C CNN "Mouser1"
F 5 "TASTER 3301B" H 1500 1000 60  0001 C CNN "Reichelt"
F 6 "113-DTS65KV" H 1500 1000 60  0001 C CNN "Mouser2"
	1    1500 2000
	1    0    0    -1  
$EndComp
$Comp
L wfred_rev6-rescue:SW_PUSH-device-wfred_rev2-rescue SW208
U 1 1 5BDFD17C
P 3100 2500
F 0 "SW208" H 3250 2610 50  0000 C CNN
F 1 "F8" H 3100 2420 50  0000 C CNN
F 2 "Button_Switch_THT:SW_PUSH_6mm_H9.5mm" H 3100 2500 60  0001 C CNN
F 3 "" H 3100 2500 60  0000 C CNN
F 4 "642-ADTS65KV" H 1500 1000 60  0001 C CNN "Mouser1"
F 5 "TASTER 3301B" H 1500 1000 60  0001 C CNN "Reichelt"
F 6 "113-DTSM65KV" H 1500 1000 60  0001 C CNN "Mouser2"
	1    3100 2500
	1    0    0    -1  
$EndComp
Wire Wire Line
	5000 1000 5300 1000
Wire Wire Line
	5000 1500 5300 1500
Text GLabel 5300 1500 2    60   Output ~ 0
F6
Wire Wire Line
	1800 1500 2100 1500
Text GLabel 5800 2400 2    60   Output ~ 0
FWD
Text GLabel 5300 1000 2    60   Output ~ 0
F3
Wire Wire Line
	2100 2500 1800 2500
$Comp
L wfred_rev6-rescue:MIC2860-my_devices-wfred_rev2-rescue IC201
U 1 1 5CD17656
P 9450 4450
F 0 "IC201" H 9450 4550 60  0000 C CNN
F 1 "CAT4002ATD-GT3" H 9450 4450 60  0000 C CNN
F 2 "Package_TO_SOT_SMD:SOT-23-6_Handsoldering" H 9450 4450 60  0001 C CNN
F 3 "" H 9450 4450 60  0001 C CNN
F 4 "863-CAT4002ATD-GT3" H 9450 4450 60  0001 C CNN "Mouser"
F 5 "C184641" H 9450 4450 50  0001 C CNN "LCSC"
	1    9450 4450
	1    0    0    -1  
$EndComp
$Comp
L wfred_rev6-rescue:LED-device-wfred_rev2-rescue D208
U 1 1 5CD176D3
P 10250 3950
F 0 "D208" H 10250 4050 50  0000 C CNN
F 1 "Cree C543A-WMN-CCCKK141" H 10250 3850 50  0000 C CNN
F 2 "myFootprints:LED_D5.0mm_Horicontal_FLIPPED_O1.27mm" H 10250 3950 60  0001 C CNN
F 3 "" H 10250 3950 60  0000 C CNN
F 4 "941-C543AWMNCCCKK141" H 10250 3950 60  0001 C CNN "Mouser"
	1    10250 3950
	0    1    1    0   
$EndComp
$Comp
L wfred_rev6-rescue:LED-device-wfred_rev2-rescue D209
U 1 1 5CD1782F
P 10550 3950
F 0 "D209" H 10550 4050 50  0000 C CNN
F 1 "Cree C543A-WMN-CCCKK141" H 10550 3850 50  0000 C CNN
F 2 "LED_THT:LED_D5.0mm_Horizontal_O1.27mm_Z3.0mm_Clear" H 10550 3950 60  0001 C CNN
F 3 "" H 10550 3950 60  0000 C CNN
F 4 "941-C543AWMNCCCKK141" H 10550 3950 60  0001 C CNN "Mouser"
	1    10550 3950
	0    1    1    0   
$EndComp
Wire Wire Line
	10050 4350 10250 4350
Wire Wire Line
	10250 4350 10250 4150
Wire Wire Line
	10050 4550 10550 4550
Wire Wire Line
	10550 4550 10550 4150
$Comp
L power:+BATT #PWR0217
U 1 1 5CD1798E
P 10250 3550
F 0 "#PWR0217" H 10250 3500 20  0001 C CNN
F 1 "+BATT" H 10250 3650 30  0000 C CNN
F 2 "" H 10250 3550 60  0001 C CNN
F 3 "" H 10250 3550 60  0001 C CNN
	1    10250 3550
	1    0    0    -1  
$EndComp
$Comp
L power:+BATT #PWR0218
U 1 1 5CD17A12
P 10550 3550
F 0 "#PWR0218" H 10550 3500 20  0001 C CNN
F 1 "+BATT" H 10550 3650 30  0000 C CNN
F 2 "" H 10550 3550 60  0001 C CNN
F 3 "" H 10550 3550 60  0001 C CNN
	1    10550 3550
	1    0    0    -1  
$EndComp
$Comp
L power:+BATT #PWR0216
U 1 1 5CD17A5E
P 8650 3550
F 0 "#PWR0216" H 8650 3500 20  0001 C CNN
F 1 "+BATT" H 8650 3650 30  0000 C CNN
F 2 "" H 8650 3550 60  0001 C CNN
F 3 "" H 8650 3550 60  0001 C CNN
	1    8650 3550
	1    0    0    -1  
$EndComp
Wire Wire Line
	8650 3550 8650 4150
Wire Wire Line
	8650 4150 8850 4150
Wire Wire Line
	10250 3550 10250 3750
Wire Wire Line
	10550 3550 10550 3750
$Comp
L wfred_rev6-rescue:R-RESCUE-wfred_rev2-wfred_rev2-rescue R211
U 1 1 5CD17C99
P 8400 4350
F 0 "R211" V 8480 4350 40  0000 C CNN
F 1 "4k7" V 8407 4351 40  0000 C CNN
F 2 "Resistor_SMD:R_0805_2012Metric_Pad1.15x1.40mm_HandSolder" V 8330 4350 30  0001 C CNN
F 3 "" H 8400 4350 30  0000 C CNN
F 4 "SMD-0805 4,70K" V 8400 4350 60  0001 C CNN "Reichelt"
F 5 "C17673" V 8400 4350 50  0001 C CNN "LCSC"
	1    8400 4350
	0    -1   -1   0   
$EndComp
$Comp
L wfred_rev6-rescue:GND-power-wfred_rev2-rescue #PWR0214
U 1 1 5CD17DBA
P 7950 5750
F 0 "#PWR0214" H 7950 5750 30  0001 C CNN
F 1 "GND" H 7950 5680 30  0001 C CNN
F 2 "" H 7950 5750 60  0001 C CNN
F 3 "" H 7950 5750 60  0001 C CNN
	1    7950 5750
	1    0    0    -1  
$EndComp
Wire Wire Line
	7950 4250 7950 4350
Wire Wire Line
	7950 4350 8150 4350
Wire Wire Line
	8650 4350 8850 4350
Text GLabel 7700 4550 0    60   Input ~ 0
FLASHLIGHT
Wire Wire Line
	7700 4550 8250 4550
$Comp
L wfred_rev6-rescue:C-RESCUE-wfred_rev2-wfred_rev2-rescue C203
U 1 1 5CD18EB9
P 7950 4050
AR Path="/5CD18EB9" Ref="C203"  Part="1" 
AR Path="/5920DD4A/5CD18EB9" Ref="C203"  Part="1" 
F 0 "C203" H 7950 4150 40  0000 L CNN
F 1 "4u7" H 7956 3965 40  0000 L CNN
F 2 "Capacitor_SMD:C_0805_2012Metric_Pad1.15x1.40mm_HandSolder" H 7988 3900 30  0001 C CNN
F 3 "" H 7950 4050 60  0000 C CNN
F 4 "KEM X5R0805 4,7U" H 7950 4050 60  0001 C CNN "Reichelt"
F 5 "C1779" H 7950 4050 50  0001 C CNN "LCSC"
	1    7950 4050
	1    0    0    -1  
$EndComp
$Comp
L power:+BATT #PWR0213
U 1 1 5CD18F9C
P 7950 3550
F 0 "#PWR0213" H 7950 3500 20  0001 C CNN
F 1 "+BATT" H 7950 3650 30  0000 C CNN
F 2 "" H 7950 3550 60  0001 C CNN
F 3 "" H 7950 3550 60  0001 C CNN
	1    7950 3550
	1    0    0    -1  
$EndComp
Wire Wire Line
	7950 3550 7950 3850
Connection ~ 7950 4350
Wire Wire Line
	2000 5250 2500 5250
$Comp
L wfred_rev6-rescue:R-RESCUE-wfred_rev2-wfred_rev2-rescue R201
U 1 1 6014A11D
P 1000 4400
F 0 "R201" V 1080 4400 40  0000 C CNN
F 1 "4k7" V 1007 4401 40  0000 C CNN
F 2 "Resistor_SMD:R_0805_2012Metric_Pad1.15x1.40mm_HandSolder" V 930 4400 30  0001 C CNN
F 3 "" H 1000 4400 30  0000 C CNN
F 4 "SMD-0805 4,70K" V 1000 4400 60  0001 C CNN "Reichelt"
F 5 "C17673" V 1000 4400 50  0001 C CNN "LCSC"
	1    1000 4400
	-1   0    0    1   
$EndComp
Wire Wire Line
	1000 4650 1000 5000
$Comp
L power:VCC #PWR0202
U 1 1 6014FE2F
P 1000 4050
F 0 "#PWR0202" H 1000 3900 50  0001 C CNN
F 1 "VCC" H 1015 4223 50  0000 C CNN
F 2 "" H 1000 4050 50  0001 C CNN
F 3 "" H 1000 4050 50  0001 C CNN
	1    1000 4050
	1    0    0    -1  
$EndComp
Wire Wire Line
	1000 4050 1000 4150
$Comp
L power:+BATT #PWR0206
U 1 1 6016009A
P 3450 3750
F 0 "#PWR0206" H 3450 3600 50  0001 C CNN
F 1 "+BATT" H 3465 3923 50  0000 C CNN
F 2 "" H 3450 3750 50  0001 C CNN
F 3 "" H 3450 3750 50  0001 C CNN
	1    3450 3750
	1    0    0    -1  
$EndComp
Wire Wire Line
	3450 3750 3450 3850
Wire Wire Line
	3450 3850 3750 3850
Wire Wire Line
	3450 3850 3450 4650
Wire Wire Line
	3450 4650 3750 4650
Connection ~ 3450 3850
Wire Wire Line
	3450 4650 3450 5250
Connection ~ 3450 4650
Wire Wire Line
	3550 5450 3550 4850
Wire Wire Line
	3550 4850 3750 4850
Wire Wire Line
	3550 4850 3550 4050
Wire Wire Line
	3550 4050 3750 4050
Connection ~ 3550 4850
Connection ~ 3550 5450
Connection ~ 3450 5250
Wire Wire Line
	3550 5450 3750 5450
Wire Wire Line
	3450 5250 3750 5250
Connection ~ 3550 6250
Wire Wire Line
	3550 6250 3550 5450
Wire Wire Line
	3550 6250 3750 6250
Wire Wire Line
	3550 6450 3550 6250
$Comp
L power:GND #PWR0207
U 1 1 6016B655
P 3550 6450
F 0 "#PWR0207" H 3550 6200 50  0001 C CNN
F 1 "GND" H 3555 6277 50  0000 C CNN
F 2 "" H 3550 6450 50  0001 C CNN
F 3 "" H 3550 6450 50  0001 C CNN
	1    3550 6450
	1    0    0    -1  
$EndComp
Wire Wire Line
	3450 6050 3750 6050
Wire Wire Line
	3450 5250 3450 6050
$Comp
L wfred_rev6-rescue:SWITCH_INV-device-wfred_rev2-rescue SW212
U 1 1 5A1281F9
P 4250 6150
F 0 "SW212" H 4050 6300 50  0000 C CNN
F 1 "LOCO4" H 4100 6000 50  0000 C CNN
F 2 "myFootprints:OS102011MS2Q" H 4250 6150 60  0001 C CNN
F 3 "" H 4250 6150 60  0000 C CNN
F 4 "611-OS102011MS2QN1" H 4250 6150 60  0001 C CNN "Mouser"
	1    4250 6150
	-1   0    0    -1  
$EndComp
Text GLabel 5300 2000 2    60   Output ~ 0
SHIFT
Wire Wire Line
	5250 4750 4850 4750
Wire Wire Line
	4750 3950 4850 3950
$Comp
L wfred_rev6-rescue:R-RESCUE-wfred_rev2-wfred_rev2-rescue R203
U 1 1 601754A9
P 5500 3950
F 0 "R203" V 5580 3950 40  0000 C CNN
F 1 "4k7" V 5507 3951 40  0000 C CNN
F 2 "Resistor_SMD:R_0805_2012Metric_Pad1.15x1.40mm_HandSolder" V 5430 3950 30  0001 C CNN
F 3 "" H 5500 3950 30  0000 C CNN
F 4 "SMD-0805 4,70K" V 5500 3950 60  0001 C CNN "Reichelt"
F 5 "C17673" V 5500 3950 50  0001 C CNN "LCSC"
	1    5500 3950
	0    -1   -1   0   
$EndComp
$Comp
L wfred_rev6-rescue:R-RESCUE-wfred_rev2-wfred_rev2-rescue R204
U 1 1 60175D73
P 5500 4750
F 0 "R204" V 5580 4750 40  0000 C CNN
F 1 "4k7" V 5507 4751 40  0000 C CNN
F 2 "Resistor_SMD:R_0805_2012Metric_Pad1.15x1.40mm_HandSolder" V 5430 4750 30  0001 C CNN
F 3 "" H 5500 4750 30  0000 C CNN
F 4 "SMD-0805 4,70K" V 5500 4750 60  0001 C CNN "Reichelt"
F 5 "C17673" V 5500 4750 50  0001 C CNN "LCSC"
	1    5500 4750
	0    -1   -1   0   
$EndComp
$Comp
L Diode:BAT54C D203
U 1 1 60180C59
P 5850 4350
F 0 "D203" V 5804 4438 50  0000 L CNN
F 1 "BAT54C" V 5895 4438 50  0000 L CNN
F 2 "Package_TO_SOT_SMD:SOT-23" H 5925 4475 50  0001 L CNN
F 3 "http://www.diodes.com/_files/datasheets/ds11005.pdf" H 5770 4350 50  0001 C CNN
F 4 "C556168" V 5850 4350 50  0001 C CNN "LCSC"
F 5 "BAT 54C NXP" V 5850 4350 50  0001 C CNN "Reichelt"
F 6 "583-BAT54C" V 5850 4350 50  0001 C CNN "Mouser"
	1    5850 4350
	0    -1   1    0   
$EndComp
$Comp
L power:VCC #PWR0209
U 1 1 6019194D
P 6150 3800
F 0 "#PWR0209" H 6150 3650 50  0001 C CNN
F 1 "VCC" H 6165 3973 50  0000 C CNN
F 2 "" H 6150 3800 50  0001 C CNN
F 3 "" H 6150 3800 50  0001 C CNN
	1    6150 3800
	1    0    0    -1  
$EndComp
Wire Wire Line
	6150 3800 6150 4350
Wire Wire Line
	6150 4350 6050 4350
Wire Wire Line
	6150 4350 6150 5750
Connection ~ 6150 4350
Wire Wire Line
	6150 5750 6050 5750
$Comp
L Diode:BAT54C D204
U 1 1 6018DE30
P 5850 5750
F 0 "D204" V 5804 5838 50  0000 L CNN
F 1 "BAT54C" V 5895 5838 50  0000 L CNN
F 2 "Package_TO_SOT_SMD:SOT-23" H 5925 5875 50  0001 L CNN
F 3 "http://www.diodes.com/_files/datasheets/ds11005.pdf" H 5770 5750 50  0001 C CNN
F 4 "C556168" V 5850 5750 50  0001 C CNN "LCSC"
F 5 "BAT 54C NXP" V 5850 5750 50  0001 C CNN "Reichelt"
F 6 "583-BAT54C" V 5850 5750 50  0001 C CNN "Mouser"
	1    5850 5750
	0    -1   1    0   
$EndComp
$Comp
L wfred_rev6-rescue:R-RESCUE-wfred_rev2-wfred_rev2-rescue R206
U 1 1 601766E1
P 5500 6150
F 0 "R206" V 5580 6150 40  0000 C CNN
F 1 "4k7" V 5507 6151 40  0000 C CNN
F 2 "Resistor_SMD:R_0805_2012Metric_Pad1.15x1.40mm_HandSolder" V 5430 6150 30  0001 C CNN
F 3 "" H 5500 6150 30  0000 C CNN
F 4 "SMD-0805 4,70K" V 5500 6150 60  0001 C CNN "Reichelt"
F 5 "C17673" V 5500 6150 50  0001 C CNN "LCSC"
	1    5500 6150
	0    -1   -1   0   
$EndComp
$Comp
L wfred_rev6-rescue:R-RESCUE-wfred_rev2-wfred_rev2-rescue R205
U 1 1 601763B4
P 5500 5350
F 0 "R205" V 5580 5350 40  0000 C CNN
F 1 "4k7" V 5507 5351 40  0000 C CNN
F 2 "Resistor_SMD:R_0805_2012Metric_Pad1.15x1.40mm_HandSolder" V 5430 5350 30  0001 C CNN
F 3 "" H 5500 5350 30  0000 C CNN
F 4 "SMD-0805 4,70K" V 5500 5350 60  0001 C CNN "Reichelt"
F 5 "C17673" V 5500 5350 50  0001 C CNN "LCSC"
	1    5500 5350
	0    -1   -1   0   
$EndComp
Wire Wire Line
	4750 5350 4850 5350
Wire Wire Line
	5250 6150 4850 6150
$Comp
L Diode:BAT54C D201
U 1 1 601AB57E
P 4850 4350
F 0 "D201" V 4804 4438 50  0000 L CNN
F 1 "BAT54C" V 4895 4438 50  0000 L CNN
F 2 "Package_TO_SOT_SMD:SOT-23" H 4925 4475 50  0001 L CNN
F 3 "http://www.diodes.com/_files/datasheets/ds11005.pdf" H 4770 4350 50  0001 C CNN
F 4 "C556168" V 4850 4350 50  0001 C CNN "LCSC"
F 5 "BAT 54C NXP" V 4850 4350 50  0001 C CNN "Reichelt"
F 6 "583-BAT54C" V 4850 4350 50  0001 C CNN "Mouser"
	1    4850 4350
	0    -1   1    0   
$EndComp
$Comp
L Diode:BAT54C D202
U 1 1 601AC210
P 4850 5750
F 0 "D202" V 4804 5838 50  0000 L CNN
F 1 "BAT54C" V 4895 5838 50  0000 L CNN
F 2 "Package_TO_SOT_SMD:SOT-23" H 4925 5875 50  0001 L CNN
F 3 "http://www.diodes.com/_files/datasheets/ds11005.pdf" H 4770 5750 50  0001 C CNN
F 4 "C556168" V 4850 5750 50  0001 C CNN "LCSC"
F 5 "BAT 54C NXP" V 4850 5750 50  0001 C CNN "Reichelt"
F 6 "583-BAT54C" V 4850 5750 50  0001 C CNN "Mouser"
	1    4850 5750
	0    -1   1    0   
$EndComp
Wire Wire Line
	4850 3950 4850 4050
Connection ~ 4850 3950
Wire Wire Line
	4850 3950 5250 3950
Wire Wire Line
	4850 4650 4850 4750
Connection ~ 4850 4750
Wire Wire Line
	4850 4750 4750 4750
Wire Wire Line
	4850 5350 4850 5450
Connection ~ 4850 5350
Wire Wire Line
	4850 5350 5250 5350
Wire Wire Line
	4850 6050 4850 6150
Connection ~ 4850 6150
Wire Wire Line
	4850 6150 4750 6150
Wire Wire Line
	5050 4350 5150 4350
Wire Wire Line
	5150 4350 5150 5050
Wire Wire Line
	5050 5750 5150 5750
Wire Wire Line
	5150 5750 5150 5050
Connection ~ 5150 5050
Wire Wire Line
	5750 3950 5850 3950
Wire Wire Line
	5850 3950 5850 4050
Wire Wire Line
	5850 4650 5850 4750
Wire Wire Line
	5850 4750 5750 4750
Wire Wire Line
	5750 5350 5850 5350
Wire Wire Line
	5850 5350 5850 5450
Wire Wire Line
	5750 6150 5850 6150
Wire Wire Line
	5850 6150 5850 6050
$Comp
L wfred_rev6-rescue:C-RESCUE-wfred_rev2-wfred_rev2-rescue C202
U 1 1 601D2BF2
P 6950 5350
AR Path="/601D2BF2" Ref="C202"  Part="1" 
AR Path="/5920DD4A/601D2BF2" Ref="C202"  Part="1" 
F 0 "C202" H 6950 5450 40  0000 L CNN
F 1 "4u7" H 6956 5265 40  0000 L CNN
F 2 "Capacitor_SMD:C_0805_2012Metric_Pad1.15x1.40mm_HandSolder" H 6988 5200 30  0001 C CNN
F 3 "" H 6950 5350 60  0000 C CNN
F 4 "KEM X5R0805 4,7U" H 6950 5350 60  0001 C CNN "Reichelt"
F 5 "C1779" H 6950 5350 50  0001 C CNN "LCSC"
	1    6950 5350
	1    0    0    -1  
$EndComp
Wire Wire Line
	6950 5050 6950 5150
$Comp
L wfred_rev6-rescue:GND-power-wfred_rev2-rescue #PWR0210
U 1 1 601D7439
P 6950 5750
F 0 "#PWR0210" H 6950 5750 30  0001 C CNN
F 1 "GND" H 6950 5680 30  0001 C CNN
F 2 "" H 6950 5750 60  0001 C CNN
F 3 "" H 6950 5750 60  0001 C CNN
	1    6950 5750
	1    0    0    -1  
$EndComp
Wire Wire Line
	6950 5750 6950 5550
$Comp
L power:VCC #PWR0211
U 1 1 601DBAB5
P 7500 1050
F 0 "#PWR0211" H 7500 900 50  0001 C CNN
F 1 "VCC" H 7515 1223 50  0000 C CNN
F 2 "" H 7500 1050 50  0001 C CNN
F 3 "" H 7500 1050 50  0001 C CNN
	1    7500 1050
	1    0    0    -1  
$EndComp
$Comp
L power:VCC #PWR0212
U 1 1 601DC387
P 7800 1050
F 0 "#PWR0212" H 7800 900 50  0001 C CNN
F 1 "VCC" H 7815 1223 50  0000 C CNN
F 2 "" H 7800 1050 50  0001 C CNN
F 3 "" H 7800 1050 50  0001 C CNN
	1    7800 1050
	1    0    0    -1  
$EndComp
$Comp
L power:VCC #PWR0215
U 1 1 601DC4F0
P 8100 1050
F 0 "#PWR0215" H 8100 900 50  0001 C CNN
F 1 "VCC" H 8115 1223 50  0000 C CNN
F 2 "" H 8100 1050 50  0001 C CNN
F 3 "" H 8100 1050 50  0001 C CNN
	1    8100 1050
	1    0    0    -1  
$EndComp
Wire Wire Line
	7500 1050 7500 1250
Wire Wire Line
	7800 1050 7800 1250
Wire Wire Line
	8100 1050 8100 1250
Text GLabel 7200 2850 0    60   Input ~ 0
LED_REV
Wire Wire Line
	7200 2850 8100 2850
Wire Wire Line
	8100 2350 8100 2850
Text GLabel 6250 3950 2    50   Output ~ 0
LOCO1
Wire Wire Line
	5850 3950 6250 3950
Connection ~ 5850 3950
Text GLabel 6250 4750 2    50   Output ~ 0
LOCO2
Text GLabel 7150 5050 2    50   Output ~ 0
ESP_ENABLE
Text GLabel 6250 5350 2    50   Output ~ 0
LOCO3
Text GLabel 6150 6150 2    50   Output ~ 0
LOCO4
Wire Wire Line
	6150 6150 5850 6150
Connection ~ 5850 6150
Wire Wire Line
	5850 4750 6250 4750
Connection ~ 5850 4750
Wire Wire Line
	6950 5050 7150 5050
Connection ~ 6950 5050
Wire Wire Line
	6250 5350 5850 5350
Connection ~ 5850 5350
$Comp
L wfred_rev6-rescue:GND-RESCUE-wfred_rev2-wfred_rev2-rescue #PWR0201
U 1 1 60224CCC
P 1000 2750
AR Path="/60224CCC" Ref="#PWR0201"  Part="1" 
AR Path="/5920DD4A/60224CCC" Ref="#PWR0201"  Part="1" 
F 0 "#PWR0201" H 1000 2750 30  0001 C CNN
F 1 "GND" H 1000 2680 30  0001 C CNN
F 2 "" H 1000 2750 60  0001 C CNN
F 3 "" H 1000 2750 60  0001 C CNN
	1    1000 2750
	1    0    0    -1  
$EndComp
Wire Wire Line
	1000 2750 1000 2500
Wire Wire Line
	1000 2500 1200 2500
Wire Wire Line
	1000 2000 1200 2000
Wire Wire Line
	1000 2000 1000 2500
Connection ~ 1000 2500
Wire Wire Line
	1200 1000 1000 1000
Wire Wire Line
	1000 1000 1000 1500
Connection ~ 1000 2000
Wire Wire Line
	1200 1500 1000 1500
Connection ~ 1000 1500
Wire Wire Line
	1000 1500 1000 2000
$Comp
L wfred_rev6-rescue:GND-RESCUE-wfred_rev2-wfred_rev2-rescue #PWR0205
U 1 1 602370C4
P 2600 2750
AR Path="/602370C4" Ref="#PWR0205"  Part="1" 
AR Path="/5920DD4A/602370C4" Ref="#PWR0205"  Part="1" 
F 0 "#PWR0205" H 2600 2750 30  0001 C CNN
F 1 "GND" H 2600 2680 30  0001 C CNN
F 2 "" H 2600 2750 60  0001 C CNN
F 3 "" H 2600 2750 60  0001 C CNN
	1    2600 2750
	1    0    0    -1  
$EndComp
Wire Wire Line
	2600 2750 2600 2500
Wire Wire Line
	2600 2500 2800 2500
Wire Wire Line
	2600 2000 2800 2000
Wire Wire Line
	2600 2000 2600 2500
Connection ~ 2600 2500
Wire Wire Line
	2800 1000 2600 1000
Wire Wire Line
	2600 1000 2600 1500
Connection ~ 2600 2000
Wire Wire Line
	2800 1500 2600 1500
Connection ~ 2600 1500
Wire Wire Line
	2600 1500 2600 2000
$Comp
L wfred_rev6-rescue:GND-RESCUE-wfred_rev2-wfred_rev2-rescue #PWR0208
U 1 1 6023BEFE
P 4200 2750
AR Path="/6023BEFE" Ref="#PWR0208"  Part="1" 
AR Path="/5920DD4A/6023BEFE" Ref="#PWR0208"  Part="1" 
F 0 "#PWR0208" H 4200 2750 30  0001 C CNN
F 1 "GND" H 4200 2680 30  0001 C CNN
F 2 "" H 4200 2750 60  0001 C CNN
F 3 "" H 4200 2750 60  0001 C CNN
	1    4200 2750
	1    0    0    -1  
$EndComp
Wire Wire Line
	4200 2750 4200 2500
Wire Wire Line
	4200 2000 4400 2000
Connection ~ 4200 2000
Wire Wire Line
	4400 1000 4200 1000
Wire Wire Line
	4200 1000 4200 1500
Wire Wire Line
	4400 1500 4200 1500
Connection ~ 4200 1500
Wire Wire Line
	4200 1500 4200 2000
Text GLabel 2100 1000 2    60   Output ~ 0
F1
Text GLabel 3700 1500 2    60   Output ~ 0
F2
Text GLabel 2100 1500 2    60   Output ~ 0
F4
Text GLabel 2100 2000 2    60   Output ~ 0
F7
Text GLabel 2100 2500 2    60   Output ~ 0
ESTOP
Text GLabel 3700 1000 2    60   Output ~ 0
F0
Text GLabel 3700 2000 2    60   Output ~ 0
F5
Text GLabel 3700 2500 2    60   Output ~ 0
F8
Wire Wire Line
	5000 2000 5300 2000
Text GLabel 5800 2600 2    60   Output ~ 0
REV
Connection ~ 4200 2500
Wire Wire Line
	4200 2500 4200 2000
Wire Wire Line
	7950 4350 7950 5450
$Comp
L wfred_rev6-rescue:R-RESCUE-wfred_rev2-wfred_rev2-rescue R210
U 1 1 602A4EBA
P 8250 5000
F 0 "R210" V 8330 5000 40  0000 C CNN
F 1 "15k" V 8257 5001 40  0000 C CNN
F 2 "Resistor_SMD:R_0805_2012Metric_Pad1.15x1.40mm_HandSolder" V 8180 5000 30  0001 C CNN
F 3 "" H 8250 5000 30  0000 C CNN
F 4 "SMD-0805 15,0K" V 8250 5000 60  0001 C CNN "Reichelt"
F 5 "Do not populate with CAT4002A" V 8250 5000 50  0001 C CNN "Remark"
	1    8250 5000
	1    0    0    -1  
$EndComp
Wire Wire Line
	8250 4550 8250 4750
Connection ~ 8250 4550
Wire Wire Line
	8250 4550 8850 4550
Wire Wire Line
	8250 5250 8250 5450
Wire Wire Line
	8250 5450 7950 5450
Connection ~ 7950 5450
Wire Wire Line
	7950 5450 7950 5750
Wire Wire Line
	8250 5450 8650 5450
Wire Wire Line
	8650 5450 8650 4750
Wire Wire Line
	8650 4750 8850 4750
Connection ~ 8250 5450
$Comp
L wfred_rev6-rescue:R-RESCUE-wfred_rev2-wfred_rev2-rescue R208
U 1 1 602F35C2
P 7800 1500
F 0 "R208" V 7880 1500 40  0000 C CNN
F 1 "680R" V 7807 1501 40  0000 C CNN
F 2 "Resistor_SMD:R_0805_2012Metric_Pad1.15x1.40mm_HandSolder" V 7730 1500 30  0001 C CNN
F 3 "" H 7800 1500 30  0000 C CNN
F 4 "SMD-0805 680" V 7800 1500 60  0001 C CNN "Reichelt"
F 5 "C17798" V 7800 1500 50  0001 C CNN "LCSC"
	1    7800 1500
	-1   0    0    1   
$EndComp
$Comp
L wfred_rev6-rescue:R-RESCUE-wfred_rev2-wfred_rev2-rescue R209
U 1 1 602F39C3
P 8100 1500
F 0 "R209" V 8180 1500 40  0000 C CNN
F 1 "680R" V 8107 1501 40  0000 C CNN
F 2 "Resistor_SMD:R_0805_2012Metric_Pad1.15x1.40mm_HandSolder" V 8030 1500 30  0001 C CNN
F 3 "" H 8100 1500 30  0000 C CNN
F 4 "SMD-0805 680" V 8100 1500 60  0001 C CNN "Reichelt"
F 5 "C17798" V 8100 1500 50  0001 C CNN "LCSC"
	1    8100 1500
	-1   0    0    1   
$EndComp
$Comp
L wfred_rev6-rescue:R-RESCUE-wfred_rev2-wfred_rev2-rescue R212
U 1 1 601789F9
P 6650 5400
F 0 "R212" V 6730 5400 40  0000 C CNN
F 1 "4M7" V 6657 5401 40  0000 C CNN
F 2 "Resistor_SMD:R_0805_2012Metric_Pad1.15x1.40mm_HandSolder" V 6580 5400 30  0001 C CNN
F 3 "" H 6650 5400 30  0000 C CNN
F 4 "SMD-0805 4,70M" V 6650 5400 60  0001 C CNN "Reichelt"
F 5 "C17674" V 6650 5400 50  0001 C CNN "LCSC"
	1    6650 5400
	1    0    0    -1  
$EndComp
Wire Wire Line
	6650 5050 6650 5150
Wire Wire Line
	5150 5050 6650 5050
Connection ~ 6650 5050
Wire Wire Line
	6650 5050 6950 5050
$Comp
L wfred_rev6-rescue:GND-power-wfred_rev2-rescue #PWR0124
U 1 1 6017E2A9
P 6650 5750
F 0 "#PWR0124" H 6650 5750 30  0001 C CNN
F 1 "GND" H 6650 5680 30  0001 C CNN
F 2 "" H 6650 5750 60  0001 C CNN
F 3 "" H 6650 5750 60  0001 C CNN
	1    6650 5750
	1    0    0    -1  
$EndComp
Wire Wire Line
	6650 5750 6650 5650
$Comp
L wfred_rev6-rescue:C-RESCUE-wfred_rev2-wfred_rev2-rescue C?
U 1 1 60C635A2
P 2000 5650
AR Path="/60C635A2" Ref="C?"  Part="1" 
AR Path="/5920DD4A/60C635A2" Ref="C201"  Part="1" 
F 0 "C201" H 2000 5750 40  0000 L CNN
F 1 "4u7" H 2006 5565 40  0000 L CNN
F 2 "Capacitor_SMD:C_0805_2012Metric_Pad1.15x1.40mm_HandSolder" H 2038 5500 30  0001 C CNN
F 3 "" H 2000 5650 60  0000 C CNN
F 4 "KEM X5R0805 4,7U" H 2000 5650 60  0001 C CNN "Reichelt"
F 5 "C1779" H 2000 5650 50  0001 C CNN "LCSC"
	1    2000 5650
	1    0    0    -1  
$EndComp
$EndSCHEMATC