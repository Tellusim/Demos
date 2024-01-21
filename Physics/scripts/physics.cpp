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

#if JOLT
	#define PHYSICS Jolt
	#include <physics/jolt/source/TellusimJolt.cpp>
	#pragma include(ROOT/plugins/physics/jolt/extern)
	#pragma library(ROOT/plugins/physics/jolt/extern/lib/ARCH/Jolt)
#elif BULLET
	#define PHYSICS Bullet
	#include <physics/bullet/source/TellusimBullet.cpp>
	#pragma include(ROOT/plugins/physics/bullet/extern/include)
	#pragma library(ROOT/plugins/physics/bullet/extern/lib/ARCH/Bullet)
#else
	#define PHYSICS PhysX
	#define PX_PHYSX_STATIC_LIB 1
	#include <physics/physx/source/TellusimPhysX.cpp>
	#pragma include(ROOT/plugins/physics/physx/extern/include)
	#pragma library(ROOT/plugins/physics/physx/extern/lib/ARCH/PhysX)
#endif

using namespace Tellusim;

layout(instance = GraphPhysics);

/*
 */
class GraphPhysics: public GraphScript {
		
	public:
		
		GraphPhysics(void *ptr) : GraphScript(ptr) {
			
			TS_LOGF(Message, "GraphPhysics::GraphPhysics(): %p\n", this);
		}
		~GraphPhysics() {
			
			TS_LOGF(Message, "GraphPhysics::~GraphPhysics(): %p\n", this);
		}
		
		/*
		 */
		virtual void update() {
			
			// update physics
			if(initialized && physics) {
				uint64_t begin = Time::current();
				Scene scene = getScene();
				physics->update();
				physics->update(scene);
				simulation_time += Time::current() - begin;
				uint32_t frames = physics->getFrame() - simulation_frame;
				if(frames > 60) {
					TS_LOGF(Message, "%s\n", String::fromTime(simulation_time / frames).get());
					simulation_frame = physics->getFrame();
					simulation_time = 0;
				}
			}
		}
		
		virtual void dispatch() {
			
			// create scene
			if(!created) {
				created = true;
				create();
			}
			
			// create physics
			else if(!initialized) {
				initialized = true;
				Scene scene = getScene();
				if(!scene.isImmutable()) {
					TS_LOGF(Message, "GraphPhysics::dispatch(): create %s physics\n", TS_STRING(PHYSICS));
					physics = makeAutoPtr(new PHYSICS());
					physics->create(getScene());
				}
			}
		}
		
	private:
		
		// create scene
		void create() {
			
			Scene scene = getScene();
			Object object = scene.getObject("Box 1x1x1");
			if(!object) return;
			
			// create piramid
			uint32_t piramid_size = 20;
			for(uint32_t z = 0; z <= piramid_size; z++) {
				for(uint32_t y = 0; y <= z; y++) {
					for(uint32_t x = 0; x <= z; x++) {
						BodyRigid body = BodyRigid(scene);
						NodeObject node = NodeObject(*this, object, body);
						node.setGlobalTransform(Matrix4x3d::translate(Vector3d(x - z * 0.5, y - z * 0.5, piramid_size - z) * 1.2 + Vector3d(-40.0, 0.0, 1.0)));
						node.setInternal(true);
						body.setInternal(true);
					}
				}
			}
			
			object = scene.getObject("Box Mesh");
			if(!object) return;
			
			// create friction
			uint32_t friction_size = 10;
			for(uint32_t y = 0; y < friction_size; y++) {
				for(uint32_t x = 0; x < friction_size; x++) {
					BodyRigid body = BodyRigid(scene);
					NodeObject node = NodeObject(*this, object, body);
					node.setGlobalTransform(Matrix4x3d::translate(Vector3d(x * 1.2, y * 1.2, 0.0) + Vector3d(-20.0, -20.0, 0.5)));
					ShapeBox shape = ShapeBox(body);
					shape.setDensity(1.0f);
					shape.setFriction(1.0f - (float32_t)(friction_size * x + y) / (friction_size * friction_size));
					body.setLinearVelocity(Vector3f(0.0f, 16.0f, 0.0f));
					node.setInternal(true);
					body.setInternal(true);
				}
			}
			
			// update graph
			updateSpatial();
			updateScene();
		}
		
		bool created = false;
		bool initialized = false;
		
		uint64_t simulation_time = 0;
		uint32_t simulation_frame = 0;
		
		AutoPtr<PHYSICS> physics;
};
