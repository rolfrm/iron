#version 300 es

precision highp float;
precision highp sampler3D;

uniform sampler3D voxel_depth;
uniform sampler3D color;
uniform vec3 camera_position;
uniform vec3 voxel_scale;
uniform vec3 voxel_offset;

in vec3 model_space;
out vec4 fragColor;

float aabb_d(vec3 pos, vec3 aabb_center, vec3 aabb_size) {
    vec3 p = pos - aabb_center;
    vec3 q = abs(p) - aabb_size;
    return length(max(q, vec3(0.0))) + min(max(max(q.x, q.y), q.z), 0.0);
}

void main() 
{
    vec3 mpos = (model_space - voxel_offset) / voxel_scale * 2.0;
	 
	 vec3 d = normalize(mpos - (camera_position - voxel_offset) / voxel_scale);
    float depth = texture(voxel_depth, mpos.xyz).r;
	 for (int i = 0; i < 32; i++) {
        mpos = depth * d * 0.5 + mpos;
        float d0 = aabb_d(mpos, vec3(0.5), vec3(0.5));

        depth = texture(voxel_depth, mpos.xyz).r;
        
		  if(depth > 0.2)
		    discard;
		  if (d0 > 0.0)
            depth += d0;
				
    }
	 vec4 color = texture(color, mpos.xyz);
        
    if(depth < 0.001){
    	 fragColor = color + vec4(0.2,0.0,0.0,1);
	 }else{
		discard;
	 	 //fragColor = vec4(1,0.5,0.5,1);
	 }
}
