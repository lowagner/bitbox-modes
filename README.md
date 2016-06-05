# bitbox-modes
prototype view modes for the bitbox console


## fullcolorbuffer

**resolution:**  160x120 with 16bit color

uses a 160x120 16bit framebuffer to write to screen.  it quadruples down the 640x480 mode,
but it could just double down on the 320x240 mode.  it does the former because i still
want higher resolution so as to include some text-based objects, but that example is in the
`multimode` directory.

some of the A/B/X/Y buttons do things like change the background evolution function,
or set the background color temporarily to red.

also, you can see some input/output here by pressing select (to save a picture) or start
(to load the last picture).  it saves/loads the background image, and you can load it in
GIMP or some other program capable of dealing with the Netpbm .ppm format.

![Screenshot](https://raw.githubusercontent.com/lowagner/bitbox-modes/master/fullcolorbuffer/fullcolorbuffer.png)

## multimode

**resolution:** 160x120 framebuffer with 16bit color, plus text at a higher resolution (color palette, a character takes up 2x4 super pixels)

controls:
* dpad moves text boxes around
* A and Y switch the full-line background evolution function (lines without text boxes)
* X and B switch the partial background evolution function (right/left of text boxes)
* L removes background cascade
* R starts background cascade
* R+down-on-dpad starts infinite background cascade
* select saves the superpixel background to disk (hello.ppm)
* start loads the hello.ppm file on disk to the background.

constraints:
* only 1 text box in any given horizontal line of the screen 
(though see multimode320 code for a work around...)
* do not allow text boxes to go offscreen in the x-direction, or bad things happen.  (you can go above and below without issue.)

![Screenshot](https://raw.githubusercontent.com/lowagner/bitbox-modes/master/multimode/multimode.png)

## multimode320

**resolution:** 160x120 framebuffer with 16bit color, plus text at a higher resolution (color palette, a character takes up 4x4 super pixels), though text is lower resolution than in `multimode`.

controls:
* dpad moves text boxes around
* A and Y switch the full-line background evolution function (lines without text boxes)
* X and B switch the partial background evolution function (right/left of text boxes)
* L removes background cascade
* R starts background cascade
* R+down-on-dpad starts infinite background cascade
* select saves the superpixel background to disk (hello.ppm)
* start loads the hello.ppm file on disk to the background.

constraints:
* any text box A whose top is higher than any box B's top must also have its bottom above or at the same y-position as B's bottom.
* other than that, you can have multiple text boxes on a line.
* do not allow text boxes to go offscreen in the x-direction, or bad things happen.  (you can go above and below without issue.)

![Screenshot](https://raw.githubusercontent.com/lowagner/bitbox-modes/master/multimode320/multimode320.png)
## fullcolorbuffer3d

**resolution:**  160x120 with 16bit color

this is a very simple example of 3d with the fullcolorbuffer described above;
it doesn't race the beam directly like non-buffered 3d examples (below).

a game using fullcolorbuffer3d is [checkers3d](https://github.com/lowagner/bitbox-checkers3d).

![Screenshot](https://raw.githubusercontent.com/lowagner/bitbox-modes/master/fullcolorbuffer3d/fullcolorbuffer3d.png)

## vertexbuffer3d

**resolution** depends on the simple mode chosen in the Makefile:
- VGA_SIMPLE_MODE=2:  800x600 at 1 bit per pixel (monochrome color palette)
- VGA_SIMPLE_MODE=3:  640x400 (with black top/bottom bands) at 2 bits per pixel (4 color palette)
- VGA_SIMPLE_MODE=4:  400x300 with 4BPP (16 colors)
- VGA_SIMPLE_MODE=5:  320x200 (with black top/bottom bands) at 8BPP (256 colors)

another buffered 3d example which doesn't race the beam directly. 

![Screenshot](https://raw.githubusercontent.com/lowagner/bitbox-modes/master/vertexbuffer3d/vertexbuffer3d.png)

## vertex3d  

**resolution:**  640x480 with 16bit color

a non-buffered 3d example, where you can look at points in a 3d world.  uses some
fancy sorting in the y-direction to make sure it only needs to look at the 
next vertex or two it needs to put on screen as it races the beam.

![Screenshot](https://raw.githubusercontent.com/lowagner/bitbox-modes/master/vertex3d/vertex3d.png)

## wireframe3d

**resolution:**  640x480 with 16bit color

a non-buffered 3d example with lines (edges) between points.  and a pretty blue border.
i don't think you can get more than about 20 edges before the current algorithms
become too much for the bitbox, but i will work on a draw-to-buffer algorithm
which may be able to do more.

a game with wireframe3d is [bbgunner](https://github.com/lowagner/bitbox-bbgunner).

![Screenshot](https://raw.githubusercontent.com/lowagner/bitbox-modes/master/wireframe3d/wireframe3d.png)


## fullcolortile

**resolution:** 320x240 with 16bit color using 16x16 pixel tiles

also sprites are available at 16x16 pixels.

![Screenshot](https://raw.githubusercontent.com/lowagner/bitbox-modes/master/fullcolortile/fullcolortile.png)


## letterbox

**resolution:** 128x84 with 16bit color

one of my first new modes for bitbox, also fullcolor, but with even further-reduced resolution, 128x84, but with room on the top and bottom for other information.


![Screenshot](https://raw.githubusercontent.com/lowagner/bitbox-modes/master/letterbox/letterbox.png)


## superletterbox 

**resolution:** 320x100 with 16bit color

also fullcolor, with room on the top and bottom for other information.
note however, that the buffer takes up all the fast CCM memory 
(320*100 pixels * 2 bytes/pixel = 64 kB) so drawing other stuff may be slower.

![Screenshot](https://raw.githubusercontent.com/lowagner/bitbox-modes/master/superletterbox/superletterbox.png)


## verticallines

**resolution:** 320x??? with 8bit color

variable vertical resolution, you can have up to MAX_V vertical pixels in each column (currently MAX_V is 32 in nonsimple.h).  
memory usage:  `320*MAX_V*2 + 320 + 256*2` (vertical pixels and overhead for variable pixel location, then palette).
if you have more than 98 vertical pixels, you should probably go with the 320x200 simple mode (with 8bit palette)

![Screenshot](https://raw.githubusercontent.com/lowagner/bitbox-modes/master/verticallines/verticallines.png)

## fullcolorvertical

**resolution:** 320x??? with 16bit color

variable vertical resolution, you can have up to MAX_V vertical pixels in each column (currently MAX_V is 32 in nonsimple.h).  
memory usage:  `320*MAX_V*3 + 320` (vertical pixels and overhead for variable pixel location).
if you have more than 66 vertical pixels, you will run out of CCM memory.

![Screenshot](https://raw.githubusercontent.com/lowagner/bitbox-modes/master/fullcolorvertical/fullcolorvertical.png)

## palette16 

**resolution:** 320x240 with a 16 color palette
(160*240 = 38.4 kB for framebuffer, 1024 bytes for a quickly drawing palette).

I used the palette from http://androidarts.com/palette/16pal.htm
though adding a purple (INDIGO) instead of NIGHTBLUE.
(the original NIGHTBLUE, if you are interested, is RGB(27, 38, 50)).

It's also like an Etch-A-Sketch, so you can move a little drawing particle (the 'player') around.

Controls:
* dpad - move player particle and paint
* B/Y - cycle player colors forward/back
* A - flood fill area to current color
* L/R - decrease/increase speed of particle
* Start - save the file to 16PAL000.PXL (where 000 will increment if you save more than one).
* Select - load the last saved file.  (or if you continue hitting select, move further back in history.)

![Screenshot](https://raw.githubusercontent.com/lowagner/bitbox-modes/master/palette16/palette16.png)

## palette16tile

**resolution:** 320x240 with a 16 color palette, 16 16x16 background tiles, and 16 sprites 16x16 with 8 frames,
and some 8192 bytes for a tile map.  Tried to keep everything under 32kB, in case we want to port it to micro.

Everything is based on nibbles (half-bytes), so you'll see lots of (X&15) and (X>>4) for some (unsigned) byte X.

There's also a tile viewer if you hit select.

Controls:
* dpad - move around
* select - switch to a different view mode

In tile-edit mode:
* X/Y - cycle color of cursor
* A - toggle painting with cursor
* B - paint (if cursor isn't painting) or fill (if cursor is painting)
* L/R - switch tile to edit

![Screenshot](https://raw.githubusercontent.com/lowagner/bitbox-modes/master/palette16tile/palette16tile.png)
![Screenshot](https://raw.githubusercontent.com/lowagner/bitbox-modes/master/palette16tile/palette16tile-editing.png)
![Screenshot](https://raw.githubusercontent.com/lowagner/bitbox-modes/master/palette16tile/palette16tile-save.png)
