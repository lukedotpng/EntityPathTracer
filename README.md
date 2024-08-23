# Entity Path Tracer 

## This is a mod to trace the path of items in Hitman WoA
- Project made using the ZHM Mod SDK and the [ZHM Mod Generator](https://zhmmod.nofate.me/) 

## Usage
- This mod is designed for tracing the path of bomb launches.
- To start tracking an item you must drop/place it on the ground, and then press `P`.
    - The last item dropped/placed will be the one selected, given it is able to be tracked
    - Breaching charges are ignored, so if you drop it after the bomb you want to launch, dont worry!
- A white box will appear around the item you are tracking as a confirmation.
- To stop tracking an item, press `P` again.
    - Dont worry if you accidently press `P` after the item has moved, as long as you dont drop/place another item, the traced path will be saved.
- To clear the current traced path, press `O`. This will not affect what item you are currently tracking.
- Customization settings available is the ZHM mod menu, which you can access by pressing the `~` key (`^` on QWERTZ layouts).

## Installation Instructions

1. Download the latest version of [ZHMModSDK](https://github.com/OrfeasZ/ZHMModSDK) and install it.
2. Download the latest version of `EntityPathTrace` and copy it to the ZHMModSDK `mods` folder (e.g. `C:\Games\HITMAN 3\Retail\mods`).
3. Run the game and once in the main menu, press the `~` key (`^` on QWERTZ layouts) and enable `EntityPathTrace` from the menu at the top of the screen.
4. Enjoy!

## Build It Yourself!

### 1. Clone this repository locally with all submodules.

You can either use `git clone --recurse-submodules` or run `git submodule update --init --recursive` after cloning.

### 2. Install Visual Studio (any edition).

Make sure you install the C++ and game development workloads.

### 3. Open the project in your IDE of choice.

See instructions for [Visual Studio](https://github.com/OrfeasZ/ZHMModSDK/wiki/Setting-up-Visual-Studio-for-development) or [CLion](https://github.com/OrfeasZ/ZHMModSDK/wiki/Setting-up-CLion-for-development).
