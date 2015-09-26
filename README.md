# bitbox-modes
prototype view modes for the bitbox console


## fullcolor_matrix  

resolution:  160x120

uses a 160x120 16bit framebuffer to write to screen.  it quadruples down the 640x480 mode,
but it could just double down on the 320x240 mode.  it does the former because i still
want higher resolution so as to include some text-based objects, but that will sit in
another directory.

some of the A/B/X/Y buttons do things like change the background evolution function,
or set the background color temporarily to red.

also, you can see some input/output here by pressing select (to save a picture) or start
(to load the last picture).  it saves/loads the background image, and you can load it in
GIMP or some other program capable of dealing with the Netpbm .ppm format.


## vertex3d  

resolution:  640x480

a simple 3d example, where you can look at points in a 3d world.


## wireframe3d

resolution:  640x480

a simple 3d example with lines (edges) between points.  and a pretty blue border.
i don't think you can get more than about 20 edges before the current algorithms
become too much for the bitbox, but i will devise some draw-to-buffer algorithms
which may be able to do more.
