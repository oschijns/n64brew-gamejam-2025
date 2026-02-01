import os.path
import bpy
import struct
import sys

D = bpy.data
track = D.collections[1].objects[0]

# first, get our curve co's and handles
all_points = [
    point for spline in track.data.splines for point in spline.bezier_points]

curve_circuit_points = []
for i in range(len(all_points)):
    first = all_points[i]
    second = all_points[(i + 1) % len(all_points)]
    print(i, (i + 1) % len(all_points))
    curve_circuit_points.extend(
        [first.co, first.handle_right, second.handle_left])
    print(curve_circuit_points[i*3:(i*3)+3])
curve_bytes = [b for x in curve_circuit_points for b in struct.pack(
    ">fff", *list((track.matrix_world @ x).xzy))]

# second, get our normals. This requires us to create a mesh
depsgraph = bpy.context.evaluated_depsgraph_get()

# materialize the mesh
track_mesh = D.meshes.new_from_object(track, depsgraph=depsgraph)
track_mesh_obj = D.objects.new("Convert", track_mesh)
assert len(
    track_mesh_obj.data.vertex_normals) > 1, \
    "Failed to generate normals for curve"

normals = []
for norm in track_mesh_obj.data.vertex_normals:
    normals += struct.pack(">fff", *
                           list((track.matrix_world @ norm.vector).xzy))

D.objects.remove(track_mesh_obj)

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
    print(f"{len(curve_bytes)} bytes written for control points")
    print("First 6 points : ", struct.unpack(
        ">"+"f"*6, bytes(curve_bytes[0:24])))
    fh.write((len(curve_bytes) // 4).to_bytes(2, byteorder='little'))
    fh.write(bytes(curve_bytes))
    print(f"{len(normals)} bytes written for normals points")
    print("First 6 normals: ", struct.unpack(">"+"f"*6, bytes(normals[0:24])))
    fh.write((len(normals) // 4).to_bytes(4, byteorder='little'))
    fh.write(bytes(normals))
    print("Bytes written: ", fh.tell())
