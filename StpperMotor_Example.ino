 
//From: https://forum.arduino.cc/index.php?topic=319247.0

// The Sparkfun Monster Moto Sheild PWM inputs are rated at 20kHz and the minimum practical pulse inout is about 4 us.
// sheild pin assignments
//Coil 1
#define PinEN1 A0
#define PinPWM1 5
#define PinCS1 A2

#define PinInA1 7
#define PinInB1 8



//Coil 2
#define PinEN2 A1
#define PinPWM2 6
#define PinCS2 A3

#define PinInA2 4
#define PinInB2 9

#define LED    13

/**
 * This code is using Timer2 for PWM @ 20kHz
 * 1. CLK on Arduino is 16 MHz
 * 2. Timer2 is used for PWM (since it's the least used timer and unlikely to upset anything except the tone library)
 * 3. PWM mod is set to CTC in byte TCCR2A, bit WGM21=1 (CTC = Clear Timer on Compare Match, OCR2A = TOP)
 * 4. Timer2 prescaler is set to 8 => 2 MHz in byte TCCR2B where bit CA21=1
 * 5. CTC-top bit is set to 99 (byte OCR2A=99) => trigger ISR interrupt every 100th clock cykel => PWM @ 20 kHz
 * 6. OCR2B is set to whatever you want the duty cykel to be. The duty cykel is the amount of clock cykels the pin is +5V before it's turned low 
 *    Since the CTC-top is set to 99, the value of OCR2B (-1) will be equal to percentage of the on (value 20 => 0n 20% of the time)
 *    PWM is limited to between 8 and 92% (8% = 4us and is minimum alowed pulse width for the shield, and 92 is set ot not to burn the motor)
 *    
**/

// Timer2 compare A interrupt service routine, triggers on OCR2A = TOP-value (time to restart Timer2 and set PWM-pins high)
ISR(TIMER2_COMPA_vect)
{
  // Turn on PWM pins 5 and 6
  //      76543210 Arduino digital pin number
  PORTD|=B01100000;
}

// Timer2 compare B interrupt service routine, triggers on OCR2B which is duty cycle (time to turn off PWM-pins)
ISR(TIMER2_COMPB_vect)      
{
  // Turn off PWN pins 5 and 6
  //      76543210 Arduino digital pin number
  PORTD&=B10011111;
}

// PWM duty percent 8% to 92%
void setPWMDuty(int duty)
{
  duty=duty-1;          // 0 to 99 equals 100
  if (duty<7) duty=7;   // Minimum pulse width is 4us (see VNH2SP30-E datasheet Table 9) (rise time 1,6 + fall time 2,4)
  if (duty>91) duty=91; // Not a good idea to give the motor 100% power (unlimited)
  OCR2B=duty;           // This affects when the ISR(TIMER2_COMPB_vect) is triggered (makes the pwm-pin low)
}

void setup()
{
  // Initialise H-Bridge (Monster Moto)
  pinMode(PinInA1,OUTPUT);
  pinMode(PinInA2,OUTPUT);
  pinMode(PinInB1,OUTPUT);
  pinMode(PinInB2,OUTPUT);
  pinMode(PinPWM1,OUTPUT);
  pinMode(PinPWM2,OUTPUT);
  pinMode(PinCS1,INPUT);
  pinMode(PinCS2,INPUT);
  pinMode(PinEN1,OUTPUT);
  pinMode(PinEN2,OUTPUT);
  digitalWrite(PinEN1,HIGH);
  digitalWrite(PinEN2,HIGH);
  digitalWrite(PinInA1,LOW);
  digitalWrite(PinInA2,LOW);
  digitalWrite(PinInB1,LOW);
  digitalWrite(PinInB2,LOW);
  digitalWrite(PinPWM1,25);
  digitalWrite(PinPWM2,25);  
 
 // Initialize timer1 
  noInterrupts();           // Disable all interrupts
  TCCR2A = B00000010;       // Disconnect Arduino D3 and D11 pins and set CTC mode (see AVR datasheet 22.11.1) "WGM21"=1(CTC = Clear Timer on Compare Match, OCRA = TOP)
  TCCR2B = B00000010;       // Set clock prescaler to 8, now clock is 2MHz (see AVR datasheet 22.11.2) "CA21"=1
  TCNT2  = B00000000;       // Reset timer (why not?) (see AVR datasheet 22.11.3)
  OCR2A  = 99;              // Set compare match register A for 20kHz PWM frequency (see AVR datasheet 22.11.4) (trigger ISR-interrupt every 100th clock-cykle) => 2000kHz=>20kHz
  OCR2B  = 7;               // OCR2B (see AVR datasheet 22.11.5) to duty, 1% per step, minimum pulse width is 4 us. (trigger ISR-interrupt every xth clock-cykle) => On for x cykles, then off to 100th cykel)
  TIMSK2 = B00000110;       // Enable timer compare interrupt on OCR2A ans 0CR2B (see AVR datasheet S18.11.6)
  interrupts();             // Enable all interrupts

  // PWM duty percent, valid values are between 8% to 92% (limits are imposed)
  setPWMDuty(10);          // 20% is a good alround value for most motors
}

  #define CW 1
  #define CCW -1
  #define Stopped 0
  
  #define SPR 200 //StepsPerRev (Full step)
  #define SPMM SPR //Lead screw => 1 rev =200 steps = 1 mm down/up
  
  //Globals
  int motorState=0;             //0-3 (full step) or 0-7 (half step)           
  int motorDirection=CW;
  int RPM = 1*60;               // RPS = RPM/60
  float motorDelay=RPM/12.0/2.0;        //5[ms] * 400[steps] (400steps/rev)=> 1[rev/sek] = 60 RMP 
  
