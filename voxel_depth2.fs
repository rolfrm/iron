#version 300 es

precision highp float;
precision highp sampler3D;

uniform sampler3D voxel_depth;
uniform sampler3D color;
uniform vec3 camera_position;
uniform vec3 voxel_scale;
uniform vec3 voxel_offset;
uniform vec3 voxel_space;

in vec3 model_space;
out vec4 fragColor;


float aabb_d(vec3 pos, vec3 aabb_center, vec3 aabb_size) {
    vec3 p = pos - aabb_center;
    vec3 q = abs(p) - aabb_size;
    return length(max(q, vec3(0.0))) + min(max(max(q.x, q.y), q.z), 0.0);
}


void main() 
{
  
  vec3 ray_pos = voxel_offset + model_space * voxel_scale;
  vec3 d = normalize(ray_pos - camera_position);
  
  vec3 ray_step = sign(d);
  // sign mask: 0-1 version of sign
  vec3 smask = (vec3(ray_step) + 1.0) * 0.5;
  // get the distance for each side.
	 // for example: 0.2 -> 0.8, with positive step
	 //    while 0.8 -> 0.2 with negative step
	 
	 vec3 side_dist =
		(1.0 - smask) * (ray_pos - floor(ray_pos))
		+ (smask) * (ceil(ray_pos) - ray_pos);
	 //bvec3 mask;
	 float last_step = 0.0;
	 
    for (int i = 0; i < 32; i++) {
			
		float depth = texture(voxel_depth, (ray_pos - voxel_offset) / voxel_scale).r;
		if(depth < 0.0001) break;
		//mask = lessThanEqual(side_dist.xyz, min(side_dist.yzx, side_dist.zxy));
		if(d.x < 0.0)
		  side_dist.x = floor(ray_pos.x) - ray_pos.x;
		if(d.x > 0.0)
		  side_dist.x = ceil(ray_pos.x) - ray_pos.x;
		if(d.y < 0.0)
		  side_dist.y = floor(ray_pos.y) - ray_pos.y;
		if(d.y > 0.0)
		  side_dist.y = ceil(ray_pos.y) - ray_pos.y;
		if(d.z < 0.0)
		  side_dist.z = floor(ray_pos.z) - ray_pos.z ;
		if(d.z > 0.0)
		  side_dist.z = ceil(ray_pos.z) - ray_pos.z;

		
		side_dist = side_dist / d;
		
		float step = min(min(side_dist.x, side_dist.z), side_dist.y);
		float step2 = 0.0;
		
		//step = 0.2;//step * 0.5;	 
		step = max(step, 0.01);
		last_step = step;
		//step = max(0.01, step);
		//step = 0.5;
		ray_pos =  d * step + ray_pos;
		
		if(ray_pos.x < 0.0 || ray_pos.y < 0.0 || ray_pos.z < 0.0) discard;
		if(ray_pos.x > voxel_scale.x || ray_pos.y > voxel_scale.y || ray_pos.z > voxel_scale.z) discard;
	 }
	
	 vec4 colorOut = texture(color, (ray_pos - voxel_offset) / voxel_scale);
	 //colorOut =vec4(last_step,-last_step,0.5, 1);
	 //if(side_dist.x < 0.001 || side_dist.y < 0.001 || side_dist.z < 0.001){
	 //	 fragColor= vec4(1,0,0,1);
	 //}else{
		fragColor = colorOut;
		//}
	 
}
