# Sexy
![screeshot](https://raw.githubusercontent.com/danieltabor/sexy_game/main/screenshot.jpg) 

Simple game to demonstrate rumble capabilities of game controllers. 

NSFW.

### Controls

Mouse Controls:

Click and drag mouse to control dials.
Click Next to see the next girl.

Keyboard Controls:
- Q - Quit
- M - Mute
- N - Next
- F - Adjust Force
- S - Adjust Speed
- B - Adjust Bounce

### Building

#### Linux/Unix

Required dependencies:
- libSDL2
- libSDL2_image
- libSDL2_mixer

Just build the executable:

```
make sexy
```

Build a release:

```
make distclean
make
```
Standalone distribution in directory ``sexy_lin64`` and packaged in ``sexy_lin64.txz``


#### Windows

Required dependecies:
- Included in the ``win64`` directory
- x86_64-w64-mingw32 build chain

Windows (64-bit) version is cross compiled on Linux.

```
make -f Makefile.win64 distclean
make -f Makefile.win64
```

Standalone distribution in directory ``sexy_win64`` and packaged in ``sexy_win64.zip``
