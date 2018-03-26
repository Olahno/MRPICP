#ifndef MRPII_H_
#define MRPII_H_
#include <string>
typedef unsigned char uint8_t;

int getCM();//activate sonar

void latch_tx(void);

void enable(); //enable motors;

void DCMotorInit(uint8_t num);

void DCMotorRun(uint8_t motornum, uint8_t cmd);

void left(unsigned int);//turn for set amout of seconds(unsigned int)

void lleft();//turn untill stop()

void right(unsigned int);

void rright();

void backwd(unsigned int);

void bbackwd();

void forwd(unsigned int);

void fforwd();

void stop();//stopping motors

#endif // MRPII_H_
