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

![Screenshot](https://raw.githubusercontent.com/lowagner/bitbox-modes/master/fullcolorbuffer.png)

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

![Screenshot](https://raw.githubusercontent.com/lowagner/bitbox-modes/master/multimode.png)

## fullcolorbuffer3d

**resolution:**  160x120 with 16bit color

this is a very simple example of 3d with the fullcolorbuffer described above;
it doesn't race the beam directly like non-buffered 3d examples (below).

a game using fullcolorbuffer3d is [checkers3d](https://github.com/lowagner/bitbox-checkers3d).

![Screenshot](https://raw.githubusercontent.com/lowagner/bitbox-modes/master/fullcolorbuffer3d.png)

## vertexbuffer3d

**resolution** depends on the simple mode chosen in the Makefile:
- VGA_SIMPLE_MODE=2:  800x600 at 1 bit per pixel (monochrome color palette)
- VGA_SIMPLE_MODE=3:  640x400 (with black top/bottom bands) at 2 bits per pixel (4 color palette)
- VGA_SIMPLE_MODE=4:  400x300 with 4BPP (16 colors)
- VGA_SIMPLE_MODE=5:  320x200 (with black top/bottom bands) at 8BPP (256 colors)

another buffered 3d example which doesn't race the beam directly. 

![Screenshot](https://raw.githubusercontent.com/lowagner/bitbox-modes/master/vertexbuffer3d.png)

## vertex3d  

**resolution:**  640x480 with 16bit color

a non-buffered 3d example, where you can look at points in a 3d world.  uses some
fancy sorting in the y-direction to make sure it only needs to look at the 
next vertex or two it needs to put on screen as it races the beam.

![Screenshot](https://raw.githubusercontent.com/lowagner/bitbox-modes/master/vertex3d.png)

## wireframe3d

**resolution:**  640x480 with 16bit color

a non-buffered 3d example with lines (edges) between points.  and a pretty blue border.
i don't think you can get more than about 20 edges before the current algorithms
become too much for the bitbox, but i will work on a draw-to-buffer algorithm
which may be able to do more.

a game with wireframe3d is [bbgunner](https://github.com/lowagner/bitbox-bbgunner).

![Screenshot](https://raw.githubusercontent.com/lowagner/bitbox-modes/master/wireframe3d.png)


## letterbox

one of my first new modes for bitbox, also fullcolor, but with even further-reduced resolution.

![Screenshot](https://raw.githubusercontent.com/lowagner/bitbox-modes/master/letterbox.png)


## fullcolortile

**resolution:** 320x240 with 16bit color using tiles
