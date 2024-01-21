// MIT License
// 
// Copyright (C) 2018-2024, Tellusim Technologies Inc. https://tellusim.com/
// 
// Permission is hereby granted, free of charge, to any person obtaining a copy
// of this software and associated documentation files (the "Software"), to deal
// in the Software without restriction, including without limitation the rights
// to use, copy, modify, merge, publish, distribute, sublicense, and/or sell
// copies of the Software, and to permit persons to whom the Software is
// furnished to do so, subject to the following conditions:
// 
// The above copyright notice and this permission notice shall be included in all
// copies or substantial portions of the Software.
// 
// THE SOFTWARE IS PROVIDED "AS IS", WITHOUT WARRANTY OF ANY KIND, EXPRESS OR
// IMPLIED, INCLUDING BUT NOT LIMITED TO THE WARRANTIES OF MERCHANTABILITY,
// FITNESS FOR A PARTICULAR PURPOSE AND NONINFRINGEMENT. IN NO EVENT SHALL THE
// AUTHORS OR COPYRIGHT HOLDERS BE LIABLE FOR ANY CLAIM, DAMAGES OR OTHER
// LIABILITY, WHETHER IN AN ACTION OF CONTRACT, TORT OR OTHERWISE, ARISING FROM,
// OUT OF OR IN CONNECTION WITH THE SOFTWARE OR THE USE OR OTHER DEALINGS IN THE
// SOFTWARE.

#include <format/TellusimMesh.h>

using namespace Tellusim;

/*
 */
Node node_sun;
Node node_earth;
Node node_galaxy;
Mesh cameras_mesh;
Array<NodeCamera> node_cameras;
Array<uint32_t> node_sun_indices;
Array<uint32_t> node_earth_indices;
Array<uint32_t> node_galaxy_indices;
Array<uint32_t> node_camera_indices;

/*
 */
struct Take {
	uint32_t index;
	float64_t scale;
	float64_t min_time;
	float64_t max_time;
};
Array<Take> takes;
uint32_t take_index = 0;
float64_t scene_time = 0.0;

/*
 */
EXPORT(create) {
	
	TS_LOG(Message, "Gravity::create(): is called\n");
	
	Scene &scene = self->scene;
	
	// graph graph
	Graph gravity_graph = scene.getGraph("Gravity");
	if(!gravity_graph) {
		TS_LOG(Error, "Gravity::create(): can't get graph\n");
		return false;
	}
	
	// get nodes
	node_sun = gravity_graph.getNode("Sun");
	node_earth = gravity_graph.getNode("Earth");
	node_galaxy = gravity_graph.getNode("Galaxy");
	if(!node_sun || !node_earth || !node_galaxy) {
		TS_LOG(Error, "Gravity::create(): can't get nodes\n");
		return false;
	}
	
	// get scene cameras
	for(uint32_t i = 0; i < Maxu16; i++) {
		String name = String::format("Camera_%u", i);
		uint32_t index = gravity_graph.findNode(name.get());
		if(index != Maxu32) node_cameras.append(NodeCamera(gravity_graph.getNode(index)));
		else break;
	}
	if(!node_cameras) {
		TS_LOG(Error, "Gravity::create(): can't find cameras\n");
		return false;
	}
	
	// load cameras mesh
	if(!cameras_mesh.load("meshes/cameras.mesh") || cameras_mesh.getNumAnimations() == 0) {
		TS_LOG(Error, "Gravity::create(): can't load cameras\n");
		return false;
	}
	
	// animation indices
	for(uint32_t i = 0; i < node_cameras.size(); i++) {
		uint32_t sun_index = cameras_mesh.findNode(String::format("Sun_%u", i).get());
		uint32_t earth_index = cameras_mesh.findNode(String::format("Earth_%u", i).get());
		uint32_t galaxy_index = cameras_mesh.findNode(String::format("Galaxy_%u", i).get());
		uint32_t camera_index = cameras_mesh.findNode(String::format("Camera_%u", i).get());
		if(sun_index == Maxu32 || earth_index == Maxu32 || galaxy_index == Maxu32 || camera_index == Maxu32) {
			TS_LOG(Error, "Gravity::create(): can't find nodes\n");
			return false;
		}
		node_sun_indices.append(sun_index);
		node_earth_indices.append(earth_index);
		node_galaxy_indices.append(galaxy_index);
		node_camera_indices.append(camera_index);
	}
	
	// camera takes
	takes = {
		// index, speed, min time, max time
		{ 0, 0.2, 0.0, 30.0 },
		{ 2, 0.4, 2.0, 31.0 },
		{ 3, 0.4, 0.0, 30.0 },
		{ 5, 0.4, 3.0, 25.0 },
		{ 6, 0.4, 2.0, 30.0 },
		{ 1, 0.4, 2.0, 30.0 },
	};
	
	return true;
}

/*
 */
EXPORT(release) {
	
	TS_LOG(Message, "Gravity::release(): is called\n");
	
	// clear cameras
	node_cameras.release();
	node_sun_indices.release();
	node_earth_indices.release();
	node_galaxy_indices.release();
	node_camera_indices.release();
	
	return true;
}

/*
 */
EXPORT(update) {
	
	Scene &scene = self->scene;
	Window &window = self->window;
	
	// update take
	float64_t take_time = scene.getTime() - scene_time;
	if(take_time > takes[take_index].max_time - takes[take_index].min_time || window.getKeyboardKey(Window::KeyReturn, true)) {
		if(take_index + 1 < takes.size()) take_index++;
		else take_index = 0;
		scene_time = scene.getTime();
		take_time = 0.0;
	}
	
	// update animation time
	const Take &take = takes[take_index];
	MeshAnimation animation = cameras_mesh.getAnimation(0);
	animation.setTime(min(take_time + take.min_time, take.max_time) * take.scale, false);
	
	// update sun transform
	node_sun.setGlobalTransform(animation.getGlobalTransform(node_sun_indices[take.index]));
	node_sun.updateScene();
	
	// update earth transform
	node_earth.setGlobalTransform(animation.getGlobalTransform(node_earth_indices[take.index]));
	node_earth.updateTransforms(true);
	
	// update galaxy transform
	node_galaxy.setGlobalTransform(animation.getGlobalTransform(node_galaxy_indices[take.index]));
	node_galaxy.updateTransforms(true);
	
	// update camera transform and exposure
	Matrix4x3d transform = animation.getGlobalTransform(node_camera_indices[take.index]);
	float32_t fade = (float32_t)max(1.0 - take_time * 2.0, 1.0 - (take.max_time - take.min_time - take_time) * 2.0, 0.0);
	for(NodeCamera &node_camera : node_cameras) {
		node_camera.getCamera().setExposureScale(-fade * 20.0f);
		node_camera.setGlobalTransform(transform);
		node_camera.updateTransforms(true);
		node_camera.updateScene();
	}
	
	return true;
}
