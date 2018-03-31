#include <stdio.h>
#include <cstring>
#include <errno.h>
#include <string.h>
#include <unistd.h>
#include <wiringPi.h>
#include <softPwm.h>
#include <stdlib.h>
#include <sys/socket.h>
#include <arpa/inet.h>
#include<pthread.h> 
#include "MRPII.h"
#include <iostream>
using namespace std;
typedef unsigned char uint8_t;
#define _BV(bit) (1 << (bit))

#define TRIG 22
#define ECHO 21
#define MOTORLATCH  0
#define MOTORCLK    1
#define MOTORENABLE 2
#define MOTORDATA   3
#define MOTOR_1_PWM 4
#define MOTOR_2_PWM 5
#define MOTOR_3_PWM 6
#define MOTOR_4_PWM 7

#define MOTOR1_A 2
#define MOTOR1_B 3
#define MOTOR2_A 1
#define MOTOR2_B 4
#define MOTOR4_A 0
#define MOTOR4_B 6
#define MOTOR3_A 5
#define MOTOR3_B 7

#define FORWARD  1
#define BACKWARD 2
#define BRAKE    3
#define RELEASE  4

static unsigned char latch_state;

void latch_tx(void)
{
   unsigned char i;
   
      digitalWrite(MOTORLATCH, LOW);

   digitalWrite(MOTORDATA, LOW);

   for (i=0; i<8; i++)
   {
      digitalWrite(MOTORCLK, LOW);

      if (latch_state & _BV(7-i))
      {
         digitalWrite(MOTORDATA, HIGH);
      }
      else
      {
         digitalWrite(MOTORDATA, LOW);
      }
      digitalWrite(MOTORCLK, HIGH);
   }
   digitalWrite(MOTORLATCH, HIGH);
}

void enable(void)
{
   wiringPiSetup();
   pinMode(TRIG, OUTPUT);
   pinMode(ECHO, INPUT);
   pinMode(MOTORLATCH,  OUTPUT);
   pinMode(MOTORENABLE, OUTPUT);
   pinMode(MOTORDATA,   OUTPUT);
   pinMode(MOTORCLK,    OUTPUT);

   //TRIG pin must start LOW
   digitalWrite(TRIG, LOW);
   delay(20);
   latch_state = 0;

   latch_tx();

   digitalWrite(MOTORENABLE, LOW);
}

void DCMotorInit(uint8_t num)
{
   switch (num)
   {
      case 1:
         latch_state &= ~_BV(MOTOR1_A) & ~_BV(MOTOR1_B);
         latch_tx();
         break;
      case 2:
         latch_state &= ~_BV(MOTOR2_A) & ~_BV(MOTOR2_B);
         latch_tx();
         break;
      case 3:
         latch_state &= ~_BV(MOTOR3_A) & ~_BV(MOTOR3_B);
         latch_tx();
         break;
      case 4:
         latch_state &= ~_BV(MOTOR4_A) & ~_BV(MOTOR4_B);
         latch_tx();
         break;
   }
}

void DCMotorRun(uint8_t motornum, uint8_t cmd)
{
   uint8_t a, b;

   switch (motornum)
   {
      case 1:
         a = MOTOR1_A; b = MOTOR1_B;
         break;
      case 2:
         a = MOTOR2_A; b = MOTOR2_B;
         break;
      case 3:
         a = MOTOR3_A; b = MOTOR3_B;
         break;
      case 4:
         a = MOTOR4_A; b = MOTOR4_B;
         break;
	  default:
         return;
   }

   switch (cmd)
   {
      case FORWARD:
         latch_state |= _BV(a);
         latch_state &= ~_BV(b);
         latch_tx();
         break;
      case BACKWARD:
         latch_state &= ~_BV(a);
         latch_state |= _BV(b);
         latch_tx();
         break;
      case RELEASE:
         latch_state &= ~_BV(a);
         latch_state &= ~_BV(b);
         latch_tx();
       break;
   }
}
class move 
{
   //class for controlling DC motors
   public:
	
	void left() 
	{
   	   DCMotorRun(2, BACKWARD);
   	   digitalWrite(MOTOR_2_PWM, 1);
  	   DCMotorRun(3, FORWARD);
   	   digitalWrite(MOTOR_3_PWM, 1);
  	   DCMotorRun(1, BACKWARD);
   	   digitalWrite(MOTOR_1_PWM, 1);
   	   DCMotorRun(4, FORWARD);
   	   digitalWrite(MOTOR_4_PWM, 1);
	}
	void right() 
	{
  	   DCMotorRun(1, FORWARD);
	   digitalWrite(MOTOR_1_PWM, 1);
	   DCMotorRun(2, FORWARD);
	   digitalWrite(MOTOR_2_PWM, 1);
	   DCMotorRun(3, BACKWARD);
	   digitalWrite(MOTOR_3_PWM, 1);
	   DCMotorRun(4, BACKWARD);
	   digitalWrite(MOTOR_4_PWM, 1);
	}
	void backward() 
	{
	   DCMotorRun(1, FORWARD);
	   digitalWrite(MOTOR_1_PWM, 1);
	   DCMotorRun(2, BACKWARD);
	   digitalWrite(MOTOR_2_PWM, 1);
	   DCMotorRun(3, FORWARD);
	   digitalWrite(MOTOR_3_PWM, 1);
	   DCMotorRun(4, BACKWARD);
	   digitalWrite(MOTOR_4_PWM, 1);
	} 
	void forward() {
	   DCMotorRun(1, BACKWARD);
	   digitalWrite(MOTOR_1_PWM, 1);
	   DCMotorRun(2, FORWARD);
	   digitalWrite(MOTOR_2_PWM, 1);
	   DCMotorRun(3, BACKWARD);
	   digitalWrite(MOTOR_3_PWM, 1);
	   DCMotorRun(4, FORWARD);
	   digitalWrite(MOTOR_4_PWM, 1);
	}

