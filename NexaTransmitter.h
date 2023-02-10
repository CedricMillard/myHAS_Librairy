 /*
 * Library to decode and send signals for 433MHz Nexa RC plugs
 * (c) Cedric Millard 2019
 */

#ifndef Nexa_h
#define Nexa_h
#define NEXA_ON true
#define NEXA_OFF false

#include <Arduino.h>

struct NexaCode
{
  //ON = true ; OFF = false
  boolean Status;
  //26 bits code
  unsigned long TransmitterNumber;
  //ON = true ; OFF = false
  boolean GroupCode;
  //protocol used (1=Nexa, 2=Groove/Anslut, 0=Unknown)
  char Protocol;
  //number from 1 to 4
  char Button;
  
  //Used only when reading code, not used for sending
  //32 bits value
  unsigned long RawCode;
  boolean IsCorrect;
};

class NexaTransmitter
{
  public:
    NexaTransmitter(int iTxPin, unsigned long iTransmitterCode);
    void SendMessage(NexaCode iCode);
    void SendMessage(char iButton, boolean iStatus);
    void SendMessage(unsigned long long iCode, int iCodeLength, unsigned int T1_1, unsigned int T1_2, unsigned int T0_1, unsigned int T0_2, unsigned int TP_1, unsigned int TP_2, unsigned int iNbRepeat);

  private:
    //return the 62bits to send through the air
    unsigned long long EncodeMessage(NexaCode iCode);
    int txPin;
    unsigned long transmitterCode;

  //Valeurs identiques a télécommande
    const int T = 200; //us
    const int T1 = 275;
    const int T0 = 1275;
    const int TS = 2500;
    const int TP = 10000; 
    
};

//Read data fron receiver and return 32bits message
NexaCode readData(int iRxPin);

//Return status of the button
//ON = true ; OFF = false
//param icode = message read from receiver
boolean getStatus(unsigned long iCode);

//Return transmitter unique code
//param icode = message read from receiver
unsigned long getTransmitterNumber(unsigned long iCode);

//Return group code
//ON = true ; OFF = false
boolean getGroupCode(unsigned long iCode);

//Return protocol used 
//1 = Nexa 
//2 = Groove/Anslut 
//0 = Unknown
char getProtocol(unsigned long iCode);

//Return which button was selected (1~4)
char getButton(unsigned long iCode);

#endif
