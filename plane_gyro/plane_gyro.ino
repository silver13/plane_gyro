//////////GYRO VARIABLES


#include <Servo.h> 
#include <util/atomic.h>

#include <Arduino.h>


Servo servo;

byte sda=1;
byte scl=1;
float potfilt;

volatile long pulsetime;

const int gyrodebug = 1;

const int sdapin = 6;
const int sclpin = 7;
const int gyrognd = 8;  // zero disables
const int gyrovcc = 9;  // zero disables


const int servomin = 1000;
const int servomax = 2000;

const int servopin = 11;

const byte inpin = 0; 

const byte potin = A1;
const byte potgnd = A0;
const byte potvcc = A2;

const float gainmin = 0.0;
const float gainmax = 0.05;

byte pulsecount;
byte started = 0;
//////////

void setup()
{
  
  Serial.begin(57600);
  Serial.println("xxx Test");
  
   
 pinMode(sdapin,INPUT);  //sda
 pinMode(sclpin,INPUT);  //scl
 
if (gyrognd) pinMode(gyrognd,OUTPUT);  //gnd
if (gyrovcc) pinMode(gyrovcc,OUTPUT);  //vcc
 
if (gyrognd) digitalWrite(gyrognd,0); //gnd
if (gyrovcc) digitalWrite(gyrovcc,1); //vcc
 
 pinMode(sdapin,OUTPUT);
 digitalWrite(sdapin,0);
 pinMode(sdapin,INPUT);
 
 pinMode(sclpin,OUTPUT);
 digitalWrite(sclpin,0);
 pinMode(sclpin,INPUT);
 
if (potgnd) pinMode(potgnd,OUTPUT);  //vcc 
if (potgnd) digitalWrite(potgnd,0); //gnd 
  
if (potvcc) pinMode(potvcc,OUTPUT);  //vcc 
if (potvcc) digitalWrite(potvcc,1); //gnd 
   
 
 Serial.begin(57600); 

////////////////GYRO SETUP
 
delay(100);
//small delay needed (10?) for powerup  
writeimu(107, B0000000);   //standby off

writeimu(26, B00000110);  //filter 5 hz /(1 khz sampling)
                          // n , n, n , n , n , x2 , x1 ,x0 //
                          // 0 = 256 hz, 8khz sample rate
                          // 1 = 188 hz, 1khz, 2 = 98Hz
                          // 3 = 42 Hz 4mS,     4 = 20 Hz 8mS
                          // 5 = 10 Hz 13 mS,     6 = 5 Hz 18mS delay
writeimu(27, B00001000);   //self test, 500 deg/s
                           //x st , y st , z st , rate 1 , rate 2, nc , nc , nc 
                           
//writeimu(27, B00101000);   //self test, 500 deg/s
                           //x st , y st , z st , rate 1 , rate 2, nc , nc , nc 
writeimu(28, B00000100);   //self test,accel fs, HPF | st st st x x NC NC NC xx= 0 -> 3 2 ->16g

writeimu(33, B00000110);  //zero motion detect treshhold

writeimu(34, B00000001);  //zero motion duration 64mS = LSB

if (readimu( 117) == 104) Serial.println("IMU Found");

delay(250);  //200 ms or 1 - 2 values slightly wrong


//writeimu(27, B00001000);   //self test off, 500 deg/s
                           //x st , y st , z st , rate 1 , rate 2, nc , nc , nc 
                          
//////// 

  servo.attach( servopin , servomin , servomax);
   
  servo.write(90);
  
  Serial.println(servo.readMicroseconds());

pinMode( inpin+2, INPUT_PULLUP);

    attachInterrupt( inpin, interrupt1rise, RISING);
  
}

////////////////////////////////
/////////Gyro Routines//////////

void sdalow()
{
  pinMode(sdapin,OUTPUT);
  digitalWrite(sdapin,0);
  sda=0;
}


void sdahigh()
{
   pinMode(sdapin,INPUT);
   sda =1; 
}


void scllow()
{
 pinMode(sclpin,OUTPUT);
 digitalWrite(sclpin,0);
 scl = 0;
}

void sclhigh()
{
 pinMode(sclpin,INPUT);
 scl = 1; 
}

void sendstart()
{
 if (scl == 0) 
 {
  if (gyrodebug) Serial.println("sendstart: scl low"); 
  sclhigh();
 } 
 if (sda == 1)
{
  if (gyrodebug) if(!readsda()) Serial.println("sendstart: sda pulled low by slave"); 
  sdalow();
}
 else {
      if (gyrodebug) Serial.println("sendstart: sda low"); 
       } 
}


