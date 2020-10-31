# FluxLib : a library for PlayStation™️ Vita

FluxLib (unofficial name) is a library that was used by Mass Media Productions for the Ratchet & Clank Trilogy HD and Jak & Daxter HD port to PS Vita™️.
(*It might have been used for the Jak & Daxter HD on PS3™️ but I haven't looked at any of those versions. Feel free to send evidence of this.*)

This repository hosts a rewritten, open-sourced version produced by reverse-engineering.
The rewritten code will NOT be a 1:1 copy of the decompiled code, some function might be improved/modified.
The exposed API, however, will be 100% compliant with the original FluxLib. 
This means that the library's behaviour cannot be changed nor expanded.
All the code is avaliable under the GPLv3 license.

**THIS REPOSITORY IS CURRENTLY IN A VERY W.I.P. STATE, MOST OF THE INFORMATION IS BASED ON MY ASSUMPTIONS AND MIGHT BE INCORRECT.**

## Submodules
FluxLib is divided in different submodules (note that this list isn't exaustive, and names may or maybe not be representative of the "official" names) :
* `FluxFS` (officially named `FluxFileIO` ?) : A helper lib for SceFIOS2, including support for PSARC
* `Flux3D`
* `FluxMemMan`
* `FluxMath`
* `FluxReflection`
* `FluxConsole`
* `FluxToolkit` (most likely a PC-sided lib)
* `FluxBase`
* `FluxInput`
* `FluxVideo`
* `FluxRender`

Documentation for each submodule will be avaliable when they are sufficiently reversed.