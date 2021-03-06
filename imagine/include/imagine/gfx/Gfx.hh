#pragma once

/*  This file is part of Imagine.

	Imagine is free software: you can redistribute it and/or modify
	it under the terms of the GNU General Public License as published by
	the Free Software Foundation, either version 3 of the License, or
	(at your option) any later version.

	Imagine is distributed in the hope that it will be useful,
	but WITHOUT ANY WARRANTY; without even the implied warranty of
	MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.  See the
	GNU General Public License for more details.

	You should have received a copy of the GNU General Public License
	along with Imagine.  If not, see <http://www.gnu.org/licenses/> */

#include <imagine/engine-globals.h>
#include <imagine/util/bits.h>
#include <imagine/util/rectangle2.h>
#include <imagine/util/time/sys.hh>
#include <imagine/gfx/defs.hh>
#include <imagine/gfx/Viewport.hh>
#include <imagine/gfx/Mat4.hh>
#include <imagine/util/pixel.h>
#include <imagine/gfx/Vec3.hh>
#include <imagine/base/Base.hh>

namespace Gfx
{

// init & control
[[gnu::cold]] CallResult init();
void setViewport(const Base::Window &win, const Viewport &v);
void setProjectionMatrix(const Mat4 &mat);
void setProjectionMatrixRotation(Angle angle);
void animateProjectionMatrixRotation(Angle srcAngle, Angle destAngle);
const Mat4 &projectionMatrix();
const Viewport &viewport();

// commit/sync
void renderFrame(Base::Window &win, Base::FrameTimeBase frameTime);
void updateFrameTime();
extern uint frameTime, frameTimeRel;

enum { TRIANGLE = 1, TRIANGLE_STRIP, QUAD, };

extern bool preferBGRA, preferBGR;

// render states

class Program : public ProgramImpl
{
public:
	constexpr Program() {}
	bool init(Shader vShader, Shader fShader, bool hasColor, bool hasTex);
	void deinit();
	bool link();
	int uniformLocation(const char *uniformName);
};

class BufferImage;
Shader makeShader(const char *src, uint type);
Shader makePluginVertexShader(const char *src, uint imgMode);
Shader makePluginFragmentShader(const char *src, uint imgMode, const BufferImage &img);
Shader makeDefaultVShader();
void deleteShader(Shader shader);
void setProgram(Program &program);
void setProgram(Program &program, Mat4 modelMat);
void uniformF(int uniformLocation, float v1, float v2);
void releaseShaderCompiler();
void autoReleaseShaderCompiler();

enum { BLEND_MODE_OFF = 0, BLEND_MODE_ALPHA, BLEND_MODE_INTENSITY };
void setBlendMode(uint mode);

enum { IMG_MODE_MODULATE = 0, IMG_MODE_BLEND, IMG_MODE_REPLACE, IMG_MODE_ADD };

enum { BLEND_EQ_ADD, BLEND_EQ_SUB, BLEND_EQ_RSUB };
void setBlendEquation(uint mode);

void setActiveTexture(TextureHandle tex, uint type = 0);

void setImgBlendColor(ColorComp r, ColorComp g, ColorComp b, ColorComp a);

void setDither(uint on);
uint dither();

void setZTest(bool on);

enum { BOTH_FACES, FRONT_FACES, BACK_FACES };
void setVisibleGeomFace(uint sides);

void setClipRect(bool on);
void setClipRectBounds(const Base::Window &win, int x, int y, int w, int h);
static void setClipRectBounds(const Base::Window &win, IG::WindowRect r)
{
	setClipRectBounds(win, r.x, r.y, r.xSize(), r.ySize());
}

void setZBlend(bool on);
void setZBlendColor(ColorComp r, ColorComp g, ColorComp b);

void clear();
void setClearColor(ColorComp r, ColorComp g, ColorComp b, ColorComp a = 1.);

void setColor(ColorComp r, ColorComp g, ColorComp b, ColorComp a = 1.);

static void setColor(ColorComp i) { setColor(i, i, i, 1.); }

enum GfxColorEnum { COLOR_WHITE, COLOR_BLACK };
static void setColor(GfxColorEnum colConst)
{
	switch(colConst)
	{
		bcase COLOR_WHITE: setColor(1., 1., 1.);
		bcase COLOR_BLACK: setColor(0., 0., 0.);
	}
}

static const PixelFormatDesc &ColorFormat = PixelFormatRGBA8888;
uint color();

// transforms

enum TransformTargetEnum { TARGET_WORLD, TARGET_TEXTURE };
void setTransformTarget(TransformTargetEnum target);

void loadTransform(Mat4 mat);
void loadTranslate(GC x, GC y, GC z);
void loadIdentTransform();

}
