 /*
 * Library to decode and send signals for 433MHz Nexa RC plugs
 * (c) Cedric Millard 2019
 */
#include "NexaTransmitter.h"

NexaTransmitter::NexaTransmitter(int iTxPin, unsigned long iTransmitterCode)
{
  transmitterCode = iTransmitterCode;
  txPin = iTxPin;
  pinMode(iTxPin, OUTPUT);
}

void NexaTransmitter::SendMessage(NexaCode iCode)
{
  //1st build the 64bits 
  unsigned long long lCodedMsg = EncodeMessage(iCode);
  //on envoie le message 5 fois de suite
  for(int tentative=0; tentative<3;tentative++)
  {
    digitalWrite(txPin, HIGH);
    delayMicroseconds(T);
    digitalWrite(txPin, LOW);
    delayMicroseconds(TS);
    for (int i=63;i>=0;i--)
    {
      digitalWrite(txPin, HIGH);
      delayMicroseconds(T);
      digitalWrite(txPin, LOW);
      if(lCodedMsg>>i & 0b1) {
        delayMicroseconds(T1);
      }
      else {
        delayMicroseconds(T0);
      }
    }
    digitalWrite(txPin, HIGH);
    delayMicroseconds(T);
    digitalWrite(txPin, LOW);
    delayMicroseconds(TP);
  }
}

unsigned long long NexaTransmitter::EncodeMessage(NexaCode iCode)
{
    unsigned long lMessage =0;
    unsigned long long lMessageManchester = 0;

    lMessage |= iCode.TransmitterNumber;
    lMessage <<=1;
    lMessage |= !iCode.GroupCode;
    lMessage <<=1;
    lMessage |= !iCode.Status;
    lMessage <<=2;
    if(iCode.Protocol==1) lMessage |= 0b11;
    lMessage <<=2;
    lMessage |= iCode.Button;

    for (int i=31;i>=0;i--)
    {
      if(lMessage>>i & 0b1) lMessageManchester |= 0b10;
      else lMessageManchester |= 0b01;
      if(i>0) lMessageManchester<<=2;
    }
    
    return lMessageManchester;
}

void NexaTransmitter::SendMessage(char iButton, boolean iStatus)
{
  NexaCode newCode;
  newCode.Status = iStatus;
  newCode.TransmitterNumber = transmitterCode;
  newCode.GroupCode = false;
  newCode.Protocol = 1;
  newCode.Button = iButton;
  SendMessage(newCode);
}

//Return status of the button
//ON = true ; OFF = false
boolean getStatus(unsigned long iCode)
{
  return !((iCode>>4 & 1)==1);
}

unsigned long getTransmitterNumber(unsigned long iCode)
{
  return iCode>>6;
}

//Return group code
//ON = true ; OFF = false
boolean getGroupCode(unsigned long iCode)
{
  return !((iCode>>5 & 1)==1);
}

char getProtocol(unsigned long iCode)
{
  if((iCode>>2 & 3)==3) return '1';
  else if((iCode>>2 & 3)==0) return '2';
  else return '0';
}

char getButton(unsigned long iCode)
{
  return (iCode & 3);
}

NexaCode readData(int iRxPin)
{
   unsigned long t;
   unsigned long t1;
   unsigned long t2;
  
   int iNbByte = 0;
   unsigned long sMessage = 0;
   boolean bContinue= true;
   NexaCode oNexaCode;

    t = pulseIn(iRxPin, LOW, 12000);
    if(t>2250 && t<2750){

      //On a trouvé un byte de synchro => On commence à ecouter
      bContinue= true;
      iNbByte = 0;
      //On ecoute 2 pulse a la fois
      while(bContinue)
      {
        t1 = pulseIn(iRxPin, LOW, 12000);
        t2 = pulseIn(iRxPin, LOW, 12000);
  
        //Si t1 = 0
        if(t1>1000 && t1<1500){
          if(t2>200 && t2<300)
          {
            iNbByte++;

          }
          //Si t1 = 0 alors t2 = 1 sinon mauvaise transmission
          else
          {
            bContinue = false;
            sMessage = 0;
          }
        }
        //Si t1 = 1
        else if(t1>200 && t1<300)
        {
          if(t2>1000 && t2<1500)
          {
            sMessage |= ((unsigned long )1<<(31-iNbByte));
            iNbByte++;
            
          }
           //Si t1 = 1 alors t2 = 0 sinon mauvaise transmission
          else
          {
            bContinue = false;
            sMessage = 0;
          }
        }
        else
        {
          bContinue = false;
          sMessage = 0;
        }
  
        if(iNbByte == 32) bContinue = false;
      }
  }

  if(sMessage==0)
    oNexaCode.IsCorrect = false;
  else
  {
    oNexaCode.IsCorrect = true;
    oNexaCode.RawCode = sMessage;
    oNexaCode.TransmitterNumber = getTransmitterNumber(sMessage);
    oNexaCode.Status = getStatus(sMessage);
    oNexaCode.GroupCode = getGroupCode(sMessage);
    oNexaCode.Protocol = getProtocol(sMessage);
    oNexaCode.Button = getButton(sMessage);
  }
  
  return oNexaCode;
}
