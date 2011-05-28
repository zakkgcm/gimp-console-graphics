# Gimp Console Graphics Plug-In
Loads tile-based video game console graphics formats (SNES, NES, etc).

## Dependencies
* gimp libraries
* gimptool-2.0

## License
GNU Lesser General Public License (LGPL)

## Compiling and Install
`make` and `make install`

## Future Plans
* load pallete from savestate (likely as separate plugin)
* support more formats

## Issues
* "console graphics" is a tad long and ambiguous, could be confused with the green/black consoles of yesteryear.
* converting from RGB back to Indexed will change the indexes of colors (thusly corrupting the original palette).
* handling of images with more than _max_ colors is shaky at best.
