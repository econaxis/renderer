# Rudimentary Software 3D Renderer
Features implemented:
 - 3d OBJ file loading
 - ambient lighting
 - specular lighting
 - object rotations/movement
 - camera rotations/movement
 - multiple rendering outputs (ASCII, PNG, SFML window with operating system API's). Live switching between outputs not supported yet.
 - shadows by rendering the scene twice--from POV of light and from POV of camera.

Features not yet worked out:
 - texture mapping
    - perspective correct texture interpolation
 - multiple models 
 - better UI controls
 - running in the web by compiling to WASM or using server/client model


# Progression

## Lines + Triangles
![image](/gifs/simple-triangle.gif)
Rendering a filled triangle


![image](/gifs/text-cube.gif)

*First perspective calculation. First rudimentary line rendering*

A lot of time was spent getting to this point that I didn't document. Getting the cube to rotate correctly in 3D space was difficult. I first prototyped the perspective matrices in MATLAB, then I copied the code over to C++ and also implemented text-based rendering in C++. 

For each frame, I output each pixel to a stringstream object, then flush that stringstream to std::cout. 

Just based from visual inspection, I confirmed the cube to be perspectively correct. For example, parralel lines do converge towards the horizon line.


![image](/gifs/window-cube.gif)

Instead of using text, I used actual pixels to render. Therefore, I could render at much higher quality and performance, without being constrained by the terminal emulator implementation.

## Importing more complex models
![image](/gifs/weird%20teapot.gif)

Implementing a rotating teapot on 3 of Euler rotation axes, with no z-buffering. I think I implemented one of the rotation matrices wrong, hence the warping at the end of the GIF.

![image](/gifs/teddy%20bear.gif)

Still no z-buffer, but filled in triangles.

![image](/gifs/teddy-bear-z-buffer-coloring-closness.gif)

Implemented z-buffering. I colored each face by how close it was. The closer the face, the more red it became.

![image](/gifs/head.gif)

Implemented z-buffering.

## Basic lighting scheme

![image](/gifs/cosine%20lighting.gif)

Instead of setting color as equals to the z-coordinate, I implemented simple cosine lighting. This means the lighting is computed by how much the triangle plane faces the light. If it faces the light directly (thus, the angle is 0), then the brightness is highest.

![image](/gifs/specular%20lighting.gif)
The cosine model ignored the fact that some faces might be reflective. Thus, the angle at which one views the face changes the intensity of the light. You can see this in real life by looking at an egg under intense lighting. If you change your eye position while keeping the egg still, you can see the color (highlight) changes.

I implemented specular lighting by computing dot product of the vector from the face to the camera, and the vector of the reflected light.


### Shadows

To implement shadows, I used shadow mapping. I render the scene twice, first pretending as if I was the light. Then, I mark all the faces that I can see. Those faces will be lit. Faces that I can't see are unmarked. 

Then, I render the scene from the camera's perspective. For every face, I check if that face has been marked *lit* or not, and change the lighting intensity to match whether the face is in shadows.

**Shadows (The sudden perspective switches are me switching between camera and light POV)**

![image](/gifs/shadows.gif)


## Final iteration

![image](/gifs/latest.gif).


### Going retro for fun

![image](/gifs/text%20size%20latest.gif)

Because I'd split the perspective/lighting/rendering module separate from the display module (which is responsible for putting pixels on the screen, either that be through SFML, PNG, or terminal), I tried to render RGB pixels to ASCII characters instead of normal pixels. I also spent a lot of time implementing variable font size, which was quite hard and I couldn't get it to work perfectly (notice the slight jumping when changing font size).

## FXAA

Hello friends, this is another person writing with another brain. So after some googling and realizing how broke both us and the program are, I decided to try some antialiasing using FXAA3. 

Note that FXAA would blur small details. Doubt that’s too big of a problem given the scope of this project.

### Finding high contrast pixels
All of our image data is in RGB, so it is math time! We’ll convert RGB to HCL to find the (human-eye perceived) luminance because computer brightness is different from human brightness. Code can be found in /source. EDIT: I'm not sure if I used the right luminance in the algorithm, but I'll assume it doesn't make too big of a difference and we'll tweak other parameters as needed.

### Identify contrast edges
We did this by comparing the luminance of colors and setting a threshold for contrast ratio.

### Blend time
So we first collected the luminance data of surrounding pixels to determine how much to filter the color. But now my neck hurts and I'll save the rest for tomorrow.

### Long edges? :o 
We are currently ignoring these because our models are not that complicated yet. Might add this feature later, though. 

## UI

Some more notable drafts in Figma
![image](/ui/drafts/figmaFrames.png)

A smol (coded) prototype
![image](/ui/drafts/vid1.mp4)

# Project Description

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

## Structure of the OBJ file

The structure of the OBJ file (which describes all the information in a 3D model) is described more completely [here](https://www.loc.gov/preservation/digital/formats/fdd/fdd000507.shtml). 

An OBJ file contains a list of vertices and their 3d coordinates.

`
v 1.0 2.0 0.0
v 0.5 3.5 1.0
`
For example, these two lines describe two vertices, located at `(1.0, 2.0, 0.0)` and `(0.5, 3.5, 1.0)`. However, vertices are not enough. We also need faces to link these vertices together.

`
f 1 3 2
f 1 4 5
`
For example, these are two faces composed of the first, third, and second vertex, and another face composed of the first, fourth, and fifth vertex. A vertex can have multiple faces connected to it.

This information is enough to render a complete 3d head, like the GIF at the top of this document. A more complex model has more vertices/faces than a simpler model. For example, the head as rendered in the GIF has 100,000 faces and 50,000 vertices. The teddy bear has 3000 faces and 1600 vertices. 

## Memory usage

In my first iteration of this project, I used integers to represent faces (as integers correspond to which vertex we should pick) and doubles to represent 3d coordinates. However, upon loading the file, I could see the memory usage increase linearly as more and more of the OBJ file was put into memory. With the added overhead of my windowing library (SFML) and maintaining an image buffer, I realized I had to reduce memory usage. I replaced doubles with floats (which are 32 bit instead of 64 bit as doubles are). I tried using unsigned shorts to represent indices, but found the conversion operations too much trouble. 

I'm storing all pixel data in memory. Each pixel consists of x, y, z coordinates, and 3 values for color: red, green, blue. I implemented runtime variable resolution, but the goal is to support 1920x1080 resolution output. Therefore, we would have `1920 * 1080 = 2073600` pixels. If we used 6 floats to represent each pixel (3 for position and 3 for color), then we'd need `2073600 * 6 * 4 (bytes/float) = 50 MB` of memory. I couldn't see a way around this big memory usage without reducing resolution. I changed x and y coordinates to use shorts instead, but used double for z coordinate (as precision was important for depth buffering).



## Multithreaded file processing, inspired from Okazaki fragments

The first problem I ran into was extremely slow file parsing. The head model has 150,000 (faces + vertices, each occupy one line) lines of text that we need to parse.

It was taking ~5 seconds to parse the head object, which was significant but not particularly terrible. However, I wanted to improve on this file parsing aspect and test the limits of how fast I could parse a larger file. To validate and measure my improvements, I concatenated the head object file until the final size was 633 MB. This would be the test file.

Running through the file parsing code (`obj_loader.h`) in Visual Studio debugger showed that IO was taking a significant percentage of time. Since I couldn't optimize the standard library, I chose instead to multi-thread file parsing with OpenMP. OpenMP allows me to spawn multiple threads (workers) that each tackle a portion of the file. I accomplished this by splitting the file into chunks equals to the number of threads.

```
------------|------------|------------|------------|------------|------------|
W1 --> ...  |W2 --> ...  |W3 --> ...  |W4 --> ...  |W5 --> ...  |W6 --> ...  |
```

*W1, W2, W3... represent worker num. 1, worker num. 2, worker num.3, ...*

Here, each 6 workers parse their portions of the file (delineated by the |) independently. Theoretically, this would cut file parsing time by 6 times. The actual implementation of this in C++ was accomplished by finding approximate locations of line separators (\n) at each "stop point" (|), then storing the location, found by `stream.tellg()`, in an array. We would loop across that 5 element array (as there are only 5 separators needed to separate 6 workers) and work from the start separator to the end separator. Then, we use OpenMP magic to parallelize that for loop. 

I had many problems with correctness when first implementing this. One bug that I struggled on and eventually diagnosed was because I forgot that vertex order mattered. Each face referenced a vertex by its order in the file (e.g. first, third, 155th, ...), and I lost all order by using multithreading. I considered fixes such as attaching a line number to each vertex and sorting it later, but I saw OpenMP had a construct for this already, the barrier. Using that construct, I fixed the ordering problem by having each worker append to its own vertex array, then concatenating each vertex array in order at the end of the parallelism.

However, there were also weird, occasional bugs. Sometimes, vertices would get duplicated. I learned this was because `tellg()` and `seekg()` was actually not reliable. I couldn't use seekg to seek to the exact position I wanted because seekg didn't support that functionality for text files. This is because seekg and tellg are only correct within a stream. For example, seeking to a position previously returned by tellg on the same stream is accurate. However, I was attempting to seekg to a position returned by tellg *on a different stream.* I couldn't use one stream because each worker thread had to have its own stream.

I couldn't find solutions to this problem, unfortunately, so I had to scrap any idea of parallelism. I only discovered this seekg/tellg behaviour after implementing all this code, and it felt like a waste. At least I did get to learn and implement more advanced parallelism constructs.

I set to work optimizing other parts. For example, I removed stringstream and chose to parse integers myself using basic C++ functions (stof, substr, and the likes). I found removing the stringstream library did significantly improve performance.


## Fractional font scaling and rendering small, subpixel characters

Check gifs folder, [font size test](https://github.com/econaxis/renderer/blob/main/gifs/font%20size%20test.webm)

Write-up soon.

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
