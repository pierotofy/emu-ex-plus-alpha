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

#define LOGTAG "ResFontAndroid"
#include <imagine/resource/font/ResourceFontAndroid.hh>
#include <imagine/gfx/Gfx.hh>
#include <imagine/util/strings.h>
#include <imagine/util/jni.hh>
#include "../../base/android/private.hh"
#include <android/bitmap.h>

using namespace Base;

static JavaInstMethod<jobject> jCharBitmap, jNewSize;
static JavaInstMethod<void> jApplySize, jFreeSize, jUnlockCharBitmap;
static JavaInstMethod<jboolean> jActiveChar;
static JavaInstMethod<jint> jCurrentCharXSize, jCurrentCharYSize, jCurrentCharXOffset, jCurrentCharYOffset,
	jCurrentFaceDescender, jCurrentFaceAscender, jCurrentCharXAdvance;

static void setupResourceFontAndroidJni(JNIEnv *jEnv, jobject renderer)//jClsLoader, const JavaInstMethod<jobject> &jLoadClass)
{
	if(jCharBitmap.m)
		return; // already setup
	//logMsg("setting up JNI methods");
	/*jstring classStr = jEnv->NewStringUTF("com/imagine/FontRenderer");
	jFontRendererCls = (jclass)jEnv->NewGlobalRef(jLoadClass(jEnv, jClsLoader, classStr));
	jEnv->DeleteLocalRef(classStr);
	jFontRenderer.setup(jEnv, jFontRendererCls, "<init>", "()V");*/
	auto jFontRendererCls = jEnv->GetObjectClass(renderer);
	jCharBitmap.setup(jEnv, jFontRendererCls, "charBitmap", "()Landroid/graphics/Bitmap;");
	jUnlockCharBitmap.setup(jEnv, jFontRendererCls, "unlockCharBitmap", "(Landroid/graphics/Bitmap;)V");
	jActiveChar.setup(jEnv, jFontRendererCls, "activeChar", "(I)Z");
	jCurrentCharXSize.setup(jEnv, jFontRendererCls, "currentCharXSize", "()I");
	jCurrentCharYSize.setup(jEnv, jFontRendererCls, "currentCharYSize", "()I");
	jCurrentCharXOffset.setup(jEnv, jFontRendererCls, "currentCharXOffset", "()I");
	jCurrentCharYOffset.setup(jEnv, jFontRendererCls, "currentCharYOffset", "()I");
	jCurrentCharXAdvance.setup(jEnv, jFontRendererCls, "currentCharXAdvance", "()I");
	//jCurrentFaceDescender.setup(jEnv, jFontRendererCls, "currentFaceDescender", "()I");
	//jCurrentFaceAscender.setup(jEnv, jFontRendererCls, "currentFaceAscender", "()I");
	jNewSize.setup(jEnv, jFontRendererCls, "newSize", "(I)Landroid/graphics/Paint;");
	jApplySize.setup(jEnv, jFontRendererCls, "applySize", "(Landroid/graphics/Paint;)V");
	jFreeSize.setup(jEnv, jFontRendererCls, "freeSize", "(Landroid/graphics/Paint;)V");
}

ResourceFont *ResourceFontAndroid::loadSystem()
{
	ResourceFontAndroid *inst = new ResourceFontAndroid;
	if(!inst)
	{
		logErr("out of memory");
		return nullptr;
	}
	auto jEnv = eEnv();
	inst->renderer = Base::newFontRenderer(jEnv);
	setupResourceFontAndroidJni(jEnv, inst->renderer);
	jthrowable exc = jEnv->ExceptionOccurred();
	if(exc)
	{
		logErr("exception");
		jEnv->ExceptionClear();
		inst->free();
		return nullptr;
	}
	inst->renderer = Base::jniThreadNewGlobalRef(jEnv, inst->renderer);

	return inst;
}

void ResourceFontAndroid::free()
{
	if(renderer)
		Base::jniThreadDeleteGlobalRef(eEnv(), renderer);
	delete this;
}

static const char *androidBitmapResultToStr(int result)
{
	switch(result)
	{
		case ANDROID_BITMAP_RESULT_SUCCESS: return "Success";
		case ANDROID_BITMAP_RESULT_BAD_PARAMETER: return "Bad Parameter";
		case ANDROID_BITMAP_RESULT_JNI_EXCEPTION: return "JNI Exception";
		case ANDROID_BITMAP_RESULT_ALLOCATION_FAILED: return "Allocation Failed";
		default: return "Unknown";
	}
}

