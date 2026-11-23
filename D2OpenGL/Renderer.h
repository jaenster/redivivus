// The D2OpenGL renderer as a D2gfx renderer: the function table D2gfx selects for
// renderer mode 5 (RENDERED_Unused5, the OpenGL slot 1.14d Windows leaves empty).
// Built only with D2_OPENGL.

#pragma once

struct D2RendererFunctionsStrc;

namespace D2OpenGL::Renderer {

extern D2RendererFunctionsStrc OpenGLFunctionTable;

} // namespace D2OpenGL::Renderer
