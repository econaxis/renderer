![image](/gifs/latest.gif)
![image](/gifs/window-cube.gif)
![image](/gifs/weird%20teapot.gif)
![image](/gifs/text-cube.gif)
![image](/gifs/text%20size%20latest.gif)

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
