# auto-room
An automatic room made with raspberry pi and shows a lot of info on a screen, turnn the lights on and off. by team DPS

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
#### Compiling yourself (if you want to)
##### Install gcc using your packange manager:
###### Arch Linux:
```bash
sudo pacman -S gcc
```
###### Kali and Ubuntu (or any debian distro
```
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
if there is executable error do `chmod ./build/linux/auto-room` and run it again
### Windows
#### open terminal and clone the repository
```bash
git clone https://github.com/rainbowdesert57/auto-room.git
cd ./auto-room
```
#### Run
```bash
./build/windows/auto-room.exe
```
or using the file manager with `explorer` and then going to build/windows and running auto-room.exe

#### Compiling yourself (if you want to)
Install [mingw-w64](https://www.mingw-w64.org/) (compiler)
Open terminal, then clone and cd into the repository using:
```bash
git clone https://github.com/rainbowdesert57/auto-room.git
cd ./auto-room
```
Then compile with:
```bash
g++ -o ./../logic-simulation.exe ./src/logic-simulation.cpp
```
### macOS
I dont have macOS right now so you have to compile it yourself, here are the steps

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
