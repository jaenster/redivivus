// The D2OpenGL renderer as a D2gfx renderer: the function table D2gfx selects for
// renderer mode 5 (RENDERED_Unused5, the OpenGL slot 1.14d Windows leaves empty).
// Restoration code, not in any Mac or Windows binary. Not referenced by the game yet: D2gfx
// keeps 1.14d's empty mode-5 slot until the renderer is linked in.

#pragma once

struct D2RendererFunctionsStrc;

namespace D2OpenGL::Renderer {

extern D2RendererFunctionsStrc OpenGLFunctionTable;

} // namespace D2OpenGL::Renderer
