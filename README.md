# bitbox-modes
prototype view modes for the bitbox console


## fullcolorbuffer

*resolution:*  160x120 with 16bit color

uses a 160x120 16bit framebuffer to write to screen.  it quadruples down the 640x480 mode,
but it could just double down on the 320x240 mode.  it does the former because i still
want higher resolution so as to include some text-based objects, but that will sit in
another directory.

some of the A/B/X/Y buttons do things like change the background evolution function,
or set the background color temporarily to red.

also, you can see some input/output here by pressing select (to save a picture) or start
(to load the last picture).  it saves/loads the background image, and you can load it in
GIMP or some other program capable of dealing with the Netpbm .ppm format.


## fullcolorbuffer3d

*resolution:*  160x120 with 16bit color

this is a very simple example of 3d with the fullcolorbuffer described above;
it doesn't race the beam directly like non-buffered 3d examples (below).


## vertexbuffer3d

*NOT WORKING for some reason.*
*if you are inclined, find out why!*
*possibly drawing too many vertices at once...*

*resolution* depends on the simple mode chosen in the Makefile:
- VGA_SIMPLE_MODE=2:  800x600 at 1 bit per pixel (monochrome color palette)
- VGA_SIMPLE_MODE=3:  640x400 (with black top/bottom bands) at 2 bits per pixel (4 color palette)
- VGA_SIMPLE_MODE=4:  400x300 with 4BPP (16 colors)
- VGA_SIMPLE_MODE=5:  320x200 (with black top/bottom bands) at 8BPP (256 colors)

another buffered 3d example which doesn't race the beam directly. 


## vertex3d  

*resolution:*  640x480 with 16bit color

a non-buffered 3d example, where you can look at points in a 3d world.  uses some
fancy sorting in the y-direction to make sure it only needs to look at the 
next vertex or two it needs to put on screen as it races the beam.


## wireframe3d

*resolution:*  640x480 with 16bit color

a non-buffered 3d example with lines (edges) between points.  and a pretty blue border.
i don't think you can get more than about 20 edges before the current algorithms
become too much for the bitbox, but i will work on a draw-to-buffer algorithm
which may be able to do more.
