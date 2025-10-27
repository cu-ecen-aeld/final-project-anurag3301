# LockPi: A Multi-User PIN-Based Door Lock

## Project Video [Link](https://github.com/cu-ecen-aeld/final-project-anurag3301/wiki/anurag3301's-Final-Project-Video)

# Overview

This project is a PIN-based smart door lock system. Unlike a standard door lock, which is intended to be used with a single PIN, this system uses a unique key for every user. 

+ A standered 4x4 Keypad matrix which has keys: 0-9, A-D, * and #. The pin will be 6 characters long, including the number digits and the four alphabet characters, making it very tollerable against bruteforce attacks. This keypad will be connected to the Raspberry Pi 4 using GPIO.
+ A simple relay will be connected to a GPIO to control unlocking the door, the relay can operate any door actuators like a solenoid or eletromagnet.
+ A 1602 LCD display will also be connected through a PCF8574 I2C to 8-bit I/O expander. This display will show messages to the users such as "Welcome <NAME>", "Incorrect PIN", etc.
+ A userland app will be running on the rpi, which will be reading from the char device of the keypad for the keypress and writing to char device of the relay to engage or disengage the relay after doing a pin match with exising dataset. Show appropriate message through the LCD display.
+ The userland app will also log the time and the name of the person who unlocked the door. This app will also run a socket server which a client device connect and request the log data.
+ The system is easy to deploy using a Yocto image.

## Block Diagram
<img alt="image" src="https://github.com/user-attachments/assets/5aa680f1-8b51-406b-b412-ada0c80d0449" />


# Target Build System
I am planing to use Yocto

# Hardware Platform
+ Raspberry Pi 4 Model B
+ 1602 LCD display
+ PCF8574 I2C 8-bit I/O expander
+ 4x4 Matrix Membrane Keypad
+ 5V Replay Module

# Open Source Projects Used
For now I'll be only needing [Yocto](https://www.yoctoproject.org) and [GNU LibC](https://www.gnu.org/software/libc/)

# Previously Discussed Content
+ Writing Yocto Meta-Layers
+ Circular buffer to store keypress
+ Writing char devices

# New Content
+ Using meta-raspberrypi to build images for Rpi
+ Interfacing with real hardware like GPIO and I2C
+ Documentation for 1602 LCD

# Shared Material
This project doesn't rely on any other course, only lot of tutorial, articles and documentations.

# Source Code Organization
Project Source: https://github.com/cu-ecen-aeld/final-project-anurag3301

# Project Video
[Link to project video](https://github.com/cu-ecen-aeld/final-project-anurag3301/wiki/anurag3301's-Final-Project-Video)

# Schedule Page
[Schedule Link](https://github.com/users/anurag3301/projects/8)
