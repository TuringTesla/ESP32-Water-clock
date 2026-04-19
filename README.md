An innovative (and slightly evil) alarm clock powered by an ESP32 that literally wakes you up by spraying water on your face when the alarm goes off.

On startup, the ESP32:
>Connects to WiFi
>Fetches current IST time from NTP servers
The system:
>Retrieves stored alarm time from Blynk
>Displays time and status on LCD
User can:
>Set alarm time via Serial Monitor
>Alarm time is saved to Blynk cloud

When time matches alarm:
>💦 Water pump activates
>Sprays water through pipe
User can stop alarm:
>Using push button (debounced)

🧰 Components Used

>ESP32	1
>DC Water Pump	1
>Water Pipe	1
>Push Button	1
>10kΩ Resistor	1
>16x2 LCD (I2C)	1
>Breadboard	1
>Jumper Wires	As needed
