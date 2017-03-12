# game-engine

This is all very much WIP

Features
--------
Emscripten - For targeting browser
Lua VM - For scripting behaviour

Building
------------
- Tup is the build system so install tup http://gittup.org/tup/
- Requires GLEW, SDL
- For native macosx application run tup build-g++
- For browser application using emscripten run tup build-em++ #TODO requires more installation

Next Steps
----------
- Call awake method in lua 
- Call Lua on a fixed update
- Better LUA editing support
  - Edit and reload LUA files on the fly
  - Edit and reload LUA files in the browser
