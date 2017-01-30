# Dual-VNH2SP30-Stepper-Current-Control
This is for the <b>30A Monster Moto Shield with Dual VNH2SP30</b> to run a high torque bipolar (4 wire) stepper motor I have rated at 6A/ph.
I tried to run it with the pololu stepper motor driver DRV8825 (2A max) but that didn't have nearly enough amps to give me any torque.
So I ordered the [30A Monster Moto Shield](http://www.ebay.com/sch/i.html?_from=R40&_trksid=p2047675.m570.l1313.TR0.TRC0.H0.X30A+Monster+Moto+Shield+Dual+VNH2SP30.TRS0&_nkw=30A+Monster+Moto+Shield+Dual+VNH2SP30&_sacat=0) for like five bucks.
The problem with these drivers is that they don't have a built in function to set the max current as the DRV8825 have... I had to ask uncle google, and the solution is simple: use PWM to control the power through the motor phases!

After some search I found out that there are librarys out there that uses the built in PWM on the arduino, but that only clocks at 960Hz, which turns out to be really noisy! 

This program is based up on this [code](https://forum.arduino.cc/index.php?topic=319247.0) that Alan0 wrote on the Arduino forum and uses Timer2 set to run at 2MHz to output a 20KHz PWM to the VNH2SP30's (which is max according to datasheet)
Timer2 is used to not disturbe functions like delay() and other vital functions.

I made a fiew functions to run 
* full step with one phase active at a time 
* Full step with two phases active all the time, high torque! (200% of current compared to the above)
* Half step, smoother (150% of current compared to first) 

You set the current by changing the duty cykle of the PWM to a percentage of max (limited to min 8% and max 92%) min limit is to not go under 4us (datasheet) and max is to not let the motor burn up (which it will probably do way before this max anyway...)

According to Alan0 20% duty is usually good, but start at 10% and go up, and put an amp-meter between the powersupply and the driver so that you know what amps your stepper is pulling! 

The VNH2SP30 has an Amp-measuring pin, but you have to know you have the right caps to eaven out the PWM! 
From [this forum post](http://forum.arduino.cc/index.php?topic=338633.0)
"Note that the current-sensing is not able to average out the PWM peaks. 
 SOLUTION: increase the value of the capacitor on the SparkFun board to fiter in the analog domain.
    The 33nF cap and 10K resistor have a cutoff frequency of 480Hz - above the 450hz freq 
    of the PWM system. Add a 0.1uF cap to this and the cutoff frequency drops to 129Hz."
    
We should be good though at 20kHz!? have to compare this with my amp-meter ;)
