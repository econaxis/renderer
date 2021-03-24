![image](/gifs/latest.gif)
![image](/gifs/window-cube.gif)
![image](/gifs/weird%20teapot.gif)
![image](/gifs/text-cube.gif)

For our capstone project, we made a renderer that converts a 3D object to a 2D image that could be displayed on the 
screen. Essentially, it's similar to simulating how the camera works. It converts a 3D scene into a 2D image, losing 
some information in the process. For example, depth is lost, objects behind other objects are hidden, and things off 
to the side are distorted (the distortion's extra visible when using a wide-angle lens, for example). 

To get a sense of depth in a photographic image, we can use some cues like converging parallel lines, vanishing 
points, atmospheric depth (losing color saturation the further away you are), and overlapping objects (objects that 
overlap another object are in front). These are the same things that we'll try to simulate with this project.

The fundamental tool we use to do this is a perspective projection, which projects a 3D coordinate into a 2D plane. 

(perspective_proj image1)

I understood this projection first by understanding a simpler, 2D to 1D projection. 

(2d to 1d images 2 4 5)

When we project something from a higher dimension to a lower dimension, we loose information. 

(loosing information image 6)

We can do the same projection from 3D to 2D. Projecting point A in 3D space to point a in 2D space on the eye plane 
is like solving similar triangles. The fovea, a, and A lie on a straight line. In other words, we could also say the 
fovea and A makes up the two points for the hypotenuse of an imaginary right triangle. Then, point a also lies on 
that hypotenuse, albeit a smaller, similar triangle. 

(perspective_proj image 3)
