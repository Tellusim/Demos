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

#include <core/TellusimLog.h>
#include <math/TellusimRandom.h>
#include <platform/TellusimBuffer.h>
#include <platform/TellusimKernel.h>
#include <platform/TellusimCompute.h>
#include <platform/TellusimDevice.h>
#include <scene/TellusimScenes.h>
#include <scene/TellusimObject.h>
#include <scene/TellusimNodes.h>

using namespace Tellusim;

layout(instance = GraphAsteroids);

/*
 */
class GraphAsteroids: public GraphScript {
		
	public:
		
		GraphAsteroids(void *ptr) : GraphScript(ptr) {
			
			TS_LOGF(Message, "GraphAsteroids::GraphAsteroids(): %p\n", this);
		}
		~GraphAsteroids() {
			
			TS_LOGF(Message, "GraphAsteroids::~GraphAsteroids(): %p\n", this);
			
			clear();
		}
		
		/*
		 */
		virtual void update() {
			
			// create asteroids
			if(num_indices == 0) {
				if(!create(getDevice())) {
					TS_LOG(Error, "GraphAsteroids::update(): can't create Asteroids\n");
					return;
				}
			}
			
			// transform asteroids
			if(transform_kernel) {
				
				// scene storage buffer
				Scene scene = getScene();
				SceneManager scene_manager = scene.getManager();
				Buffer scene_storage_buffer = scene_manager.getStorageHeapBuffer();
				
				// create command list
				Device device = getDevice();
				Compute compute = device.createCompute();
				
				// transform parameters
				// time previous scene time is used because the scene time has been updated
				// but our camera transformation is from the previous frame
				// using current time can lead to asteroids jittering
				struct TransformParameters {
					uint32_t node_address;
					uint32_t num_indices;
					float32_t time;
				};
				TransformParameters transform_parameters;
				transform_parameters.node_address = getNodeAddress();
				transform_parameters.num_indices = num_indices;
				transform_parameters.time = time;
				time = (float32_t)scene.getTime();
				
				// set transform kernel
				compute.setKernel(transform_kernel);
				compute.setUniform(0, transform_parameters);
				compute.setStorageBuffer(0, indices_buffer);
				compute.setStorageBuffer(1, scene_storage_buffer);
				compute.dispatch(num_indices);
				compute.barrier(scene_storage_buffer);
				
				// update graph
				updateLightTree();
				updateObjectTree();
				updateScene();
			}
		}
		
	private:
		
		/*
		 */
		bool create(const Device &device) {
			
			// gravity graph
			Scene scene = getScene();
			Graph gravity_graph = scene.getGraph("Gravity");
			if(!gravity_graph) {
				TS_LOG(Error, "GraphAsteroids::create(): can't get graphs\n");
				return false;
			}
			
			// get gravity nodes
			Node node_sun = gravity_graph.getNode("Sun");
			Node node_galaxy = gravity_graph.getNode("Galaxy");
			Node node_depth = gravity_graph.getNode("Earth_Depth");
			Node node_surface = gravity_graph.getNode("Earth_Surface");
			if(!node_sun || !node_galaxy || !node_depth || !node_surface) {
				TS_LOG(Error, "GraphAsteroids::create(): can't get nodes\n");
				return false;
			}
			
			// get scene asteroids
			Array<Object> asteroids;
			for(uint32_t i = 0; i < Maxu16; i++) {
				String name = String::format("Asteroid_%02u", i);
				uint32_t index = scene.findObject(name.get());
				if(index == Maxu32) break;
				Object object = scene.getObject(index);
				if(object) asteroids.append(object);
			}
			if(!asteroids) {
				TS_LOG(Error, "GraphAsteroids::create(): can't find asteroids\n");
				return false;
			}
			
			// motion hash
			uint32_t hash = 0;
			hash = node_galaxy.setMotionHash(hash);
			hash = node_surface.setMotionHash(hash);
			hash = node_depth.setMotionHash(hash);
			node_galaxy.updateScene();
			node_surface.updateScene();
			node_depth.updateScene();
			
			// create asteroids
			Random<> random(0u);
			uint32_t num_asteroids = 200000;
			Array<uint32_t> indices_data(num_asteroids);
			for(uint32_t i = 0; i < num_asteroids; i++) {
				Object &object = asteroids[random.geti32(0, asteroids.size() - 1)];
				NodeObject node_object(*this, object);
				indices_data[i] = node_object.getIndex();
				hash = node_object.setMotionHash(hash);
				node_object.setInternal(true);
			}
			
			// create indices buffer
			indices_buffer = device.createBuffer(Buffer::FlagStorage, indices_data.get(), indices_data.bytes());
			if(!indices_buffer) {
				TS_LOG(Error, "GraphAsteroids::create(): can't create indices buffer\n");
				return false;
			}
			num_indices = indices_data.size();
			
			// create transform kernel
			transform_kernel = device.createKernel().setUniforms(1).setStorages(2);
			transform_kernel.loadShaderGLSL("shaders/transform.shader", "COMPUTE_SHADER=1");
			if(!transform_kernel.create()) {
				TS_LOG(Error, "GraphAsteroids::create(): can't create transform kernel\n");
				return false;
			}
			
			return true;
		}
		
		/*
		 */
		void clear() {
			
			// release nodes
			releaseNodes();
			updateScene();
			
			// clear resources
			indices_buffer.clear();
			transform_kernel.clearPtr();
			
			num_indices = 0;
		}
		
		float32_t time = 0.0f;
		
		Buffer indices_buffer;
		Kernel transform_kernel;
		uint32_t num_indices = 0;
};
