
#include <TimerOne.h>

#include "crc16.h"

typedef unsigned int uint;
typedef unsigned char uint8;

unsigned long previousMillis = 0; // will store last time LED was updated

uint8 recvbuffer[16];
int left_speed = 0; 
int right_speed = 0; 
uint8 direction = 0; 

// constants won't change:
const long interval = 1000; // interval at which to blink (milliseconds)

#define MAX_SPEED 128
#define MIN_SPEED 50
#define RIGHT_MOTOR_FORWARD 3//OCR2B
#define RIGHT_MOTOR_REVERSE 9//OCR1B
#define LEFT_MOTOR_FORWARD 11//OCR2A
#define LEFT_MOTOR_REVERSE 10//OCR1A


void connect()
{

  Serial.write("AT+CIPSTART=\"UDP\",\"255.255.255.255\",8080,9000,0\r\n");
  delay(1000);
  //printmessage();

  Serial.write("AT+CIPMODE=1\r\n");
  delay(1000);
  //printmessage();

  Serial.write("AT+CIPSEND\r\n");
  delay(1000);
  //printmessage();
}
void inline adjust_pwm(){
  static int ls=0,rs=0;
    ls=left_speed+MIN_SPEED;
    rs=right_speed+MIN_SPEED;
    if (ls<MIN_SPEED){
            ls=MIN_SPEED;
    }
    if (rs<MIN_SPEED){
            rs=MIN_SPEED;
    }
    if (ls>MAX_SPEED){
            ls=MAX_SPEED;
    }
    if (rs>MAX_SPEED){
            rs=MAX_SPEED;
    }
    
    if(left_speed==0){
      analogWrite(LEFT_MOTOR_FORWARD,0);
      analogWrite(LEFT_MOTOR_REVERSE,0);
    }else if(left_speed<0){
      analogWrite(LEFT_MOTOR_FORWARD,0);
      Timer1.pwm(LEFT_MOTOR_REVERSE,ls*4);
    }else{
      analogWrite(LEFT_MOTOR_FORWARD,ls);
      analogWrite(LEFT_MOTOR_REVERSE,0);
    }

    if(right_speed==0){
      analogWrite(RIGHT_MOTOR_FORWARD,0);
      analogWrite(RIGHT_MOTOR_REVERSE,0);
    }else if(right_speed<0){
      analogWrite(RIGHT_MOTOR_FORWARD,0);
      Timer1.pwm(RIGHT_MOTOR_REVERSE,rs*4);
    }else{    
      analogWrite(RIGHT_MOTOR_FORWARD,rs);
      analogWrite(RIGHT_MOTOR_REVERSE,0);
    }



}

int  adjust_direction(unsigned char b){
    static uint8 index = 0;
    static uint8 datalen = 0;

    recvbuffer[index]=b;

    switch(index){
      case 0:
          if (recvbuffer[0] != '$'){
            index=0;
            return -1;
          }
        
        break;
      case 1:
          if (recvbuffer[1] != 'M'){
            index=0;
            return -1;
          }
          break;
      case 2:
          if (recvbuffer[2] != '<'){
            index=0;
            return -1;
          }
          break;
      case 3:
          datalen=recvbuffer[3]+2; //crc16
          break;
      default:
        --datalen;
        if(datalen==0){

            uint8 *data=&recvbuffer[3];
            unsigned short crc=CRC16(data,recvbuffer[3]+1);

            if(crc != (data[recvbuffer[3]+1]|data[recvbuffer[3]+2]<<8)){
                
                return -2;
            }else{

                 switch(data[1]){
                   case 0x55: //设置速度
                      if(data[0]==5){
                        left_speed=data[2]|data[3]<<8;
                        right_speed=data[4]|data[5]<<8;
                        adjust_pwm();
                      }else{
                        index=0;
                        return -3;
                      }
                    break;
                   default:

                    break;
                 }

            }

            index=0;
            return 1;
        }
      
    }
    index++;
    return 0;
}


void setup_timer2(){

TCCR2B = (TCCR2B & 0b11111000) | 0x02;

}





void set_speed(){

}

void setup()
{
  // Open serial communications and wait for port to open:
  //Serial.begin(115200);
  Timer1.initialize(250);
  setup_timer2();
  Serial.begin(38400);
  pinMode(3, OUTPUT); 
  pinMode(11, OUTPUT); 
  



  pinMode(10,OUTPUT);
  pinMode(9,OUTPUT);
  delay(10000);
  connect();
  Serial.println("Goodnight moon!");

  //Timer1.pwm(9, 512);
  //Timer1.pwm(10, 512);
  //delay(1000);

}




void loop()
{ // run over and over
  unsigned long currentMillis = millis();
  while(Serial.available()){
      uint8 b=Serial.read();
      adjust_direction(b);

  }
  if (currentMillis - previousMillis >= interval)
  {

    // save the last time you blinked the LED

    previousMillis = currentMillis;
    Serial.println(left_speed);
    Serial.println(right_speed);
  }
}