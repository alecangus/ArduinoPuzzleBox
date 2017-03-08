# ArduinoPuzzleBox
A wooden haptics puzzle box based around a Caesar Cipher.
## Premise
Students are met with a wooden box with "Hail Caesar" burned into the lid. Also on the lid is an LCD panel reading "Knock on wood..." and three unlit LEDs. Knocking on the box causes the LCD screen output to change and all three LEDs to start blinking randomly. The box has transitioned into its next state.
### Lift me up high
Knocking on the box caused the box to pick a random number between 1 and 9, and use this as the key to a Caesar Cipher. The phrase "Lift me up high" is converted character by character into ASCII and shifted by this randomly selected key. This key remains constant throughout the rest of the game. The hope is that "Lift me up high" contains enough repeated characters, spaces and two-letter-words that the students will be able to recognise it as a ciphered text and be able to crack it. Cracking the cipher and lifting the box up >1m completes this level.
### Roll me over
The LCD readout changes again; applying the same key to the new cipher text produces "Roll me over". The first LED is now lit solid and the other two are switched off. Rolling the box such that it passes through being upside-down then the right way up again completes this level. A new cipher text is presented and the first two LEDs are now lit.
### Turn me round
Solving the new ciphertext with the same key produces the phrase "Turn me round". Flat-spinning the box 360 degrees completes this level. A new cipher text is presented and all three LEDs are now lit solid.
### Round of applause
The last level requests the students applaud themselves :p. Applause from more than one student will complete the game.
### Congratulations
In clear text the LCD scrolls the message "Congratulations! Passcode: We shall wear our glitter with pride"

## Ingredients
* Arduino Uno
* 2x16 LCD Screen
* Piezo
* Adafruit 10DOF IMU
* Battery
* 3xLEDs

## Technical notes
The loop of the Arduino is written to be a state machine where each of the states is listening to a different aspect of the box's haptic nature.
### State 0
A button on the back of the box switches the box on. The piezo is attached to the underside of the lid of the box and a slight hole is drilled. Initially the Arduino is listening to the piezo.
### State 1
The Arduino switches to using the Adafruit temperature and barometric pressure sensors to calculate its altitude. Smoothing is applied to these readings such that an average of the last 50 readings is used in each consideration to determine what height the box is at. This is to try to smooth away anomolous readings from the sensors.
### State 2
The Arduino then switches to smoothing readings from the Adafruit accelerometer and gyroscope to determine which way up it is. Once it's upside-down it waits to be the correct way up again.
### State 3
Using the Adafruit accelerometer, gyroscope and magnetic compass to determine that the box is staying the right way up and rotating through 360 degrees. I think it actually checks that it's rotated through at least 270 degrees or something like that...
### State 4
There's a threshold applied to the piezo readings to tune out background noise. The threshold we set in State 0 for the knocking is applied to a multiplier to try to get the Arduino to ignore one person clapping but recognise more than one person.