void restart()
{ 
 if (gyrodebug) if (scl == 1) Serial.println("restart: scl high"); 
 if (sda == 0) 
 {
   sdahigh();
 }
 sclhigh();
 sdalow();
}

void sendstop()
{
  
  if (sda == 1) 
  {
    if (!scl) sdalow(); else if (gyrodebug) Serial.println("stop: error");
  }
  if (scl == 0) sclhigh();
  else if (gyrodebug) Serial.println("stop: scl high");
  sdahigh();
 
}



int readsda()
{
 if (!sda)  if (gyrodebug) Serial.println("readsda: sda low");
return digitalRead(sdapin); 
}



int sendbyte( int value )
{
 if (scl == 1) 
 {
  scllow();
 }
 
 for (int i = 7; i >= 0 ;i--)
 {
 if (bitRead(value,i) == 1) 
 {
 if (!sda) sdahigh();
 }
 else 
 {
 if (sda) sdalow();
 }
 
 sclhigh();
 scllow();

 }
 if (!sda) sdahigh(); // release the line
 //get ack

 sclhigh();

 int ack = readsda();

  if (!ack) ;//Serial.println("ACK = RECEIVED");
  else if (gyrodebug) Serial.println("NOT RECEIVED"); 
 scllow();
return ack; 
}

int readbyte(int ack)  //ACK 1 single byte ACK 0 multiple bytes
{
 int data=0;
 if (scl == 1) if (gyrodebug) Serial.println("read: scl high");
 if (sda == 0) 
 {
   sdahigh();
 }
 for( int i = 7; i>=0;i--)
 {
  sclhigh(); 
 if (readsda() ) bitSet(data,i);
  scllow();

 }
 
 // ack

if (ack)  
{
  sdahigh();
} 
else 
{
  sdalow();
}

  sclhigh();

  scllow();

if (sda) sdalow(); 
//Serial.println(data);
return data;
}

void writeimu( int address,byte value)  // 690 uS
{
 sendstart();
 sendbyte(208); 
 sendbyte(address);
 sendbyte(value);
 sendstop();
}

byte readimu(int address_gyro)   // 910 uS
{
 sendstart();
 sendbyte(208);
 sendbyte(address_gyro);
 restart(); 
 sendbyte(209);
 byte x = readbyte(1); 
 sendstop();
 return x; 
}

int readimu2(int address_gyro)   // 1140 uS
{
 sendstart();
 sendbyte(208);
 sendbyte(address_gyro);
 restart();
 sendbyte(209);
 int xh = readbyte(0); 
 int xl = readbyte(1);
 sendstop();
 int x = xh<<8;
 x=x | xl;
 return x; 
}

///////////////////////////////END GYRO I2C///////


void loop()
{

// GYRO  
long servoin = 1500;
int servoout= 1500;
float gyro;
float gyrogain = 0.1;

if(started)
{
ATOMIC_BLOCK(ATOMIC_RESTORESTATE) 
{
 servoin = pulsetime; 
}
}

if ( !started && pulsecount > 100) started = 1;

//servoin = constrain( servoin/2 , 1000 , 2000 );
float potread = analogRead ( potin);

potfilt = potfilt*0.96 + 0.04*potread;

gyrogain = mapf( potfilt , 0 , 1023 , gainmin , gainmax );

unsigned long timeloop = micros();

//long xg = readimu2(71); //z axis gyro
long xg = readimu2(67); //z axis gyro
  
gyro =  gyrogain * xg;

Serial.print(" in:");
Serial.print(servoin);

Serial.print(" s:");
Serial.print(started);

Serial.print(" p:");
Serial.print(potfilt);

Serial.print(" gain:");
Serial.print(gyrogain);

Serial.print(" g:");
Serial.print(gyro);
 
/*
Serial.print(" Zero motion:"); 
Serial.print(readimu(97));
*/

servoout = servoin;
servoout+= gyro;

constrain( servoout , servomin , servomax);

servo.writeMicroseconds( servoout);

Serial.print(" ");
Serial.println( micros() - timeloop);

Serial.println();

while( micros() - timeloop < 20000);

}

volatile unsigned long time1start;

void interrupt1rise( void)
{
 time1start = micros();
 pulsecount++;
 attachInterrupt( inpin, interrupt1fall, FALLING);  
}


void interrupt1fall( void)
{
 
 unsigned long time = micros() - time1start; 
 pulsetime = time;  
 attachInterrupt( inpin, interrupt1rise, RISING); 

}


float mapf(float x, long in_min, long in_max, float out_min, float out_max)
{

return float((x - in_min) * (out_max - out_min)) / (in_max - in_min) + out_min;

}

