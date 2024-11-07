#version 400 core

layout( location = 0 ) in vec3 vPosition;
layout( location = 1 ) in vec4 vColour;
layout( location = 2 ) in vec3 vNormal;
layout (location = 3) in vec2 aTexCoord;

uniform vec3 lightPos;

uniform vec4 ambient;
uniform vec3 dLight;


uniform vec3 sLight;
uniform float sShine;

uniform mat4 mv_matrix;
uniform mat4 p_matrix;

out vec4 fragColour;
out vec2 TexCoord;


void
main()
{
	// view-space coordinate
    vec4 P = mv_matrix * vec4(vPosition,1.0);
	// calculate normal in view-space
	vec3 N = mat3(mv_matrix) * vNormal;
	// calcuate light vector in view space
	vec3 L = lightPos - P.xyz;
	// calc the view vector
	vec3 V = -P.xyz;
	
	//Normalise
	N = normalize(N);
	L = normalize(L);
	V = normalize(V);

	// calc the reflection on the plane for the normal
	vec3 R = reflect(-L,N);

	// added specular and diffuse contributions to the colour 
	vec3 diffuse = max(0.0, dot(N,L)) * dLight;
	vec3 specular = pow(max(dot(R,V), 0.0), sShine) * sLight;

	fragColour =  (ambient + vec4(diffuse,1.0) + vec4(specular,1.0) ) * vColour;

	gl_Position = p_matrix * P;
	TexCoord = aTexCoord;
}