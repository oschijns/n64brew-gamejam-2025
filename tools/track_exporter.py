#!/usr/bin/env python3

import os.path
import sys
import bpy
import struct
import numpy as np
import itertools

from typing    import Self, Any
from bpy.types import Vector, Point, Spline, BezierSplinePoint


# Define a 3D vector
class Vec3:

    # Create a vector from raw data
    def __init__(self, coords: Any):
        self.coords = np.array(coords)

    # Create a vector3 from a Blender vector type
    @staticmethod
    def from_blender(vec: Vector) -> Self:
        return Self(vec.xzy)

    # +
    def __add__(self, other: Self) -> Self:
        return Self(self.coords + other.coords)

    # -
    def __sub__(self, other: Self) -> Self:
        return Self(self.coords - other.coords)

    # *
    def __mul__(self, other: float) -> Self:
        return Self(self.coords * other)

    # /
    def __truediv__(self, other: float) -> Self:
        return Self(self.coords / other)

    # serialize the data as a sequence of bytes
    def serialize(self) -> bytes:
        return struct.pack(">fff", *self.coords)

    # serialize the data as a sequence of bytes
    def serialize_normal(self) -> bytes:
        coords = np.array(self.coords * 127.0, dtype=np.int8)
        return struct.pack('>bbb', *coords)

    # Normalize the vector
    def normalize(self) -> Self:
        self.coords = self.coords / np.linalg.norm(self.coords)
        return self

    # Rotate the vector around the provided axis
    def rotate_around(self, rot_axis: Self, angle: float) -> Self:
        vec   = self.coords
        axis  = rot_axis.coords
        sin   = np.sin(angle)
        cos   = np.cos(angle)
        dot   = np.dot  (axis, vec)
        cross = np.cross(axis, vec)

        return Self(
            vec   * cos + 
            cross * sin + 
            axis  * dot * (1.0 - cos)
        )



# Define curve data such as normal and width
class SectionData:

    # create a section from raw data
    def __init__(self, normal: Any, width: float):
        self.normal = Vec3(normal)
        self.width  = width

    # Create a section data from a Blender BezierSplinePoint type
    @staticmethod
    def from_blender(pt: BezierSplinePoint) -> Self:
        up   = Vec3(0.0, 1.0, 0.0)
        pt0  = Vec3.from_blender(pt.co          )
        pt1  = Vec3.from_blender(pt.handle_right)
        axis = (pt1 - pt0).normalize()

        return Self(up.rotate_around(axis, pt.tilt), pt.radius)

    # serialize the data as a sequence of bytes
    def serialize(self) -> bytes:
        return self.normal.serialize_normal() + struct.pack(">f", self.width)



# Track read from the Blender curve and to be serialized
class Track:

    # Constructor for a track
    def __init__(self, spline: Spline):
        # Storage for the data to read
        self.points  : list[Vec3       ] = []
        self.sections: list[SectionData] = []

        # iterate over every bezier points two by two
        for pt0, pt1 in pairwise(spline.bezier_points):
            # get the three control points
            self.points  .append(Vec3.from_blender(pt0.co          ))
            self.points  .append(Vec3.from_blender(pt0.handle_right))
            self.points  .append(Vec3.from_blender(pt1.handle_left ))
            self.sections.append(SectionData.from_blender(pt0))

    # serialize the data as a sequence of bytes
    def serialize(self) -> bytes:
        # We should have 3 control points for every section of the curve
        assert len(self.points) == len(self.sections) * 3

        # sequence of bytes to return
        serial: bytes = bytes()

        # Encode the number of elements as unsigned 16-bits integer
        serial += struct.pack(">H", len(self.sections))

        # Encode the control points then encode the section data
        for point   in self.points  : serial += point  .serialize()
        for section in self.sections: serial += section.serialize()

        # Return the generated file
        return serial



# iterate list by overlapping pairs
def pairwise(iterable):
    a, b = itertools.tee(iterable)
    next(b, None)
    return zip(a, b)



"""

DATA = bpy.data
track = DATA.collections[1].objects[0]

# first, get our curve co's and handles
curve_points = []
content = "\n"
for i, spline in enumerate(track.data.splines):
    for j, point in enumerate(spline.bezier_points):
        curve_points += [b
                         for x in (point.co, point.handle_left, point.handle_right)
                         for b in struct.pack(">fff", *list((track.matrix_world @ x).xzy))]

# second, get our normals. This requires us to create a mesh
depsgraph = bpy.context.evaluated_depsgraph_get()

# materialize the mesh
track_mesh = DATA.meshes.new_from_object(track, depsgraph=depsgraph)
track_mesh_obj = DATA.objects.new("Convert", track_mesh)
assert len(
    track_mesh_obj.data.vertex_normals) > 1, "Failed to generate normals for curve"

normals = []
for norm in track_mesh_obj.data.vertex_normals:
    normals += struct.pack(">fff", *
                           list((track.matrix_world @ norm.vector).xzy))

DATA.objects.remove(track_mesh_obj)

# todo: Take this as a cli param
base_path = r'\\wsl.localhost\Ubuntu\home\ischweer\dev\n64dev'
if len(sys.argv) > 1:
    print(sys.argv[-1])
    base_path = sys.argv[-1]

base_path = os.path.join(base_path, "track_layout.bin")

with open(base_path, "wb") as fh:
    fh.write((1).to_bytes(1, byteorder='little'))  # version header
    # number of points per object
    fh.write((3).to_bytes(1, byteorder='little'))
    # number of floats upcoming
    print(f"{len(curve_points)} bytes written for control points")
    print("First 6 points : ", struct.unpack(
        ">"+"f"*6, bytes(curve_points[0:24])))
    fh.write((len(curve_points) // 4).to_bytes(2, byteorder='little'))
    fh.write(bytes(curve_points))
    print(f"{len(normals)} bytes written for normals points")
    print("First 6 normals: ", struct.unpack(">"+"f"*6, bytes(normals[0:24])))
    fh.write((len(normals) // 4).to_bytes(4, byteorder='little'))
    fh.write(bytes(normals))
    print("Bytes written: ", fh.tell())

"""