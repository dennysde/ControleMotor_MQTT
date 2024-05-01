# Motor Control Via MQTT

## Brief

This project receives the motor’s spin direction and speed from the mqtt broker on subscription dennys/motor -> [EMQX](http://www.emqx.io/online-mqtt-client#/recent_connections/148bd9f2-fdd0-45e8-9e39-d7640543fddb)
-	If letter “h” or “H” is received. The motor will spin clockwise.
-	If letter “a” or “A” is received. The motor will spin counterclockwise.
-	If an integer number from 0 to 100 is received. The motor’s speed will change accordingly. 

The result is recorded at [Youtube](https://youtu.be/Ftu7ZLE-D48)

## Bill of Material - BOM

-	12V DC motor
-	H-bridge L298N module
-	Raspberry PICO W board
-	5-12V DC Power Supply 

## Fritzing schematics
![image](https://github.com/dennysde/ControleMotor_MQTT/assets/57273197/905ad6b4-ab86-4c4f-9b92-966085e7aa09)

## Assembly
![image](https://github.com/dennysde/ControleMotor_MQTT/assets/57273197/3c8f98f3-c984-4ce5-a19b-6f86cfd066c3)

## Messages sent on EMQX broker
![image](https://github.com/dennysde/ControleMotor_MQTT/assets/57273197/49b08df1-6645-41de-9ac9-216be127ad45)
