#version 110

//attribute vec2 vertex;

//void main(void) {
//    gl_Position = vec4(vertex, 0.0, 1.0);
//}


attribute vec3 vertex;
//attribute vec4 qt_MultiTexCoord0;
//uniform mat4 qt_ModelViewProjectionMatrix;
//varying vec4 qt_TexCoord0;

void main(void)
{
    gl_Position = gl_ModelViewProjectionMatrix * vec4(vertex[0], vertex[1], 0.0, 1.0);
//    qt_TexCoord0 = qt_MultiTexCoord0;
}
