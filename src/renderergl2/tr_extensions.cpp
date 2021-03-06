/*
===========================================================================
Copyright (C) 2011 James Canete (use.less01@gmail.com)
Copyright (C) 2015-2019 GrangerHub

This file is part of Tremulous Arena source code.

Tremulous is free software; you can redistribute it
and/or modify it under the terms of the GNU General Public License as
published by the Free Software Foundation; either version 3 of the License,
or (at your option) any later version.

Tremulous is distributed in the hope that it will be
useful, but WITHOUT ANY WARRANTY; without even the implied warranty of
MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.  See the
GNU General Public License for more details.

You should have received a copy of the GNU General Public License
along with Tremulous; if not, see <https://www.gnu.org/licenses/>

===========================================================================
*/
// tr_extensions.c - extensions needed by the renderer not in sdl_glimp.c

#ifdef USE_LOCAL_HEADERS
#include "SDL.h"
#else
#include <SDL.h>
#endif

#include "tr_local.h"
#include "tr_dsa.h"

#define GLE(ret, name, ...) name##proc *qgl##name;
QGL_1_3_PROCS;
QGL_1_5_PROCS;
QGL_2_0_PROCS;
QGL_3_0_PROCS;
QGL_ARB_framebuffer_object_PROCS;
QGL_ARB_vertex_array_object_PROCS;
QGL_EXT_direct_state_access_PROCS;
#undef GLE

