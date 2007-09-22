uniform sampler2D MirrorMap;
uniform sampler2D BumpMap;
uniform float aspect;
uniform float focus;
uniform float Time;
uniform mat3 CamToMir;

varying vec3  Normal;
varying vec3  EyeDir;
varying float Fog;
varying vec3 SunDir;


const vec3  BaseColor = vec3(31.0, 38., 52.0)/255.0;
const float MixRatio = 0.02;
const float waves=.5;

const int octaves = 2;
const vec4 octave_scales = vec4(1.0, 4.0, 64.0, 512.0);
const vec4 octave_factors = vec4(1.0, 1.0/2.0, 1.0/64.0, 1.0/8.0);
const vec4 octave_dxdt = 1.0/240.0*vec4(8.0, -4.0, -0.5,  0.5);
const vec4 octave_dydt = 1.0/240.0*vec4(0.0,  2.0,  1.0, -0.0625);

const float FresnelExponent = 0.4;
const float MinReflectivity = 0.062;
const float MaxReflectivity = 0.6;

const float PhongExponent = 40.0;
const float PhongFactor = 0.8;
const vec3 SunColor = vec3(0.97,0.9,0.8);

void main (void)
{
    if(Fog >= 1.0) {
        gl_FragColor = gl_Fog.color;
        return;
    }
    vec3 EyeDirNorm = normalize(EyeDir);
    
    vec3 bumpColor = vec3(0.0,0.0,0.0);
    for(int i=0; i<octaves; ++i) {
        vec2 texcoord = octave_scales[i]*gl_TexCoord[0].st;
        texcoord += Time * vec2(octave_dxdt[i], octave_dydt[i]);
        vec4 color = texture2D(BumpMap, texcoord)-vec4(.5,.5,.5,0.0);
        bumpColor += vec3(color) * octave_factors[i];
    }
    
    bumpColor *= -dot(EyeDirNorm, Normal);
        
    vec3 pxNormal = normalize(Normal - waves*vec3(bumpColor.s, 0, bumpColor.t));
    
    vec3 reflectDir = reflect(EyeDirNorm, pxNormal);
    
    // Careful: reflectDir is in OpenGL's coordinate system, where -Z is the
    // direction the camera looks at. CamToMir on the other hand is specified
    // in the game's CS where the camera is pointed toward +Z. We convert
    // reflectDir into the game's CS but we do not convert the result (mirrorDir)
    // back into OpenGL's.
    vec3 mirrorDir = CamToMir * vec3(reflectDir.x, reflectDir.y, -reflectDir.z);
    // mirrorDir has a positive Z value.
    
    vec2 mirrorTexCoords = 0.5*focus * mirrorDir.xy / mirrorDir.z;
    mirrorTexCoords.x /=  aspect;
    mirrorTexCoords = vec2(0.5,0.5) + mirrorTexCoords;

    vec3 reflectionColor = vec3(texture2D(MirrorMap, mirrorTexCoords));

    float fresnel = max(0.0,-dot(EyeDirNorm, pxNormal));
    fresnel = max(0.0, min(1.0, 1.0-pow(fresnel,FresnelExponent)));
    float reflection = mix(MinReflectivity, MaxReflectivity, fresnel);
    
    vec3 envColor = BaseColor + reflection*reflectionColor;
    envColor += SunColor * PhongFactor * pow(max(0.0,dot(reflectDir,SunDir)), PhongExponent);
    envColor = mix(envColor, gl_Fog.color.rgb, Fog);
    gl_FragColor = vec4 (envColor, 1.0);
}

