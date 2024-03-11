# A.k.a. Minecratf chunking

Maincraft is 3d, for now, this project is like a fake 3d.
So the chunking will be 2d.
    
## Chunking levels

I think I can define 3 scopes of chunking:

1.  The pngs for the minimap are already given by [OSM](https://wiki.openstreetmap.org/wiki/Slippy_map_tilenames). 
    What OSM does is splitting the world map into a 2d grid, which size varies with the "zoom" level.
    i.e.:
    - zoom:0, splits the world into `2⁰ = 1` chunks (the whole world).
    - zoom:1, splits the world into `2¹ = 2` chunks in the x axis and y axis.
        Hence the world is divided into 4 chunks. (0,0), (0,1), (1,0), (1,1)
    ...
    - zoom:13 splits the world into `2¹³ = 8192` chunks in the x and y axis.
    ```
    https://a.tile.openstreetmap.org/{z}/{x}/{y}.png
    ```
    The function has the signature: ` (lat , lon , zoom) -> (x, y) `. Where `x` and `y` are the coordinates of the chunk in the grid.
    [OSM wiki function for this conversion](https://wiki.openstreetmap.org/wiki/Slippy_map_tilenames#Derivation_of_tile_names):

2.  road vertexes/textures, which are a function, kinda:
    ` (wayPoints, wayProperties) -> ( roadTexture, roadVertexes)`
    `wayProperties` are the [tags](https://wiki.openstreetmap.org/wiki/Tags)

3.  For path solving, there has to be a chunking of the graphs, although, I'm not quite sure on this one.

4.  Collision objects, which has to be much less than the road vertexes.
    Otherwise the collision detection will be too slow. At least, for the roads, this can be a subset of the road vertexes. Not that hard.

## Do I need a spatial database?
Nah
__Not yet__

## Implementation

1. Minimap
    1. First iteration: 
        - [X] Load the png from the internet
        - [X] Display the png in a corner of the screen
        - [ ] Make the minimap move with the player



    

    

    
    
    
    







    
    