void GLimp_InitExtraExtensions()
{
    const char *extension;
    const char *result[3] = {"...ignoring %s\n", "...using %s\n", "...%s not found\n"};

    // Check OpenGL version
    sscanf(glConfig.version_string, "%d.%d", &glRefConfig.openglMajorVersion, &glRefConfig.openglMinorVersion);
    if (glRefConfig.openglMajorVersion < 2) ri.Error(ERR_FATAL, "OpenGL 2.0 required!");
    ri.Printf(PRINT_ALL, "...using OpenGL %s\n", glConfig.version_string);

    bool q_gl_version_at_least_3_0 = (glRefConfig.openglMajorVersion >= 3);
    bool q_gl_version_at_least_3_2 = (glRefConfig.openglMajorVersion > 3 ||
            (glRefConfig.openglMajorVersion == 3 && glRefConfig.openglMinorVersion > 2));

    // Check if we need Intel graphics specific fixes.
    glRefConfig.intelGraphics = qfalse;
    if (strstr((char *)qglGetString(GL_RENDERER), "Intel")) glRefConfig.intelGraphics = qtrue;

        // set DSA fallbacks
#define GLE(ret, name, ...) qgl##name = GLDSA_##name;
    QGL_EXT_direct_state_access_PROCS;
#undef GLE

    // GL function loader, based on https://gist.github.com/rygorous/16796a0c876cf8a5f542caddb55bce8a
#define GLE(ret, name, ...) qgl##name = (name##proc *)SDL_GL_GetProcAddress("gl" #name);

    // OpenGL 1.3, was GL_ARB_texture_compression
    QGL_1_3_PROCS;

    // OpenGL 1.5, was GL_ARB_vertex_buffer_object and GL_ARB_occlusion_query
    QGL_1_5_PROCS;
    glRefConfig.occlusionQuery = qtrue;

    // OpenGL 2.0, was GL_ARB_shading_language_100, GL_ARB_vertex_program, GL_ARB_shader_objects, and
    // GL_ARB_vertex_shader
    QGL_2_0_PROCS;

    // OpenGL 3.0 - no matching extension
    // QGL_*_PROCS becomes several functions, do not remove {}
    if (q_gl_version_at_least_3_0)
    {
        QGL_3_0_PROCS;
    }

    // OpenGL 3.0 - GL_ARB_framebuffer_object
    extension = "GL_ARB_framebuffer_object";
    glRefConfig.framebufferObject = qfalse;
    glRefConfig.framebufferBlit = qfalse;
    glRefConfig.framebufferMultisample = qfalse;
    if (q_gl_version_at_least_3_0 || SDL_GL_ExtensionSupported(extension))
    {
        glRefConfig.framebufferObject = !!r_ext_framebuffer_object->integer;
        glRefConfig.framebufferBlit = qtrue;
        glRefConfig.framebufferMultisample = qtrue;

        qglGetIntegerv(GL_MAX_RENDERBUFFER_SIZE, &glRefConfig.maxRenderbufferSize);
        qglGetIntegerv(GL_MAX_COLOR_ATTACHMENTS, &glRefConfig.maxColorAttachments);

        QGL_ARB_framebuffer_object_PROCS;

        ri.Printf(PRINT_ALL, result[glRefConfig.framebufferObject], extension);
    }
    else
    {
        ri.Printf(PRINT_ALL, result[2], extension);
    }

    // OpenGL 3.0 - GL_ARB_vertex_array_object
    extension = "GL_ARB_vertex_array_object";
    glRefConfig.vertexArrayObject = qfalse;
    if (q_gl_version_at_least_3_0 || SDL_GL_ExtensionSupported(extension))
    {
        if (q_gl_version_at_least_3_0)
        {
            // force VAO, core context requires it
            glRefConfig.vertexArrayObject = qtrue;
        }
        else
        {
            glRefConfig.vertexArrayObject = !!r_arb_vertex_array_object->integer;
        }

        QGL_ARB_vertex_array_object_PROCS;

        ri.Printf(PRINT_ALL, result[glRefConfig.vertexArrayObject], extension);
    }
    else
    {
        ri.Printf(PRINT_ALL, result[2], extension);
    }

    // OpenGL 3.0 - GL_ARB_texture_float
    extension = "GL_ARB_texture_float";
    glRefConfig.textureFloat = qfalse;
    if (q_gl_version_at_least_3_0 || SDL_GL_ExtensionSupported(extension))
    {
        glRefConfig.textureFloat = !!r_ext_texture_float->integer;

        ri.Printf(PRINT_ALL, result[glRefConfig.textureFloat], extension);
    }
    else
    {
        ri.Printf(PRINT_ALL, result[2], extension);
    }

    // OpenGL 3.2 - GL_ARB_depth_clamp
    extension = "GL_ARB_depth_clamp";
    glRefConfig.depthClamp = qfalse;
    if (q_gl_version_at_least_3_2 || SDL_GL_ExtensionSupported(extension))
    {
        glRefConfig.depthClamp = qtrue;

        ri.Printf(PRINT_ALL, result[glRefConfig.depthClamp], extension);
    }
    else
    {
        ri.Printf(PRINT_ALL, result[2], extension);
    }

    // OpenGL 3.2 - GL_ARB_seamless_cube_map
    extension = "GL_ARB_seamless_cube_map";
    glRefConfig.seamlessCubeMap = qfalse;
    if (q_gl_version_at_least_3_2 || SDL_GL_ExtensionSupported(extension))
    {
        glRefConfig.seamlessCubeMap = !!r_arb_seamless_cube_map->integer;

        ri.Printf(PRINT_ALL, result[glRefConfig.seamlessCubeMap], extension);
    }
    else
    {
        ri.Printf(PRINT_ALL, result[2], extension);
    }

    // Determine GLSL version
    if (1)
    {
        char version[256];

        Q_strncpyz(version, (char *)qglGetString(GL_SHADING_LANGUAGE_VERSION), sizeof(version));

        sscanf(version, "%d.%d", &glRefConfig.glslMajorVersion, &glRefConfig.glslMinorVersion);

        ri.Printf(PRINT_ALL, "...using GLSL version %s\n", version);
    }

    glRefConfig.memInfo = MI_NONE;

    // GL_NVX_gpu_memory_info
    extension = "GL_NVX_gpu_memory_info";
    if (SDL_GL_ExtensionSupported(extension))
    {
        glRefConfig.memInfo = MI_NVX;

        ri.Printf(PRINT_ALL, result[1], extension);
    }
    else
    {
        ri.Printf(PRINT_ALL, result[2], extension);
    }

    // GL_ATI_meminfo
    extension = "GL_ATI_meminfo";
    if (SDL_GL_ExtensionSupported(extension))
    {
        if (glRefConfig.memInfo == MI_NONE)
        {
            glRefConfig.memInfo = MI_ATI;

            ri.Printf(PRINT_ALL, result[1], extension);
        }
        else
        {
            ri.Printf(PRINT_ALL, result[0], extension);
        }
    }
    else
    {
        ri.Printf(PRINT_ALL, result[2], extension);
    }

    glRefConfig.textureCompression = TCR_NONE;

    // GL_ARB_texture_compression_rgtc
    extension = "GL_ARB_texture_compression_rgtc";
    if (SDL_GL_ExtensionSupported(extension))
    {
        bool useRgtc = r_ext_compressed_textures->integer >= 1;

        if (useRgtc) glRefConfig.textureCompression |= TCR_RGTC;

        ri.Printf(PRINT_ALL, result[useRgtc], extension);
    }
    else
    {
        ri.Printf(PRINT_ALL, result[2], extension);
    }

    glRefConfig.swizzleNormalmap = r_ext_compressed_textures->integer && !(glRefConfig.textureCompression & TCR_RGTC);

    // GL_ARB_texture_compression_bptc
    extension = "GL_ARB_texture_compression_bptc";
    if (SDL_GL_ExtensionSupported(extension))
    {
        bool useBptc = r_ext_compressed_textures->integer >= 2;

        if (useBptc) glRefConfig.textureCompression |= TCR_BPTC;

        ri.Printf(PRINT_ALL, result[useBptc], extension);
    }
    else
    {
        ri.Printf(PRINT_ALL, result[2], extension);
    }

    // GL_EXT_direct_state_access
    extension = "GL_EXT_direct_state_access";
    glRefConfig.directStateAccess = qfalse;
    if (SDL_GL_ExtensionSupported(extension))
    {
        glRefConfig.directStateAccess = !!r_ext_direct_state_access->integer;

        // QGL_*_PROCS becomes several functions, do not remove {}
        if (glRefConfig.directStateAccess)
        {
            QGL_EXT_direct_state_access_PROCS;
        }

        ri.Printf(PRINT_ALL, result[glRefConfig.directStateAccess], extension);
    }
    else
    {
        ri.Printf(PRINT_ALL, result[2], extension);
    }

#undef GLE
}
