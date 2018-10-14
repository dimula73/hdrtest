#ifndef USE_OPENGLES
#define INATTR in
#define OUTATTR out
#define highp
#else
#define INATTR attribute
#define OUTATTR varying
#endif
uniform mat4 viewProjectionMatrix;
uniform mat4 textureMatrix;
INATTR highp vec3 vertexPosition;
INATTR highp vec2 texturePosition;
OUTATTR highp vec4 textureCoordinates;
void main()
{
   textureCoordinates = textureMatrix * vec4(texturePosition.x, texturePosition.y, 0, 1);
   gl_Position = viewProjectionMatrix * vec4(vertexPosition.x, vertexPosition.y, 0, 1);
}
