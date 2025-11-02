# Catacombs
Catacombs game written in C, inspired by a scene in Season 3 of Realmfall, which is a fanmade series of a series about desserts accelerating for whatever reason.

Yes. It's in C. This is my OWN mistake and I will live up to it.

# Build Instructions

You will need to compile both the map generator and the game artifacts.

### Linux:

Linux compilation requires GCC. You will need to refer to your distro's package managers to install it for your specific system.

```
gcc -o catacomb_generator catacomb_generator.c -lm
gcc -o catacombs catacombs.c -lm
```
OR, via shell script:
```
chmod +x ./compile.sh
./compile.sh
```
Game: Will output `catacombs` once compiled, run with `./catacombs`
Map Generator: Will output `catacomb_generator` once compiled, run with `./catacomb_generator`

### Windows:
***
WARNING: Windows Defender should be scolded before attempting to run!
***
```
cl catacombs.c
cl catacomb_generator.c
```
... Or use Visual Studio, I guess.

Should output `catacombs.exe` and `catacomb_generator.exe` once both artifacts are compiled. Open the executables through your preferred method.

# Getting started

In order to play the game, you must run `catacomb_generator` in the same directory as the game files are. Leave the name input blank to generate a default map.

A minimum board size of 50 x 50 is recommended for gameplay.

Once the map has been generated, you may run `catacombs` and play!

## Selecting Custom Maps

Catacombs will load up a custom map that is in the same directory as the game executable. Simply add the name (no spaces) of the map to the program runtime arguments.


# Controls:

- W: Move North
- A: Move West
- S: Move South
- D: Move East
- E: Forfeit turn (Do nothing)
- Q: Check heartrate

To hide, move into a hiding spot.

To open a chest and get an item, move into a treasure chest tile.

# Credits:
- Developer & Designer: Modoromu (モドローム)

# License:
"Catacombs" is licensed under the Mozilla Public License 2.0
