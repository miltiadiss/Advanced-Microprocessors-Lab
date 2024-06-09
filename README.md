# Overview
These reports are part of **Advanced Microprocessors** compulsory Lab course in Computer Engineering & Informatics Department of University of Patras for Spring Semester 2024 (Semester 8).  

For the execution of the exercises we will be using **ATmega4808-4809** microcontroller. The main goal is to program the microcontroller in such way that we can simulate the function of some "smart" appliances and devices.  

Below we can see the block diagram of the microcontroller:

![Στιγμιότυπο οθόνης 2024-05-18 021937](https://github.com/miltiadiss/Microcontrollers-Lab/assets/45690339/74a185d1-9dab-45d0-9276-00794b18dc8c)

For this course we will be using *TCA0* timer-counter in both single and split mode in Waveform Generation Mode (**NORMAL** and **PULSE-WIDTH MODULATION**). For the internal clock frequency of *TCA0* we have chosen to prescale the external **ATmega4808-4809** frequency (fCLK_PER = 20 MHz) by 1024. Also, we will be using the Analog-to-Digital Converter *ADC0* in 10-bit resolution by connecting it to pin 7 of *PORTD* (PD7) that will serve as the analog input. The output signal of *TCA0* and *ADC0* will be simulated by the first 4 LEDs that are connenected to the pins of *PORTD* (PD0 - PD3). Also, switches 5 and 6 of *PORTF* (PF4 - PF5) will be used to simulate the input signal that triggers an interrupt (most commonly it simulates a button being pressed).  

We have chosen to simulate the function of the following "smart" appliances:
* **Traffic Lights**: Exercise 1
* **Robot vacuum**: Exercise 2
* **Fan**: Exercise 3
* **Greenhouse**: Exercise 4
  

**Programming Language and Environment**: C, Microchip Studio.

For further information you can take a look in the code and the technical report that are provided for each exercise.
