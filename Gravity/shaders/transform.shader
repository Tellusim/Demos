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

#version 430 core

#if COMPUTE_SHADER
	
	layout(local_size_x = 64) in;
	
	layout(std140, binding = 0) uniform TransformParameters {
		uint node_address;
		uint num_asteroids;
		float time;
	};
	
	layout(std430, binding = 1) readonly buffer IndicesBuffer { uint node_indices_buffer[]; };
	layout(std430, binding = 2) buffer StorageBuffer { vec4 scene_storage_buffer[]; };
	
	/*
	 */
	#define mat4x3_mul_func					1
	#define mat4x3_compose_func				1
	#define mat4x3_rotate_z_func			1
	#define quat_rotate_zyx_func			1
	#define set_node_global_transform_func	1
	#include <SceneNodes.shader>
	#include <SceneMath.shader>
	
	/*
	 */
	float halton2(uint i) {
		uint bits = (i << 16u) | (i >> 16u);
		bits = ((bits & 0x00ff00ffu) << 8u) | ((bits & 0xff00ff00u) >> 8u);
		bits = ((bits & 0x0f0f0f0fu) << 4u) | ((bits & 0xf0f0f0f0u) >> 4u);
		bits = ((bits & 0x33333333u) << 2u) | ((bits & 0xccccccccu) >> 2u);
		bits = ((bits & 0x55555555u) << 1u) | ((bits & 0xaaaaaaaau) >> 1u);
		return float(bits) * 2.3283064365386963e-10f;
	}
	
	/*
	 */
	void main() {
		
		uint global_id = gl_GlobalInvocationID.x;
		
		[[branch]] if(global_id < num_asteroids) {
			
			float k = halton2(global_id);
			
			float noise_0 = fract(sin(global_id * 17.31713f) * 13731.13731f);
			float noise_1 = fract(sin(global_id * 13.71317f) * 17371.17371f);
			float noise_2 = fract(sin(global_id * 37.13717f) * 37137.37137f);
			float noise = (noise_0 + noise_1 + noise_2) / 3.0f;
			
			float offset = 1337.0f * k + noise * 173.0f;
			float range = 2001.0f * k + cos(noise * 113.0f) * 173.0f;
			float angle = time * (noise + 1.4f - k) * 0.011f;
			
			vec3 scale = vec3(1.3f + sin(noise * 117.3f) * 0.4f);
			float rotate_x = k * 31.31f + sin(noise_0 * 31.7f) * time * 0.31f;
			float rotate_y = k * 13.13f + sin(noise_1 * 13.7f) * time * 0.27f;
			float rotate_z = k * 37.37f + sin(noise_2 * 37.1f) * time * 0.33f;
			vec4 rotate = quat_rotate_zyx(vec3(rotate_x, rotate_y, rotate_z));
			vec3 translate = vec3(2000.0f + range, sin(noise_0 * 31.7f) * noise * 213.0f, sin(noise * 13.7f) * 217.0f * (noise_0 * noise_2 + (k - 0.5f) * 0.1f));
			
			mat3x4 transform = mat4x3_compose(translate, rotate, scale);
			transform = mat4x3_mul(mat4x3_mul(mat4x3_rotate_z(offset), mat4x3_rotate_z(angle)), transform);
			
			uint node_index = node_indices_buffer[global_id];
			set_node_global_transform(node_address, node_index, transform);
		}
	}
	
#endif
