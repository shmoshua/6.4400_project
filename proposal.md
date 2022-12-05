# Project Proposal

Sangbeom Park, Cella Florescu

## Marching Cubes for Terrain Elevation

### Overview
While researching isosurface extraction, we have learned that it can be used to create cool looking terrain surfaces. The example surfaces in [this video] show that these surfaces can be used to generate landscapes, underwater caves and the basis of many 3D suface generation used in games and movies. 

We first aim to implement the marching cubes algorithm as descripted in [this paper]. Our goal is then to use the algorithm to generate interesting 3D surfaces, especially terrains. To generate a terrain model using marching cubes, we will have to find the appropriate function that produces such a terrain as an isosurface. We can also color the surface depending on the elevation to add some taste to the model as well. An interesting experiment would also be trying to come up with different functions that could possibly model various types of terrain, e.g., caves, mountains.

Once we have a terrain model, the next goal is to parametrize the elevation. We can implement this as a slider so that the user can interesting random terrains depends on the parameter given. If we have time after the implementation, we could also add more gimmicks such as a moving camera. This should allow the user to "travel" above the terrain surface like a drone view. Additionally, if time permits, we could add infinite terrain generation while the user is panning across the generated landscape.

For the code, we plan to build on the skeleton of Assignment 0~2 for mesh shading and rendering. We can also make use of the sliders in Assignment 2 for our elevation parameter, which essentially controls the cutoff value for the scalar function that generates the terrain.

### Demonstration of the Product

For the presentation, we can do a demo of the terrain surface generation. We can also show the basics of the marching cubes algorithm using other simple functions that generate simpler shapes. Once we have the parametrization done, we could also demonstrate the change of surface shape depending on the elevation parameter. If the optional deliverables are implemented, we could showcase their workings via a short video that includes the camera moving through the scene and the generation of the terrain when the border is reached.


[this paper]: https://people.eecs.berkeley.edu/~sequin/CS285/PAPERS/Lorensen_Cline.pdf

[this video]: https://www.youtube.com/watch?v=M3iI2l0ltbE


### Deliverables
- Correct implementation of the marching cube algorithm;
- Generation of a terrain model using marching cube algorithm and noise;
- Parametrization of elevation with a slider to acquire a different form of terrain;
- (Optional) Added interactions with the scene, e.g. moving camera;
- (Optional) Infinite terrain generation while moving the camera.
