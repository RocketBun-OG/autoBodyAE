# autoBody AE
A SKSE Plugin for distributing CBBE and HIMBO presets throughout the gameworld
* Configurable through morphs.ini files (optionally created by [JBS2BG](https://www.nexusmods.com/skyrim/mods/88707/)), just like Racemenu's Bodygen
* Faction **and** race support! formatted in morphs.ini like this: ``All|Female|TownWhiterunFaction`` or ``All|Female|NordRace``. Uses EditorIDs just like Bodygen does. 
* Reimplementation of [Obody's](https://www.nexusmods.com/skyrimspecialedition/mods/51084) ORefit algorithm! 
## Development/Compile Requirements
* [CMake](https://cmake.org/)
	* Add this to your `PATH`
* [The Elder Scrolls V: Skyrim Special Edition](https://store.steampowered.com/app/489830)
	* Add the environment variable `Skyrim64Path` to point to the root installation of your game directory (the one containing `SkyrimSE.exe`).
* [PowerShell](https://github.com/PowerShell/PowerShell/releases/latest)
* [Vcpkg](https://github.com/microsoft/vcpkg)
	* Add the environment variable `VCPKG_ROOT` with the value as the path to the folder containing vcpkg
* [Visual Studio Community 2022](https://visualstudio.microsoft.com/)
	* Desktop development with C++

## Register Visual Studio as a Generator
* Open `x64 Native Tools Command Prompt`
* Run `cmake`
* Close the cmd window

## Building
```
git clone https://github.com/napmouse/autoBodyAE
cd autoBodyAE
git submodule init
git submodule update
cmake --preset vs2022-windows
cmake --build build --config Release
```

## Credit/thanks
* [Sairion350](https://github.com/Sairion350) for their original work on OBody, which I referenced extensively and copied wholesale in a few places. 
* [Ryan-rsm-McKenzie](https://github.com/Ryan-rsm-McKenzie/) for their work on CommonLibSSE and their lovely example plugin, without which this project would not have been possible. 
* My buddy Mike, for being my rubber duck. 
*Scrab, for their assistance with a particular feature of the CommonLibSSE library