IG::Pixmap ResourceFontAndroid::charBitmap()
{
	assert(!lockedBitmap);
	auto jEnv = eEnv();
	lockedBitmap = jCharBitmap(jEnv, renderer);
	assert(lockedBitmap);
	//logMsg("got bitmap @ %p", lockedBitmap);
	//lockedBitmap = Base::jniThreadNewGlobalRef(jEnv, lockedBitmap);
	AndroidBitmapInfo info;
	{
		auto res = AndroidBitmap_getInfo(jEnv, lockedBitmap, &info);
		//logMsg("AndroidBitmap_getInfo returned %s", androidBitmapResultToStr(res));
		assert(res == ANDROID_BITMAP_RESULT_SUCCESS);
		//logMsg("size %dx%d, pitch %d", info.width, info.height, info.stride);
	}
	IG::Pixmap pix{PixelFormatA8};
	pix.init2(nullptr, info.width, info.height, info.stride);
	{
		auto res = AndroidBitmap_lockPixels(jEnv, lockedBitmap, (void**)&pix.data);
		//logMsg("AndroidBitmap_lockPixels returned %s", androidBitmapResultToStr(res));
		assert(res == ANDROID_BITMAP_RESULT_SUCCESS);
	}
	return pix;
}

void ResourceFontAndroid::unlockCharBitmap(IG::Pixmap &pix)
{
	auto jEnv = eEnv();
	AndroidBitmap_unlockPixels(jEnv, lockedBitmap);
	jUnlockCharBitmap(jEnv, renderer, lockedBitmap);
	jEnv->DeleteLocalRef(lockedBitmap);
	//Base::jniThreadDeleteGlobalRef(jEnv, lockedBitmap);
	lockedBitmap = nullptr;
}

CallResult ResourceFontAndroid::activeChar(int idx, GlyphMetrics &metrics)
{
	//logMsg("active char: %c", idx);
	auto jEnv = eEnv();
	if(jActiveChar(jEnv, renderer, idx))
	{
		metrics.xSize = jCurrentCharXSize(jEnv, renderer);
		metrics.ySize = jCurrentCharYSize(jEnv, renderer);
		metrics.xOffset = jCurrentCharXOffset(jEnv, renderer);
		metrics.yOffset = jCurrentCharYOffset(jEnv, renderer);
		metrics.xAdvance = jCurrentCharXAdvance(jEnv, renderer);
		//logMsg("char metrics: size %dx%d offset %dx%d advance %d", metrics.xSize, metrics.ySize,
		//		metrics.xOffset, metrics.yOffset, metrics.xAdvance);
		return OK;
	}
	else
	{
		logMsg("char not available");
		return INVALID_PARAMETER;
	}
}

/*int ResourceFontAndroid::currentFaceDescender () const
{ return jCurrentFaceDescender(eEnv(), renderer); }
int ResourceFontAndroid::currentFaceAscender () const
{ return jCurrentFaceAscender(eEnv(), renderer); }*/

CallResult ResourceFontAndroid::newSize(const FontSettings &settings, FontSizeRef &sizeRef)
{
	freeSize(sizeRef);
	auto jEnv = eEnv();
	auto size = jNewSize(jEnv, renderer, settings.pixelHeight);
	assert(size);
	logMsg("allocated new size %dpx @ 0x%p", settings.pixelHeight, size);
	sizeRef.ptr = Base::jniThreadNewGlobalRef(jEnv, size);
	return OK;
}

CallResult ResourceFontAndroid::applySize(FontSizeRef &sizeRef)
{
	jApplySize(eEnv(), renderer, sizeRef.ptr);
	return OK;
}

void ResourceFontAndroid::freeSize(FontSizeRef &sizeRef)
{
	if(!sizeRef.ptr)
		return;
	auto jEnv = eEnv();
	jFreeSize(jEnv, renderer, sizeRef.ptr);
	Base::jniThreadDeleteGlobalRef(jEnv, (jobject)sizeRef.ptr);
	sizeRef.ptr = nullptr;
}
