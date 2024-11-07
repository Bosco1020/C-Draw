# 2021-COMP3016-W2: Free Project

YouTube Walkthrough - https://youtu.be/RyvNNc3Rgdc

Load the Visual Studio local debugger to try the application in the IDE, or visit my portfolio website to download the executable - https://www.connorbos-dev.com/c-draw


## User Interaction:
As a drawing application, the user can hold left click and move the mouse to draw lines across the white canvas. Using the scroll wheel, you can change the thickness of the lines drawn. By right clicking you cycle through 6 available colours.
There is also a GUI panel which can be minimised, expanded and moved. This panel allows you to edit the width of your brush, alter the sensitivity (frequency of extrusions), change the colour of the current selection and swap from the regular line drawing method to an experimental triangle drawing method, which draws a series of sharp triangles against the direction of the cursor.


## Implementation:
The lines drawn are created through continuously adding rectangles onto the end. By calculating the direction of the cursor, it works out the coordinates of the 2 new vertices and which 2 of the previous vertices make up the newly added square, which is drawn from 2 triangles.
The lineShape class is used to create and add new lines to the total arrays, which hold the vertex information of all the shapes on screen.
To ensure that the width of the drawn line remains consistent no matter the angle of line drawn, a set of calculations using the perpendicular gradient and Pythagoras’s theorem are used to deduce the exact positions of the points in space. These calculations and explanations will be added at the end for those interested.


## Ideation:
The idea came from looking at 3D drawing applications, where users can paint in 3 dimensions. Thinking about how that could be done, I looked into methods of rendering lines and adopted a method I saw from a post which took a different approach [2]. This method of generating lines in space could be used for both 2D and 3D lines, or even any sequence of shapes given some edits to the indices.
When using common drawing applications, you can see the pixels colour being changed individually within the brush’s radius to the set colour. Rather than applying a colour to a space, this application creates a series of shapes that extend from one another to form a cohesive line.
By creating a system which adds a sequence of triangles to fill in where the line is, it’d also be possible to upscale how many triangles are drawn. It could be changed to draw circles, pentagons, or even 3D cylinders and boxes using the same extrusion method, just changing the logic behind how the elements are stored.


## Contribution:
When creating the application, the university provided a template project to use. This template only rendered a square with a texture and I have added to or altered the entirety of the project bar the buffer generations which remain the same for most render projects. I also added and implemented the additional package DEAR imgui for the GUI interface [1]. All of the calculations, logic and implementation for this idea were constructed by me alone.


## The Maths:
Perpendicular Gradient (Pg) = -1/Gradient   (Gradient being change in Y / change in X)
By visualising the cursor as (0,0) and the vectors we want as corners of triangles, we can say the hypotenuse is half the scale (as we go half down and half up for a total width = the scale). Then the change in y and change in x create a right-angled triangle with the hypotenuse. The Perpendicular gradient gives a relationship between X and Y, so we can replace with PgX.

Using Pythagoras’s Theorem: (scale/2)^2 = Pgx ^2 + x^2  <- substitute x for y in a^2 + b^2 = c^2
Factor out common x^2: x^2(Pg^2 + 1) = (scale/2)^2
Construct Quadratic equation: (Pg ^ 2 + 1)x^2 + 0x - (scale/2)^2 = 0

a = (Pg^2 + 1); b = 0; c = - (scale/2)^2
x = (-b +- sqrt(b^2 – 4ac)) /2a  <- c will always be negative, so by taking it from 0 it's guaranteed that the sqrt will always have a positive value.

x1 = sqrt(– 4ac)/2a	y1 = Pg * x1
x2 = -sqrt(– 4ac)/2a 	y2 = Pg * x2

[1] – Cornut, Omar. DEAR IMGUI. Available At: https://github.com/ocornut/imgui
[2] –Deslauriers, Matt. 09/03/2015. Drawing Lines is Hard. Available At: https://mattdesl.svbtle.com/drawing-lines-is-hard
