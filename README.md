# Catacombs
Catacombs game written in C, inspired by a scene in Season 3 of Realmfall, which is a fanmade series of a series about desserts accelerating for whatever reason.

Yes. It's in C. This is my OWN mistake and I will live up to it.

# Build Instructions
### Linux:
```
gcc -o catacombs catacombs.c -lm
```
Will output `catacombs` once compiled, run with `./catacombs`

### Windows:
***
WARNING: Windows Defender should be scolded before attempting to run!
***
```
cl catacombs.c
```
... Or use Visual Studio, I guess.

Should output `catacombs.exe` once compiled. Open the executable through your preferred method.

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
