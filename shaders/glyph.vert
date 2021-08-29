#version 460 core

uniform samplerBuffer uCurvesSampler;
uniform sampler2DRect uAtlasSampler;
uniform vec2 uPositionMul;
uniform vec2 uPositionAdd;
uniform vec2 uCanvasSize;

layout(location = 0) in vec4 aColor;
layout(location = 1) in vec2 aPosition;
layout(location = 2) in vec2 aNormCoord;
layout(location = 3) in int aCurvesMin;

out vec4 vColor;
out int vCurvesMin;  // (x).rg has the start of the grid cell;
                     // (x).ba has the size of the grid cell;
                     // (x+i).ba, (x+i+1).rg, (x+i+1).ba describe bezier curve i
out ivec2 vGridMin;
out ivec2 vGridSize;
out vec2 vNormCoord; // normalized glyph coordinate with border offset (e.g. -0.1 to 1.1)

#define kGlyphExpandFactor 0.1

void main()
{
    vColor = aColor;

    vCurvesMin = aCurvesMin;

    vec4 texGrid = texelFetch(uCurvesSampler, aCurvesMin);
    vGridMin = ivec2(texGrid.rg);
    vGridSize = ivec2(texGrid.ba);

    // Adjust vNormCoord to compensate for expanded glyph bounding boxes
    vNormCoord = aNormCoord * (1.0 + 2.0 * kGlyphExpandFactor) - kGlyphExpandFactor;

    // Transform position
    vec2 pos = aPosition;
    pos.y = 1.0 - pos.y;
    pos = pos * uPositionMul + uPositionAdd;

    gl_Position = vec4(pos, 0.0, 1.0);
    gl_Position.x *= uCanvasSize.y / uCanvasSize.x;

}
