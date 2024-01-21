# MIT License
# 
# Copyright (C) 2018-2024, Tellusim Technologies Inc. https://tellusim.com/
# 
# Permission is hereby granted, free of charge, to any person obtaining a copy
# of this software and associated documentation files (the "Software"), to deal
# in the Software without restriction, including without limitation the rights
# to use, copy, modify, merge, publish, distribute, sublicense, and/or sell
# copies of the Software, and to permit persons to whom the Software is
# furnished to do so, subject to the following conditions:
# 
# The above copyright notice and this permission notice shall be included in all
# copies or substantial portions of the Software.
# 
# THE SOFTWARE IS PROVIDED "AS IS", WITHOUT WARRANTY OF ANY KIND, EXPRESS OR
# IMPLIED, INCLUDING BUT NOT LIMITED TO THE WARRANTIES OF MERCHANTABILITY,
# FITNESS FOR A PARTICULAR PURPOSE AND NONINFRINGEMENT. IN NO EVENT SHALL THE
# AUTHORS OR COPYRIGHT HOLDERS BE LIABLE FOR ANY CLAIM, DAMAGES OR OTHER
# LIABILITY, WHETHER IN AN ACTION OF CONTRACT, TORT OR OTHERWISE, ARISING FROM,
# OUT OF OR IN CONNECTION WITH THE SOFTWARE OR THE USE OR OTHER DEALINGS IN THE
# SOFTWARE.

import math

from tellusim import *

#
# scene
#
scene = None
graph = None
box_mesh = None
sphere_mesh = None
cylinder_mesh = None
root_material = None

#
# create bodies
#
def create_body(shape, mesh, size, transform, density, friction, restitution, color):
	
	# create body
	body = BodyRigid(scene)
	body.setInternal(True)
	
	# add shape
	shape.setDensity(density)
	shape.setFriction(friction)
	shape.setRestitution(restitution)
	body.addShape(shape)
	
	# create node
	node = NodeObject(graph, mesh, body)
	node.setPivotTransform(Matrix4x3f.scale(size))
	node.setGlobalTransform(transform)
	node.setInternal(True)
	
	# create material
	material = Material(parent = root_material)
	material.setUniform('diffuse_0_color', color)
	material.setUniform('diffuse_1_color', color * 0.3)
	material.setUniform('grid_size', 1.0 / size)
	material.setInternal(True)
	node.addMaterial(material)
	
	# update graph
	graph.updateSpatial()
	graph.updateScene()
	
	return body

def create_box(size, transform, density = 1.0, friction = 0.5, restitution = 0.5, color = Color.white):
	shape = ShapeBox()
	shape.setSize(size)
	return create_body(shape, box_mesh, size, transform, density, friction, restitution, color)

def create_sphere(radius, transform, density = 1.0, friction = 0.5, restitution = 0.5, color = Color.white):
	shape = ShapeSphere()
	shape.setRadius(radius)
	return create_body(shape, sphere_mesh, Vector3f(radius), transform, density, friction, restitution, color)

def create_cylinder(radius, height, transform, density = 1.0, friction = 0.5, restitution = 0.5, color = Color.white):
	shape = ShapeCylinder()
	shape.setRadius(radius)
	shape.setHeight(height)
	return create_body(shape, cylinder_mesh, Vector3f(radius, radius, height), transform, density, friction, restitution, color)

#
# create scene
#
def create(s):
	
	global scene
	global graph
	global box_mesh
	global sphere_mesh
	global cylinder_mesh
	global root_material
	
	scene = s
	
	# get resources
	graph = scene.getGraph('Graph')
	box_mesh = scene.getObject('Box Mesh')
	sphere_mesh = scene.getObject('Sphere Mesh')
	cylinder_mesh = scene.getObject('Cylinder Mesh')
	root_material = scene.getMaterial('Checkerboard Material')
	
	# create scene
	create_cylinder(0.2, 2.0, Matrix4x3d.translate(0.0, 0.0, 0.2) * Matrix4x3d.rotateX(90.0), density = 10.0)
	create_box(Vector3f(8.0, 1.0, 0.3), Matrix4x3d.translate(0.0, 0.0, 0.6))
	
	for z in range(0, 7):
		create_cylinder(0.5, 1.0, Matrix4x3d.translate(-3.0 + z, 0.0, 2.0) * Matrix4x3d.rotateX(90.0), color = Color(0.1, 1.0, 0.1))
	
	create_box(Vector3f(1.0), Matrix4x3d.translate(-3.0, 0.0, 4.0), color = Color(1.0, 0.1, 0.1))
	
	create_sphere(0.75, Matrix4x3d.translate(3.0, 0.0, 8.0), density = 10.0, color = Color.red)
	
	for x in range(0, 40):
		for z in range(0, x + 1):
			k = x * 0.17 + z * 0.13
			create_box(Vector3f(0.5), Matrix4x3d.translate(Vector3d(x - z * 0.5 - 20.0, 16.0, z + 1.0) * 0.5), color = Color(math.cos(k), 1.0, math.cos(k + 0.7)) * 0.4 + 0.6)
	
	for x in range(0, 20):
		for z in range(0, x + 1):
			k = x * 0.17 + z * 0.13
			create_cylinder(0.25, 0.5, Matrix4x3d.translate(Vector3d(x - z * 0.5 - 10.0, 8.0, z + 1.0) * 0.5), color = Color(math.cos(k), math.cos(k + 0.3), 1.0) * 0.4 + 0.6)
