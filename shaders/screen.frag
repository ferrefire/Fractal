#version 460

#extension GL_ARB_shading_language_include : require

layout(set = 0, binding = 0) uniform Variables
{
	dvec2 center;
	double zoom;
	float aspect;
	int maxIterations;
} variables;

layout(location = 0) in vec2 localUV;

layout(location = 0) out vec4 pixelColor;

//const int maxIterations = 30;

vec3 ToNonLinear(vec3 linearColor)
{
	vec3 cutoff = step(vec3(0.0031308), linearColor);
	vec3 lower = linearColor * 12.92;
	vec3 higher = 1.055 * pow(linearColor, vec3(1.0 / 2.4)) - 0.055;

	return (mix(lower, higher, cutoff));
}

vec3 Manderbrot(dvec2 uv)
{
	double x = (uv.x - 0.5) * 2.0 * variables.aspect / variables.zoom + variables.center.x;
	double y = (uv.y - 0.5) * 2.0 / variables.zoom + variables.center.y;
	dvec2 c = dvec2(x, y);
	dvec2 z = dvec2(0.0);

	int iterations;
	for (iterations = 0; iterations < variables.maxIterations; iterations++)
	{
		double tmpX = (z.x * z.x - z.y * z.y) + c.x;
		z.y = 2.0 * z.x * z.y + c.y;
		z.x = tmpX;

		if (dot(z, z) > 4.0)
		{
			break;
		}
	}

	float smoothIteration = float(iterations) - log2(log2(float(dot(z, z)))) + 4.0;
	float colorVal = smoothIteration / float(variables.maxIterations);
	vec3 color = vec3(0.5 + 0.5 * cos(6.2831 * colorVal + vec3(0.0, 0.33, 0.67)));  

	return (ToNonLinear(color));
	//return vec3(float(iterations) / float(maxIterations));
}

void main()
{
	//dvec2 uv = gl_FragCoord.xy / resolution.xy;
	dvec2 uv = localUV;
	vec3 result = Manderbrot(uv);
	pixelColor = vec4(result, 1.0);
}