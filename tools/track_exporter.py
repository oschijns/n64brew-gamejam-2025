#!/usr/bin/env python3

import bpy
import json
import struct
import itertools

from pathlib   import Path
from typing    import Self, Any
from mathutils import Vector, Quaternion
from bpy.types import Operator, Panel, Spline, BezierSplinePoint
from bpy.props import StringProperty


# object to look for in the blender project
# and where to write it as a binary file
TRACK_NAME : str  = "TRACK"
OUTPUT_PATH: Path = Path("/home/oschijns/Projects/n64")
AUDIT_FILE : str  = "track.json"
BINARY_FILE: str  = "track.bin"


# MARK: functions

# iterate list by overlapping pairs
def pairwise(iterable):
    a, b = itertools.tee(iterable)
    next(b, None)
    return zip(a, b)

# reorder 3D vector components from Blender's coordinate system to libdragon's one
def reorder_coords(vec: Vector) -> Vector:
    return vec.xzy

# serialize the data as a sequence of bytes
def serialize_point(vec: Vector) -> bytes:
    return struct.pack(">fff", *vec)

# serialize the data as a sequence of bytes
def serialize_normal(vec: Vector) -> bytes:
    norm: Vector = vec * 127.0
    return struct.pack('>bbb', int(norm.x), int(norm.y), int(norm.z))

# convert to json
def vector_to_json(vec: Vector) -> dict[str, float]:
    return {
        'x': vec.x,
        'y': vec.y,
        'z': vec.z,
    }



# MARK: Section Data

# Define curve data such as normal and width
class SectionData:

    # create a section from raw data
    def __init__(self, normal: Vector, width: float):
        self.normal: Vector = normal
        self.width : float  = width


    # Create a section data from a Blender BezierSplinePoint type
    @classmethod
    def from_blender(clazz: type[Self], pt: BezierSplinePoint) -> Self:
        up  : Vector     = Vector((0.0, 1.0, 0.0))
        pt0 : Vector     = reorder_coords(pt.co          )
        pt1 : Vector     = reorder_coords(pt.handle_right)
        axis: Vector     = (pt1 - pt0).normalized()
        quat: Quaternion = Quaternion(axis, pt.tilt)
        up.rotate(quat)
        return clazz(up, pt.radius)


    # serialize the data as a sequence of bytes
    def serialize(self) -> bytes:
        return serialize_normal(self.normal) + struct.pack(">f", self.width)


    # convert to json
    def to_json(self) -> dict[str, Any]:
        return {
            'normal': vector_to_json(self.normal),
            'width' : self.width,
        }



# MARK: Track

# Track read from the Blender curve and to be serialized
class Track:

    # Constructor for a track
    def __init__(self, points: list[Vector], sections: list[SectionData]):
        # Storage for the data to read
        self.points  : list[Vector     ] = points
        self.sections: list[SectionData] = sections


    # Create a track object from a Blender Spline
    @classmethod
    def from_blender(clazz: type[Self], spline: Spline) -> Self:
        # Storage for the data to read
        points  : list[Vector     ] = []
        sections: list[SectionData] = []

        # iterate over every bezier points two by two
        for pt0, pt1 in pairwise(spline.bezier_points):
            # get the three control points
            points  .append(reorder_coords(pt0.co          ))
            points  .append(reorder_coords(pt0.handle_right))
            points  .append(reorder_coords(pt1.handle_left ))
            sections.append(SectionData.from_blender(pt0))

        return clazz(points, sections)


    # serialize the data as a sequence of bytes
    def serialize(self) -> bytes:
        # We should have 3 control points for every section of the curve
        assert len(self.points) == len(self.sections) * 3

        # sequence of bytes to return
        serial: bytes = bytes()

        # Encode the number of elements as unsigned 16-bits integer
        serial += struct.pack(">H", len(self.sections))

        # Encode the control points then encode the section data
        for point in self.points:
            serial += serialize_point(point)
        for section in self.sections:
            serial += section.serialize()

        # Return the generated file
        return serial


    # convert to json
    def to_json(self) -> dict[str, list]:
        return {
            'points'  : [vector_to_json(p) for p in self.points  ],
            'sections': [s.to_json()       for s in self.sections],
        }


    # write the serialization of this track to a file
    def write_to_file(self, file_path: Path):
        header: bytes = struct.pack('>BB', 1, 3)
        serial: bytes = self.serialize()
        with open(file_path, 'wb+') as file:
            file.write(header)
            file.write(serial)



# MARK: file handling

# Read a bezier curve in the Blender scene and write it to a file
def process(
        track_name : str, 
        output_path: Path,
        binary_file: str,
        audit_file : str,
    ):
    blend_obj = bpy.context.scene.objects[track_name]
    track: Track = Track.from_blender(blend_obj.data.splines[0])

    # where to write the files
    path_bin : Path = output_path / binary_file
    path_json: Path = output_path / audit_file

    # write a JSON representation of the file to audit it
    with open(path_json, 'w+') as file:
        file.write(json.dumps(track.to_json(), indent=4))

    # write the actual binary file
    track.write_to_file(path_bin)



# MARK: Blender Operator

class N64BREW_OT_exporter(Operator):
    bl_idname: str = "n64brew.exporter"
    bl_label : str = "N64 Track Export"

    # Name of the spline object to serialize
    spline_name: StringProperty(name = "Spline Name", default = "TRACK")

    # Select a directory to write the files
    directory: StringProperty(
        name    = "Directory",
        subtype = 'DIR_PATH'
    )

    # Name of the two files to generate
    outname_bin : StringProperty(name = "Output name binary", default = "track.bin")
    outname_json: StringProperty(name = "Output name audit" , default = "track.json")

    def invoke(self, context, event):
        return context.window_manager.invoke_props_dialog(self)

    def draw(self, context):
        layout = self.layout
        layout.prop(self, "spline_name" )
        layout.prop(self, "directory"   )
        layout.prop(self, "outname_bin" )
        layout.prop(self, "outname_json")

    def execute(self, context):
        process(self.spline_name, Path(self.directory), self.outname_bin, self.outname_json)
        return {'FINISHED'}


# MARK: Blender Panel

class N64BREW_PT_exporter(Panel):
    bl_idname     : str = "N64BREW_PT_exporter"
    bl_label      : str = "N64 brew Track Exporter"
    bl_space_type : str = 'VIEW_3D'
    bl_region_type: str = 'UI'
    bl_category   : str = 'N64 brew'

    def draw(self, context):
        row = self.layout.row()
        row.operator(N64BREW_OT_exporter.bl_idname, text = "Export track")


# MARK: Blender register

def blender_register():
    bpy.utils.register_class(N64BREW_OT_exporter)
    bpy.utils.register_class(N64BREW_PT_exporter)

def blender_unregister():
    bpy.utils.unregister_class(N64BREW_PT_exporter)
    bpy.utils.unregister_class(N64BREW_OT_exporter)


# MARK: main

if __name__ == "__main__":
    blender_register()
