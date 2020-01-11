# NeoTrellis 8x8 Games

[![Build Status](https://travis-ci.org/sielenk/neotrellis8x8.svg?branch=master)](https://travis-ci.org/sielenk/neotrellis8x8)

[PlatformIO](https://platformio.org/) project for an [AdaFruit Feather HUZZAH](https://www.adafruit.com/product/2821) board attached to 2x2 [NeoTrellis 4x4](https://www.adafruit.com/product/3954) modules with [Enclosure](https://www.adafruit.com/product/4372) and [buttons](https://www.adafruit.com/product/1611).

## Features

* Four games selectable by a long (>3s) press of the buttons 0..3
  * Button 0 (also active after startup): **Color Toggle**
  
    Toggle through 8 different colors (including black aka off) with each push of a button.
   
  * Button 1: [**Tic Tac Toe**](https://en.wikipedia.org/wiki/Tic-tac-toe)
  
    Shows a 3x3 white(ish) grid.
    It is tinted sightly red or green to indicate the player for the next move.
    Pressing on any button in the 2x2 fields claims that field for the current player.
    If three identical colors in a row are detected, the other color is dimmed to signal a win.
    After a draw, the grid turns white.
    In both cases the next push of any button starts a new game. 
  
  * Button 2: [**Trisentis**](https://ivv5hpp.uni-muenster.de/u/cl/disentis/spektrum.pdf) (or [this](https://ivv5hpp.uni-muenster.de/u/cl/Abschiedsvorlesung.pdf) starting at slide 12, both in german)
  
    The whole 8x8 field starts red and the goal is to turn it green.
    Pressing a button toggles the eight fields surrounding it.
  
  * Button 3: [**Connect Four**](https://en.wikipedia.org/wiki/Connect_Four)
  
    The 6 rows, 7 columns board is framed with blue fields, two of them blinking in the color of the current player.
    When selecting a column by pressing any button in it, the lowest free field is colored according to the current player.
    If four or more fields in a row are detected, they start blinking.
    Pressing button 63 restarts the game at any time.
    
* A basic web page showing the current state of the 8x8 LEDs as a colored HTML table and the current battery voltage.
