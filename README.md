# InfiniteFRI #
## What is this project about? ##
This UE5 project is a procedural generator of 3D structures using the Wave Function Collapse (WFC) algorithm. Specifically, it generates certain parts of the FRI (Facilty of Computer and Information Science in Ljubljana) building.

## What is the project made of? ##
The majority of files belong to the Unreal Engine 5 project, which contains code for the WFC algorithm and a offers a playable demo to test the generator yourself.

Then there are 2 .blend files from Blender (free 3D modelling software), where FriTiles.blend contains 3d models of parts of the FRI building, and Tiles.blend contains the FRI building put together from the custom parts and a script to analyze the structure.

## How was the project made? ##
Before modelling the parts, I had to take a walk through the FRI building and capture many photos as reference images.

![imagesAndMore](https://github.com/AlenLeban/InfiniteFRI/assets/99842431/377a1436-0094-4b69-87be-437233e9311f)

### Blender part ###

Because I had to decompose the whole building into equally sized square parts (a requirement for the WFC algorithm), I modelled the hallways, stairs, chairs and other structures with square bounds in mind, also making sure that if put together, 
they would connect as seamlessly as possible. For texturing the models, I simply projected the reference images onto the surfaces to speed up the whole workflow.

![image](https://github.com/AlenLeban/InfiniteFRI/assets/99842431/d1ce29db-c0a8-4fd9-9223-4ded6a5af28f)

After modelling, in Tile.blend file I imported those parts and put the back together into something that resembled the original FRI. In the picture you can notice lots of purple squares floating around, which mark the presence of an "empty" tile, important for 
the WFC algorithm to make sure there are empty spaces between generated parts.

![FRIBackTogether](https://github.com/AlenLeban/InfiniteFRI/assets/99842431/d9d17e49-83a8-4aa7-b36a-fe658854bfad)

WFC Algorithm works by solving a grid of cells with possible states by eliminating impossible combinations of neighboring cells using beforehand provided data about the cells and their neighbors. To generate this data I used a custom python script in the Tiles.blend file
to loop through all selected FRI parts, count their neighbors, normalized counts into probabilities and exported it as a .json file. 

![image](https://github.com/AlenLeban/InfiniteFRI/assets/99842431/033cc9c3-362f-4c07-9886-dc821fa47368)
![image](https://github.com/AlenLeban/InfiniteFRI/assets/99842431/25143ec0-1caf-456f-9b10-35282fbda672)

### Unreal Engine 5 part ###

The final step was to import all FRI parts and the .json file into my UE5 project.

My version of WFC algorithm is generalized for 3D generation on a grid of infinite size, but generated in smaller bounded chunks. This allows us generate larger areas by spawning generating chunks that overlap with existing generated chunks to ensure connectivity.
Because each chunk generates inside the bounds only, 






