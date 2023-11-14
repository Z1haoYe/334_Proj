#version 330

const int NORMALMODE_FACE = 0;			// Flat normals
const int NORMALMODE_SMOOTH = 1;		// Smooth normals

const int LIGHTTYPE_POINT = 0;
const int LIGHTTYPE_DIRECTIONAL = 1;

const int OBJTYPE_FLOOR = 0;
const int OBJTYPE_CUBE = 1;

layout(location = 0) in vec3 pos;			// Model-space position
layout(location = 1) in vec3 norm;		    // Model-space face normal
layout(location = 2) in vec2 uv;	        // Texture coordinates
layout(location = 3) in vec3 tangent;	    // Right vector in tangent space
layout(location = 4) in vec3 bitangent;	    // Forward vector in tangent space

smooth out vec3 fragPos;	    // Interpolated position in world-space
smooth out vec3 fragNorm;	    // Interpolated normal in world-space
smooth out vec3 fragColor;	    // Interpolated color (for Gouraud shading)
smooth out vec2 fragUV;         // Interpolated texture coordinates
smooth out vec3 tanLightPos;    // Light position in tangent space
smooth out vec3 tanViewer;      // Viewing vector in tangent space
smooth out vec3 tanFragPos;     // Fragment position in tangent space
smooth out vec4 lightFragPos;   // Fragment position in light space

// Light information
struct LightData {
	bool enabled;
	int type;
	vec3 pos;
	vec3 color;
};
	
// Array of lights
const int MAX_LIGHTS = 1;
layout (std140) uniform LightBlock {
	LightData lights [MAX_LIGHTS];
};

uniform mat4 modelMat;		 // Model-to-world transform matrix
uniform mat4 lightSpaceMat;  // World-to-light matrix location
uniform int objType;         // 0 for floor and 1 for cube
uniform mat4 viewProjMat;	 // World-to-clip transform matrix
uniform int shadingMode;     // Phong vs. colored normals
uniform vec3 camPos;         // Camera position

uniform vec3 floorColor;
uniform float floorAmbStr;
uniform float floorDiffStr;
uniform float floorSpecStr;
uniform float floorSpecExp;
uniform vec3 cubeColor;
uniform float cubeAmbStr;
uniform float cubeDiffStr;
uniform float cubeSpecStr;
uniform float cubeSpecExp;

void main() {
	// Get world-space position and normal
	fragPos = vec3(modelMat * vec4(pos, 1.0));
	fragNorm = vec3(modelMat * vec4(norm, 0.0));

	// Get light-space position, pass to fragment shader
	lightFragPos = lightSpaceMat * vec4(fragPos, 1.0);

	// TODO 2-1 TBN system construction
	mat3 newmat = mat3(transpose(inverse(modelMat)));
	vec3 t = normalize(vec3(newmat * normalize(tangent)));
	vec3 n = normalize(vec3(newmat * normalize(norm)));
	t = normalize(t - dot(t, n) * n);
	vec3 b = normalize(cross(t, n));
	mat3 tbn = transpose(mat3(t, b, n));

	// TODO 2-2 Convert lighting related parameters based on TBN system
	//          Hint: "lights[0].pos", "camPos", and "fragPos" to tangent space: light position ("tanLightPos"), camera position ("tanViewer") and fragment position ("tanFragPos") to tangent space

	tanLightPos = tbn * lights[0].pos;
	tanViewer = tbn * camPos.xyz;
	tanFragPos = tbn * fragPos;

	// Pass the interpolated texture coordinates to the fragment shader
	fragUV = uv;

	// Output clip-space position
	gl_Position = viewProjMat * vec4(fragPos, 1.0);
}
