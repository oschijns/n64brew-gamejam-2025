# Design document

Game Jam theme: **REPAIR**

## Project description

A racing game where contestants can damage each other.
The game should have elements of Team Fight Tactics with a game-loop split into two phases.

In the first phase, you earn or buy repairs for your vehicule.
Those repairs influence the stats of your vehicule for the next phase.

In the second phase, the constestants race.
The racing is done automatically with few timed actions to perform.


## Technical aspect

### Shop / Garage phase



### Racing phase

Tracks are defined as chains of cubic Bézier curves (sections).
Either as a closed loop or as a opened trail.

#### Data format

The track model is loaded from a binary file with the following data layout:

| Size            | Element                | Description |
|----------------:|------------------------|-------------|
|               1 | Version                |             |
|               1 | Configuration          | Bitmask     |
|               2 | Number of sections (N) | 16-bits unsigned integer |
| N *  3 * 3 * 4  | Control points         | Position of the control points using floating point values |
| N * (3 * 4 + 4) | Section Data           | Normal and width of the track at the beginning of this section |


_note: Binary file format is subject to change to incorporate additional features, such as assets to place in the world._


Assuming we have a circuit defined as this:
```txt
      D2 -- A0 -- A1
     /             \
    D1              A2
    |               |
    D0              B0
    |               |
    C2              B1
     \             /
      C1 -- C0 -- B2
```
Then we have 4 sections: **A**, **B**, **C**, **D**
Each section has 4 control points:
- Section **A**: `[ A0, A1, A2, B0 ]`
- Section **B**: `[ B0, B1, B2, C0 ]`
- Section **C**: `[ C0, C1, C2, D0 ]`
- Section **D**: `[ D0, D1, D2, A0 ]`


#### Processing

This data layout allow optimizing the memory necessary to load the whole track.
But it is not usable as is neither for rendering nor physics calculations.
We need to sample a section of the track to convert it from a parametric Bézier curve into a sequence of linear segments.

As a sequence of segments, it is trivial to compute an approximation of the total length of the section or to figure out at which position is the vehicule along the section.
We also compute the normal, binormal and the width of the track at each sampled point.
The extra data provide us means to generate a corresponding mesh at runtime and do physics calculations.


#### Rendering

Rendering is done using _Tiny3D_.
When generating a `SampledSection` we also build a `rspq_block_t` using `build_render_commands` containing the vertex data of the mesh for this section.

Mesh data submitted to the **RSP** must follow the `T3DVertPacked` data layout where two distinct vertices are intertwined at once.
Since we are drawing a road, we use a strip of triangles to render it.
Then we simply need a constant array of indexes `RSPQ_INDEXES` and the sequence of intertwined vertices.


#### Physics

The track can support variable width and tilted surface.

Since the vehicules are following a given track, we can make a few assumptions.
They can only go in two general direction: forward or backward.
Side motions from the road center don't really matter.

We can keep track of which section and which segment of a section the vehicule is currently on.
This simplify finding where the vehicule will be next as we can start looking from a known section and from a known segment.

Vehicules use a simple sphere with a variable radius as there collision shape.
This simplyfing collision detection between vehicules and the road.
We don't need to build a **BVH** as there are too few objects to make it worth it.
Thus collision detection between vehicules will be _O(n²)_.
