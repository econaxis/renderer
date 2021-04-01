![image](/gifs/latest.gif)
![image](/gifs/window-cube.gif)
![image](/gifs/weird%20teapot.gif)
![image](/gifs/text-cube.gif)
![image](/gifs/font size test.webm)

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


Thus, assuming the eye is at coordinate (0, 0), and the plane upon which we project the image is at z=1, then all we need to do is to divide the (x, y, z) coordinates by z. The resulting coordinate for all points would be (x/z, y/z, z/z) = (x/z, y/z, 1).

Points farther away from us would be closer to the origin (as we experience in real life too). 

(1dperspective)[https://www.thesprucecrafts.com/thmb/yTFMfeNboT1MOQYsdeiSJL_MZSc=/800x800/filters:no_upscale():max_bytes(150000):strip_icc()/perpsective8example3-56a26ccd3df78cf77275803b.jpg]


Loosing information is inevitable when we project to 2D. For example, the coordinate (1, 1, 1) and (2, 2, 2), although they are distinct points, map to the same (1, 1) coordinate on the eye plane. It's similar to what happens when things overlap each other in real life. They map to the same position in our eye, but we only see the one that's closer to us. We loose information.

With this, we can place any arbitrary 3D point into a location in our image. But since 3D objects can be described with triangles that are described with vertices, we can also view any 3D model.

(teddy bear 3d model)

The first problem we encountered, upon reaching this stage, was that the model appears transparent. Triangles in front don't block triangles behind. To solve this issue, we used an old technique called the z-buffer.

# Some other problems

Some of these topics are problems that I solved through this project that I found was interesting and had to devise my own methods to solve. 

## Multithreaded file processing using Okazaki fragments

## Fractional font scaling and rendering small, subpixel characters

## Warm vs cool, artificial vs natural lighting and shadows and hue

## std::vector<std::vector<Pixel>> vs std::vector<Pixel> for cache efficiency



# Compiling/Running

Compiling this is quite weird :(

You need to compile SFML first.
```
cd extern/sfml
mkdir build
cd build
cmake .. -DCMAKE_BUILD_TYPE=Release -DBUILD_SHARED_LIBS=FALSE -DCMAKE_INSTALL_PREFIX=Release
cmake --build . --target install
```

Then, you need to compile the rest of the program.
Change to the root source directory
```
mkdir build
cd build
cmake .. -DCMAKE_BUILD_TYPE=Release
cmake --build .
./game
```