void loop()
{  
  /**
   * Go_X_mm_FullStep_SinglePh() is only activating one phase at a time  => 100% of phase current
   * Go_X_mm_FullStep_BothPh() is allways running current in both phases => 200 % of one phase current
   * Go_X_mm_HalfStep() is activating both phases every other step       => 150% of one phase current
   * 
  */
  //Forward
  Go_X_mm_FullStep_SinglePh(10.0, 2.0); //Go forward 10mm, 2mm/s
  delay(1000);
  Go_X_mm_FullStep_BothPh(10.0, 2.0);            //Go forward 10mm, 2mm/s
  delay(1000);
  Go_X_mm_HalfStep(10.0, 2.0);            //Go forward 10mm, 2mm/s
  delay(2500);
  
  //Backwards
  Go_X_mm_FullStep_SinglePh(-10.0, 1.0);  //Go back 10mm, 1mm/s
  delay(1000);
  Go_X_mm_FullStep_BothPh(-10.0, 1.0);             //Go back 10mm, 1mm/s
  delay(1000);
  Go_X_mm_HalfStep(-10.0, 1.0);             //Go back 10mm, 1mm/s
  delay(2500);
}

void Go_X_mm_FullStep_SinglePh(float distance, float Speed){  //distance = mm, Speed = mm/sek
  if(distance>=0){
    motorDirection=CW;
  }else{
    motorDirection=CCW;
    distance=-distance;
  }
  
  long NrOfSteps = distance*SPMM;   //SPMM = steps per mm

  motorDelay = 1000.0/(Speed*SPMM);   //motorDelay is ms/step <= 1000/([mm/s]*[Steps/mm])
  
  //Full step: http://www.nmbtc.com/step-motors/engineering/full-half-and-microstepping/
  while (NrOfSteps>0) {
    if (motorState==0) {
      digitalWrite(PinInA1,HIGH);digitalWrite(PinInB1,LOW); //[ 1] Coil 1 
      digitalWrite(PinInA2,LOW); digitalWrite(PinInB2,LOW); //[ 0] Coil 2  
      
    } else if (motorState==1) { 
      digitalWrite(PinInA1,LOW);digitalWrite(PinInB1,LOW);  //[ 0] 
      digitalWrite(PinInA2,HIGH);digitalWrite(PinInB2,LOW); //[ 1]
      
    } else if (motorState==2) { 
      digitalWrite(PinInA1,LOW);digitalWrite(PinInB1,HIGH); //[-1] 
      digitalWrite(PinInA2,LOW); digitalWrite(PinInB2,LOW); //[ 0]  
      
    } else {
      digitalWrite(PinInA1,LOW);digitalWrite(PinInB1,LOW);  //[ 0]
      digitalWrite(PinInA2,LOW); digitalWrite(PinInB2,HIGH);//[-1] 
    }
    motorState+=motorDirection;
    motorState=(motorState+4)%4;
    delay(motorDelay);
    NrOfSteps--;
  }
}

