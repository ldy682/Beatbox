# Beatbox
 
This is an application that uses the Beaglebone Green along with a Red Zen Cape attached on top of it. The host (the vm) and the target (Beaglebone) uses debian 11.8 as their operating systems.

## Function 
- Building the program using the CMake copies the wave-files, nodejs-server, and the arm executable to a beatbox folder that can be sent to the target to run.
- Continuously play a beat which is outputted from the hardware's headphone jack.
- Joystick controls
  - up/down to increase and decrease volume
  - left/right to increase and decrease tempo
  - press in to change the beat, no-beat -> beat1 -> beat2 -> no-beat -> ...
- These controls can also be accessed on the webpage using this command on the server directory:
    ```
  $node server.js
- Additionally, moving the hardware left/right, up/down and forward/backward, produces different sounds using the accelerometer.

## Dependencies
- CMake is required to be installed in the host in order to build this project
  ```
  (host)$ sudo apt-get install cmake 
  ```
- Cross compiler tools needed to be installed in the host to create an executable for the target
  ```
  (host)$ sudo apt install gcc make
  (host)$ sudo apt install gcc-arm-linux-gnueabihf
  (host)$ sudo apt install binutils-arm-linux-gnueabihf
  ```
- In order to cross compile the program to play the audio on the target
  ```
  (target)$ sudo apt update
  (target)$ sudo apt install libasound2
  ```
  ```
  (host)$ sudo apt install libasound2-dev
  (host)$ sudo dpkg --add-architecture armhf
  (host)$ sudo apt install libasound2-dev:armhf
  ```
