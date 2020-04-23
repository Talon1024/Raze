/*
** gl_shader.cpp
**
** GLSL shader handling
**
**---------------------------------------------------------------------------
** Copyright 2004-2019 Christoph Oelckers
** All rights reserved.
**
** Redistribution and use in source and binary forms, with or without
** modification, are permitted provided that the following conditions
** are met:
**
** 1. Redistributions of source code must retain the above copyright
**    notice, this list of conditions and the following disclaimer.
** 2. Redistributions in binary form must reproduce the above copyright
**    notice, this list of conditions and the following disclaimer in the
**    documentation and/or other materials provided with the distribution.
** 3. The name of the author may not be used to endorse or promote products
**    derived from this software without specific prior written permission.
**
** THIS SOFTWARE IS PROVIDED BY THE AUTHOR ``AS IS'' AND ANY EXPRESS OR
** IMPLIED WARRANTIES, INCLUDING, BUT NOT LIMITED TO, THE IMPLIED WARRANTIES
** OF MERCHANTABILITY AND FITNESS FOR A PARTICULAR PURPOSE ARE DISCLAIMED.
** IN NO EVENT SHALL THE AUTHOR BE LIABLE FOR ANY DIRECT, INDIRECT,
** INCIDENTAL, SPECIAL, EXEMPLARY, OR CONSEQUENTIAL DAMAGES (INCLUDING, BUT
** NOT LIMITED TO, PROCUREMENT OF SUBSTITUTE GOODS OR SERVICES; LOSS OF USE,
** DATA, OR PROFITS; OR BUSINESS INTERRUPTION) HOWEVER CAUSED AND ON ANY
** THEORY OF LIABILITY, WHETHER IN CONTRACT, STRICT LIABILITY, OR TORT
** (INCLUDING NEGLIGENCE OR OTHERWISE) ARISING IN ANY WAY OUT OF THE USE OF
** THIS SOFTWARE, EVEN IF ADVISED OF THE POSSIBILITY OF SUCH DAMAGE.
**---------------------------------------------------------------------------
**
*/
#include <memory>
#include "gl_load.h"
#include "glbackend.h"
#include "gl_shader.h"
#include "zstring.h"

#include "baselayer.h"

//==========================================================================
//
//
//
//==========================================================================

bool FShader::Load(const char * name, const char * vert_prog, const char * frag_prog)
{
	FString error;
	static char buffer[10000];

	hVertProg = glCreateShader(GL_VERTEX_SHADER);
	hFragProg = glCreateShader(GL_FRAGMENT_SHADER);	

	glShaderSource(hVertProg, 1, &vert_prog, nullptr);
	glShaderSource(hFragProg, 1, &frag_prog, nullptr);

	glCompileShader(hVertProg);
	glCompileShader(hFragProg);

	hShader = glCreateProgram();

	glAttachShader(hShader, hVertProg);
	glAttachShader(hShader, hFragProg);

	glBindAttribLocation(hShader, 0, "i_vertPos");
	glBindAttribLocation(hShader, 1, "i_texCoord");
	glBindAttribLocation(hShader, 2, "i_color");

	glLinkProgram(hShader);

	glGetShaderInfoLog(hVertProg, 10000, NULL, buffer);
	if (*buffer) 
	{
		error << "Vertex shader:\n" << buffer << "\n";
	}
	glGetShaderInfoLog(hFragProg, 10000, NULL, buffer);
	if (*buffer) 
	{
		error << "Fragment shader:\n" << buffer << "\n";
	}

	glGetProgramInfoLog(hShader, 10000, NULL, buffer);
	if (*buffer) 
	{
		error << "Linking:\n" << buffer << "\n";
	}
	int linked;
	glGetProgramiv(hShader, GL_LINK_STATUS, &linked);
	if (linked == 0)
	{
		// only print message if there's an error.
		I_Error("Init Shader '%s':\n%s\n", name, error.GetChars());
	}
	return true;
}

//==========================================================================
//
//
//
//==========================================================================

FShader::~FShader()
{
	glDeleteProgram(hShader);
	glDeleteShader(hVertProg);
	glDeleteShader(hFragProg);
}


//==========================================================================
//
//
//
//==========================================================================

bool FShader::Bind()
{
	glUseProgram(hShader);
	return true;
}


//==========================================================================
//
//
//
//==========================================================================

bool PolymostShader::Load(const char * name, const char * vert_prog, const char * frag_prog)
{
	if (!FShader::Load(name, vert_prog, frag_prog)) return false;
	
	Flags.Init(hShader, "u_flags");
	Shade.Init(hShader, "u_shade");
	ShadeDiv.Init(hShader, "u_shadeDiv");
	NumShades.Init(hShader, "u_numShades");
    VisFactor.Init(hShader, "u_visFactor");
    NPOTEmulationFactor.Init(hShader, "u_npotEmulationFactor");
    NPOTEmulationXOffset.Init(hShader, "u_npotEmulationXOffset");
    Brightness.Init(hShader, "u_brightness");
	FogColor.Init(hShader, "u_fogColor");
	AlphaThreshold.Init(hShader, "u_alphaThreshold");
	FullscreenTint.Init(hShader, "u_fullscreenTint");
	TintModulate.Init(hShader, "u_tintModulate");
	TintOverlay.Init(hShader, "u_tintOverlay");
	TintFlags.Init(hShader, "u_tintFlags");

    RotMatrix.Init(hShader, "u_rotMatrix");
	ModelMatrix.Init(hShader, "u_modelMatrix");
	ProjectionMatrix.Init(hShader, "u_projectionMatrix");
	DetailMatrix.Init(hShader, "u_detailMatrix");
	TextureMatrix.Init(hShader, "u_textureMatrix");


    
	glUseProgram(hShader);

	VSMatrix identity(0);
	TextureMatrix.Set(identity.get());
	DetailMatrix.Set(identity.get());

	int SamplerLoc;
    SamplerLoc = glGetUniformLocation(hShader, "s_texture");
	glUniform1i(SamplerLoc, 0);
	SamplerLoc = glGetUniformLocation(hShader, "s_brightmap");
	glUniform1i(SamplerLoc, 1);
	SamplerLoc = glGetUniformLocation(hShader, "s_detail");
	glUniform1i(SamplerLoc, 2);
	SamplerLoc = glGetUniformLocation(hShader, "s_glow");
	glUniform1i(SamplerLoc, 3);
	SamplerLoc = glGetUniformLocation(hShader, "s_palette");
	glUniform1i(SamplerLoc, 4);
	SamplerLoc = glGetUniformLocation(hShader, "s_palswap");
	glUniform1i(SamplerLoc, 5);

	glUseProgram(0);
	return true;
}

//==========================================================================
//
//
//
//==========================================================================

bool SurfaceShader::Load(const char* name, const char* vert_prog, const char* frag_prog)
{
	if (!FShader::Load(name, vert_prog, frag_prog)) return false;

	glUseProgram(hShader);

	int SamplerLoc;
	SamplerLoc = glGetUniformLocation(hShader, "s_texture");
	glUniform1i(SamplerLoc, 0);
	SamplerLoc = glGetUniformLocation(hShader, "s_palette");
	glUniform1i(SamplerLoc, 1);
	
	glUseProgram(0);
	return true;
}
