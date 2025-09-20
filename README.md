# Auto-Room
An automated room system built with Raspberry Pi.  
It controls lights, displays room information on a screen, and provides automation features.  
Developed by **Team DPS**.

![Platforms: ](https://img.shields.io/badge/platform-linux%20%7C%20windows%20%7C%20macos-blue)

## Installation
### Linux
#### Clone the repository using terminal
```bash
git clone https://github.com/rainbowdesert57/auto-room.git
cd ./auto-room
```
#### Run
```bash
./build/linux/auto-room
```
If you get an executable permission error, run `chmod +x ./build/linux/auto-room`
#### Optional: Compile from source
##### Install gcc using your package manager:
###### Arch Linux:
```bash
sudo pacman -S gcc
```
###### Kali and Ubuntu (or any debian distro)
```bash
sudo apt install gcc
```
#### Then clone and cd into the repository:
```bash
git clone https://github.com/rainbowdesert57/auto-room.git
cd ./auto-room
```
##### Now compile using:
```bash
g++ ./src/logic-simulation.cpp -o ./../logic-simulation
```
After compiling run it using `./../logic-simulation`
### Windows
#### open terminal and clone the repository
```cmd
git clone https://github.com/rainbowdesert57/auto-room.git
cd ./auto-room
```
#### Run
```cmd
./build/windows/auto-room.exe
```
or using the file manager with `explorer` and then going to build/windows and running auto-room.exe

#### Optional: Compile from source
Install [mingw-w64](https://www.mingw-w64.org/) (compiler)
Open terminal, then clone and cd into the repository using:
```cmd
git clone https://github.com/rainbowdesert57/auto-room.git
cd ./auto-room
```
Then compile with:
```cmd
g++ -o ./../logic-simulation.exe ./src/logic-simulation.cpp
```
### macOS
I don't have macOS right now so you have to compile it yourself, here are the steps

#### Install dependencies
```bash
xcode-select --install
```
#### Clone and cd into repository
```bash
git clone https://github.com/rainbowdesert57/auto-room.git
cd ./auto-room
```
#### Compile
```bash
g++ -std=c++17 -O2 -Wall -o ./../logic-simulation ./src/logic-simulation.cpp
```
#### Run
```bash
./../logic-simulation
```

## Updating
```bash
git pull
```
This will update everything if there are no changes, if there are any changes, then you need to reinstall

For macOS, compile again for the updated source code using `g++ -std=c++17 -O2 -Wall -o ./../logic-simulation ./src/logic-simulation.cpp` and run using `./../logic-simulation`
