# FluxLib : a library for PlayStation™️ Vita

FluxLib (unofficial name) is a library that was used by Mass Media Productions for the Ratchet & Clank Trilogy HD port to PS Vita.
(*It most likely has been used for the Jak & Daxter HD version, at least on PS Vita™️, but I haven't looked at any of those yet. Feel free to send evidence of this in issues.*)

This repository hosts a rewritten, open-sourced version produced by reverse-engineering.
It is made to work with DolceSDK but can easily be adapted.
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