# Overview
These reports are part of **Advanced Microprocessors** compulsory Lab course in Computer Engineering & Informatics Department of University of Patras for Spring Semester 2024 (Semester 8).  
For the execution of the exercises we will be using **ATmega4808-4809** microprocessor. The main goal is to program the microprocessor in such way that we can simulate the function of some "smart" appliances and devices.  

![Στιγμιότυπο οθόνης 2024-05-18 021937](https://github.com/miltiadiss/Microcontrollers-Lab/assets/45690339/74a185d1-9dab-45d0-9276-00794b18dc8c)

For this course we will be using *TCA0* timer-counter in both single and split mode (or *TCB0*, *TCB1*). Also, *PORTF(PFn)* will be used to simulate the input signal that indicates an interrupt (most commonly it simulates a button being pressed) and *PORTD(PDn)* will be the output signal.  

We have chosen to simulate the function of the following "smart" appliances:
* **Traffic Lights**: Exercise 1
* **Robot vacuum**: Exercise 2
* **Fan**: Exercise 3
* **Greenhouse**: Exercise 4
  

**Programming Language and Environment**: C, Microchip Studio.
