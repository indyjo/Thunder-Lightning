varying vec3  Normal;
varying vec3  EyeDir;
varying vec3  SunDir;
varying float Fog;

const float GroundFog_min = 0.0;
const float GroundFog_max = 400.0;
const float GroundFog_scale = 1.0/3500.0;

const vec3 SunDirWCS = vec3(-0.91381, 0.40614, 0.0);

uniform vec3  CamPos;

float uniformFog(float l) {
    return max(0.0, (l-gl_Fog.start) * gl_Fog.scale);
}

float groundFog(float y_a, float y_b, float dist) {
    // make y_a >= y_b
    if (y_a < y_b) {
        float dummy = y_a;
        y_a = y_b;
        y_b = dummy;
    }
    
    if (y_b >= GroundFog_max) {
        return 0.0;
    }
    if (y_a <= GroundFog_min) {
        return 0.0;
    }
    
    if (y_a > GroundFog_max) {
        dist *= (GroundFog_max-y_b) / (y_a-y_b);
        y_a = GroundFog_max;
    }
    
    /*
    if (y_b < GroundFog_min) {
        dist *= (GroundFog_min-y_a) / (y_b-y_a);
        //y_b = GroundFog_min;
    }
    */
    
    return dist * GroundFog_scale;
}

void main(void) 
{
    gl_Position    = ftransform();
    Normal         = gl_NormalMatrix * gl_Normal;
    vec4 pos       = gl_ModelViewMatrix * gl_Vertex;
    EyeDir         = pos.xyz / pos.w;
    gl_TexCoord[0] = gl_MultiTexCoord0;
    
    float dirlen = length(EyeDir);
    Fog  = uniformFog(dirlen);
    Fog += groundFog(gl_Vertex.y, CamPos.y, dirlen);
    Fog = min(1.0, Fog);
    
    SunDir = gl_NormalMatrix * SunDirWCS;
}

