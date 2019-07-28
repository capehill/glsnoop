# glSnoop
An experimental graphics API tracing tool for AmigaOS 4

Work-in-progress and incomplete, pull requests welcome!

## Tracing support

glSnoop supports partial logging of:

- ogles2.library
- Warp3DNova.library

Pausing stops the serial logging of functions until resumed.

## Profiling

glSnoop records the call count and duration of known OpenGL ES 2.0
and Warp3D Nova functions. Profiling summaries are logged when
contexts are being destroyed, or when user finishes profiling manually
using the GUI. Re-starting profiling clears the current statistics.

Time spent inside OpenGL ES 2.0 functions includes also time spent
inside Warp3D Nova functions.

## Filtering

Filtering can be used to patch only a subset of OGLES2/NOVA
functions. This produces less logs and is faster. Filtering
impacts also profiling, because non-patched functions
cannot be profiled either.

## Command-line parameters

- OGLES2: enable ogles2.library tracing and profiling
- NOVA: enable Warp3DNova.library tracing and profiling
- GUI: launch the graphical user interface
- PROFILE: disable function tracing in serial logs but keep profiling stats
- FILTER filename: define a subset of patched functions

## Tips

glSnoop uses serial port for logging. To redirect logs
to a file (for speed), one may use the Sashimi tool:

Sashimi > RAM:MyLogFile.txt

Download Sashimi: http://os4depot.net/index.php?function=showfile&file=development/debug/sashimi.lha

## License

PUBLIC DOMAIN

## Thanks

- Kas1e for ideas, testing and AmigaGuide document
- Mason for icons
- samo79 for ideas
- Hans for ideas

