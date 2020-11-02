# Package

version       = "0.1.0"
author        = "jprochazk"
description   = "Tile map editor"
license       = "MIT"
srcDir        = "src"
backend       = "cpp"
bin           = @["tedit"]

# Dependencies

requires "nim >= 1.4.0"
requires "nimgl >= 1.1.5"
requires "glm >= 1.1.1"
requires "stb_image >= 2.5"

import os, strformat

task pretty, "Prettify all files in the source directory":
    for file in walkDirRec(srcDir):
        exec &"nimpretty {file} --maxLineLen:120"