	void stop() 
	{
	   DCMotorRun(1, RELEASE);
	   DCMotorRun(2, RELEASE);
	   DCMotorRun(3, RELEASE);
	   DCMotorRun(4, RELEASE);
	}
};


int getCM() 
{
   //delay(200);
   //Send trig pulse
   digitalWrite(TRIG, HIGH);
   delayMicroseconds(60);
   digitalWrite(TRIG, LOW);
   //Wait for echo start
   while(digitalRead(ECHO) == LOW);
   //Wait for echo end
	   long startTime = micros();
   while(digitalRead(ECHO) == HIGH);
	   long travelTime = micros() - startTime;
   //Get distance in cm
	   int distance = travelTime / 58;
   // printf("Distance: %dcm\n", distance);
   return distance;
}

void *connection_handler(void *socket_desc)
{
    //Get the socket descriptor
    int sock = *(int*)socket_desc;
    int read_size;
    char *message , client_message[2000];
    move mrpi;
     
       
    //Receive a message from client
    while( (read_size = recv(sock , client_message , 2000 , 0)) > 0 )
    {
        //end of string marker
		client_message[read_size] = '\0';
		
		if (strcmp(client_message,"forward") == 0)
			mrpi.forward();
		else if (strcmp(client_message,"left") == 0)
			mrpi.left();
		else if (strcmp(client_message,"right") == 0)
			mrpi.right();
		else if (strcmp(client_message,"backward") == 0)
			mrpi.backward();
		else if (strcmp(client_message,"stop") == 0)
			mrpi.stop();
		else if (strcmp(client_message,"sonar") == 0) {
			sprintf(message, "%d",getCM());
			write(sock, message, strlen(message));
			}
		else 
			cout<<client_message<<'\n';
		
		
		//clear the message buffer
		memset(client_message, 0, 2000);
    }
     
    if(read_size == 0)
    {
        puts("Client disconnected");
        fflush(stdout);
    }
    else if(read_size == -1)
    {
        perror("recv failed");
    }
	return 0;
} 








int main(int argc, char *argv[])
{	
    if (wiringPiSetup () == -1)
   {
      fprintf (stdout, "oops: %s\n", strerror (errno)) ;
      return 1 ;
   }
   pinMode(MOTOR_1_PWM, OUTPUT);
   pinMode(MOTOR_2_PWM, OUTPUT);
   pinMode(MOTOR_3_PWM, OUTPUT);
   pinMode(MOTOR_4_PWM, OUTPUT);

   digitalWrite(MOTOR_1_PWM, 0);
   digitalWrite(MOTOR_2_PWM, 0);
   digitalWrite(MOTOR_3_PWM, 0);
   digitalWrite(MOTOR_4_PWM, 0);


   enable();

   DCMotorInit(1);
   DCMotorInit(2);
   DCMotorInit(3);
   DCMotorInit(4);

int socket_desc , client_sock , c;
    struct sockaddr_in server , client;
     
    //Create socket
    socket_desc = socket(AF_INET , SOCK_STREAM , 0);
    if (socket_desc == -1)
    {
        printf("Could not create socket");
    }
    puts("Socket created");
     
    //Prepare the sockaddr_in structure
    server.sin_family = AF_INET;
    server.sin_addr.s_addr = inet_addr("192.168.1.103");
    server.sin_port = htons( 1080 );
     
    //Bind
    if( bind(socket_desc,(struct sockaddr *)&server , sizeof(server)) < 0)
    {
        //print the error message
        perror("bind failed. Error");
        return 1;
    }
    puts("bind done");
     
    //Listen
    listen(socket_desc , 3);
     
    //Accept and incoming connection
    puts("Waiting for incoming connections...");
    c = sizeof(struct sockaddr_in);
     
     
    //Accept and incoming connection
    puts("Waiting for incoming connections...");
    c = sizeof(struct sockaddr_in);
	pthread_t thread_id;
	
    while( (client_sock = accept(socket_desc, (struct sockaddr *)&client, (socklen_t*)&c)) )
    {
        puts("Connection accepted");
         
        if( pthread_create( &thread_id , NULL ,  connection_handler , (void*) &client_sock) < 0)
        {
            perror("could not create thread");
            return 1;
        }
         
        //Now join the thread , so that we dont terminate before the thread
        //pthread_join( thread_id , NULL);
        puts("Handler assigned");
    }
     
    if (client_sock < 0)
    {
        perror("accept failed");
        return 1;
    }
 
return 0;
}
