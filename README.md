# plane_gyro
simple gyro for fixed wing, single channel

This was a quickly cobbled up gyro from parts laying around, I use it to provide wind resistance on small planes. It can be improved in many ways and might be in the future.  Weight approx 5g. __The gyro is not pass-thru if not powered / malfunctioning!__

Requirements: 

* arduino 328 based ( mini pro used )
* mpu 6050 gyro
* potentiometer for gain ( 1k - 10k)
* power 3.3 - 5v

# Connections:

The gyro can connect to any arduino pins for convenience. Power pins are used so the board connections are lined up, they can be ignored

```arduino
const int sdapin = 6;
const int sclpin = 7;
const int gyrognd = 8;  // zero disables
const int gyrovcc = 9;  // zero disables
```

The servo output pin can be any , set to 11

```arduino
const int servopin = 11;
```

The servo input pin can be only A2 ( default)  or A3 as it uses interrupts

The potentiometer can be also any pin, the gain limits can also be set

```arduino
const byte potin = A1;
const byte potgnd = A0; // set to ground
const byte potvcc = A2; // set to vcc

const float gainmin = 0.0;
const float gainmax = 0.05;
```


![alt text](https://github.com/silver13/plane_gyro/blob/master/img/gyro_arduino.jpg "img")
