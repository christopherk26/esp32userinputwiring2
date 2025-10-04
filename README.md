

This sketch is a self-test for part of a larger ESP32 MP3 player project. 

It demonstrates the correct wiring for an SD card module and five user buttons, along with serial output for basic validation. 
The buttons are each connected between a GPIO pin and ground, with the ESP32’s internal pull up resistors enabled in software. 

This means each pin idles HIGH and reads LOW when the button is pressed, eliminating the need for external resistors. 
The sketch also filters out macOS hidden system files from SD card listings so only relevant content is shown.

The wiring is shown in the following image. 

<img width="869" height="549" alt="Screenshot 2025-10-04 at 1 40 24 PM" src="https://github.com/user-attachments/assets/a5b0b35e-2f61-4d8b-8485-2138a99434cf" />


The serial output is shown in the following image. 

<img width="659" height="441" alt="Screenshot 2025-10-04 at 1 34 05 PM" src="https://github.com/user-attachments/assets/38ec9c86-e8e7-4500-8dad-00ea69181e32" />
