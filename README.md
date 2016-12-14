# Introduction
Petometer is a pedometer made just for pets. 
An accelerometer counts the number of steps your dog takes and it gets displayed on the Petometer app.
Simultaneously, a temperature sensor detects the temperature of your pet and it gets displayed on the app as well.
#Specifications
This project was developed in Atmel Studio 6.2 and Android Studio.
For hardware, I used 2 ATMEGA 1284 microcontrollers, 1 HC-05 bluetooth module, 1 ADXL 335 accelerometer and 1 TMP 36 temperature sensor. 
I created two seperatre c files for the servant microcontroller and master microcontroller.
The servant one is responsable for taking sensor readings and using SPI sends them to the master.
The master microcontroller receives servant's data and gives it to the bluetooth module.
#Difficulties
- Parsing data sent to Android app
- Making the physical Petometer device small and animal-friendly

#Demonstration
https://www.youtube.com/watch?v=cnPDSTLkDaA
