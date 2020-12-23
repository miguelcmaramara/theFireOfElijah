#include <Arduino.h>
#include <Pixy2.h>

// This is the main Pixy object 
Pixy2 pixy;

unsigned long t0 = millis();
unsigned long deltaT;

int count = 0;
int i; 


void setup()
{
  Serial.begin(9600);
  Serial.print("Starting...\n");
  
  pixy.init();
}

void loop()
{ 
  // grab blocks!

  t0 = millis();
  count = 0;
  // find out how many!
  do{
    pixy.ccc.getBlocks();
    for(i = 0; i < pixy.ccc.numBlocks; i++){
      if(pixy.ccc.blocks[i].m_signature == 2){
        count++;
      }
    }
  } while(pixy.ccc.numBlocks);

  if(count > 0){
    deltaT = millis() - t0;
    Serial.print(count);
    Serial.print(" readings in ");
    Serial.print((double)deltaT / (double)1000);
    Serial.print(" seconds\n");
    Serial.print((double)count / (double)deltaT * (double)1000);
    Serial.print(" readings per second\n");
  }

}