void Go_X_mm_FullStep_BothPh(float distance, float Speed){  //distance = mm, Speed = mm/sek
  if(distance>=0){
    motorDirection=CW;
  }else{
    motorDirection=CCW;
    distance=-distance;
  }
  
  long NrOfSteps = distance*SPMM;   //SPMM = steps per mm

  motorDelay = 1000.0/(Speed*SPMM);   //motorDelay is ms/step <= 1000/([mm/s]*[Steps/mm])
  
  //Full step: http://www.hurst-motors.com/img/Motors/Stepper/stepper_sequences_small.JPG
  while (NrOfSteps>0) {
    if (motorState==0) {
      digitalWrite(PinInA1,LOW);digitalWrite(PinInB1,HIGH); //[-1] Coil 1 
      digitalWrite(PinInA2,LOW);digitalWrite(PinInB2,HIGH); //[-1] Coil 2  
      
    } else if (motorState==1) { 
      digitalWrite(PinInA1,LOW); digitalWrite(PinInB1,HIGH);//[-1] 
      digitalWrite(PinInA2,HIGH);digitalWrite(PinInB2,LOW); //[ 1]
      
    } else if (motorState==2) { 
      digitalWrite(PinInA1,HIGH);digitalWrite(PinInB1,LOW); //[ 1] 
      digitalWrite(PinInA2,HIGH);digitalWrite(PinInB2,LOW); //[ 1]
      
    } else {
      digitalWrite(PinInA1,HIGH);digitalWrite(PinInB1,LOW); //[ 1] 
      digitalWrite(PinInA2,LOW); digitalWrite(PinInB2,HIGH);//[-1] 
    }
    motorState+=motorDirection;
    motorState=(motorState+4)%4;
    delay(motorDelay);
    NrOfSteps--;
  }
}

void Go_X_mm_HalfStep(float distance, float Speed){  //distance = mm, Speed = mm/sek
  if(distance>=0){
    motorDirection=CW;
  }else{
    motorDirection=CCW;
    distance=-distance;
  }
  
  long NrOfSteps = distance*SPMM*2.0;   //SPMM = steps per mm (full step) *2=>half step

  motorDelay = 1000.0/(Speed*SPMM*2.0);   //motorDelay is ms/step <= 1000/([mm/s]*[Steps/mm])
  
  while (NrOfSteps>0) {
    // Half step: https://i0.wp.com/www.alfadex.com/wp-content/uploads/2014/11/Untitled4.jpg 
    //         or http://www.hurst-motors.com/img/Motors/Stepper/stepper_sequences_small.JPG
    if (motorState==0) {
      digitalWrite(PinInA1,HIGH);digitalWrite(PinInB1,LOW); //[ 1] Coil 1  
      digitalWrite(PinInA2,LOW); digitalWrite(PinInB2,HIGH);//[-1] Coil 2  

    } else if (motorState==1) {
      digitalWrite(PinInA1,LOW);digitalWrite(PinInB1,LOW);  //[ 0]        
      digitalWrite(PinInA2,LOW);digitalWrite(PinInB2,HIGH); //[-1]

    } else if (motorState==2) { 
      digitalWrite(PinInA1,LOW); digitalWrite(PinInB1,HIGH);//[-1]  
      digitalWrite(PinInA2,LOW);digitalWrite(PinInB2,HIGH); //[-1]

    } else if (motorState==3) {
      digitalWrite(PinInA1,LOW);digitalWrite(PinInB1,HIGH); //[-1]       
      digitalWrite(PinInA2,LOW);digitalWrite(PinInB2,LOW);  //[ 0]

    } else if (motorState==4) {
      digitalWrite(PinInA1,LOW);digitalWrite(PinInB1,HIGH); //[-1] 
      digitalWrite(PinInA2,HIGH); digitalWrite(PinInB2,LOW);//[ 1] 

    } else if (motorState==5) {
      digitalWrite(PinInA1,LOW);digitalWrite(PinInB1,LOW);  //[ 0]         
      digitalWrite(PinInA2,HIGH);digitalWrite(PinInB2,LOW); //[ 1]

    } else if (motorState==6) { 
      digitalWrite(PinInA1,HIGH); digitalWrite(PinInB1,LOW);//[ 1]        
      digitalWrite(PinInA2,HIGH);digitalWrite(PinInB2,LOW); //[ 1]

    } else {
      digitalWrite(PinInA1,HIGH);digitalWrite(PinInB1,LOW); //[ 1]            
      digitalWrite(PinInA2,LOW);digitalWrite(PinInB2,LOW);  //[ 0]
    }
    motorState+=motorDirection;
    motorState=(motorState+8)%8;
    delay(motorDelay);
    NrOfSteps--;
  }
}

