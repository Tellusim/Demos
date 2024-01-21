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

#include <physics/jolt/source/TellusimJolt.cpp>

#pragma cflags(-std=c++17)
#pragma include(ROOT/plugins/physics/jolt/extern)
#pragma library(ROOT/plugins/physics/jolt/extern/lib/ARCH/Jolt)

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
				Scene scene = getScene();
				physics->update();
				physics->update(scene);
			}
		}
		
		virtual void dispatch() {
			
			// create physics
			if(!initialized) {
				initialized = true;
				Scene scene = getScene();
				if(!scene.isImmutable()) {
					physics = makeAutoPtr(new Jolt());
					physics->create(getScene());
				}
			}
		}
		
	private:
		
		bool initialized = false;
		
		AutoPtr<Jolt> physics;
};
