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

#include <binding/python/source/TellusimPyBase.cpp>
#include <binding/python/source/TellusimPyMath.cpp>
#include <binding/python/source/TellusimPyAPI.cpp>
#include <system/python/source/TellusimPython.cpp>

#pragma cflags($(shell python3-config --includes))
#pragma ldflags($(shell python3-config --ldflags --libs --embed))

using namespace Tellusim;

layout(instance = GraphPython);

/*
 */
class GraphPython: public GraphScript {
		
	public:
		
		GraphPython(void *ptr) : GraphScript(ptr) {
			
			TS_LOGF(Message, "GraphPython::GraphPython(): %p\n", this);
		}
		~GraphPython() {
			
			TS_LOGF(Message, "GraphPython::~GraphPython(): %p\n", this);
		}
		
		/*
		 */
		virtual void update() {
			
			// load script
			if(!initialized) {
				initialized = true;
				python = makeAutoPtr(new Python());
				if(python->load("python")) {
					if(python->isFunction("create")) {
						Scene scene = getScene();
						python->run("create", scene);
					}
				} else {
					python.clear();
				}
			}
			
			// run update function
			if(initialized && python && python->isFunction("update")) {
				Scene scene = getScene();
				python->run("update", scene);
			}
		}
		
		virtual void dispatch() {
			
			// run dispatch function
			if(initialized && python && python->isFunction("dispatch")) {
				Scene scene = getScene();
				python->run("dispatch", scene);
			}
		}
		
	private:
		
		bool initialized = false;
		
		AutoPtr<Python> python;
};
