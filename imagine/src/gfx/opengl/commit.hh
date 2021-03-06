#pragma once

namespace Gfx
{

void waitVideoSync()
{
	logDMsg("called wait video sync");
	/*#ifdef CONFIG_BASE_X11
	if(useSGIVidSync)
	{
		uint oldFrameTime = gfx_frameTime;
		//glXGetVideoSyncSGI(&gfx_frameTime);
		glXWaitVideoSyncSGI(2, (gfx_frameTime+1)%2, &gfx_frameTime);
		//if(gfx_frameTime - oldFrameTime > 1)
		//	logDMsg("%d since last sync, after wait", gfx_frameTime - oldFrameTime);
	}
	#endif*/
}

/*void gfx_initFrameClockTime()
{
	/*#ifdef CONFIG_BASE_X11
	if(useSGIVidSync)
	{
		glXGetVideoSyncSGI(&gfx_frameTime);
		gfx_frameTimeRel = 1;
	}
	else
	#endif*/
	/*{
		gfx_frameClockTime.setTimeNow();
		//gfx_frameTimeRel = 1;
	}
}*/

void updateFrameTime()
{
//	#ifdef CONFIG_GFX_OPENGL_GLX
//	if(useSGIVidSync)
//	{
//		uint prevFrameTime = gfx_frameTime;
//		glXGetVideoSyncSGI(&gfx_frameTime);
//		gfx_frameTimeRel = gfx_frameTime - prevFrameTime;
//	}
//	#endif
	//logMsg("current frame %d, diff %d", gfx_frameTime, gfx_frameTimeRel);
}

void clear()
{
	// always clear screen to trigger a discarded buffer optimization
	// TODO: test other systems to determine the what's best
	glClear(GL_COLOR_BUFFER_BIT | GL_DEPTH_BUFFER_BIT | GL_STENCIL_BUFFER_BIT);
}

void renderFrame(Base::Window &win, Base::FrameTimeBase frameTime)
{
	Base::onDraw(win, frameTime);

	//glFlush();
	//glFinish();
	#ifdef __ANDROID__
	if(unlikely(glSyncHackEnabled)) glFinish();
	#endif

	/*#if defined(CONFIG_GFX_OPENGL_ES) && defined(CONFIG_BASE_IOS)
	if(useDiscardFramebufferEXT)
	{
		static const GLenum discardAttachments[] = { GL_COLOR_EXT, GL_DEPTH_EXT, GL_STENCIL_EXT };
		glDiscardFramebufferEXT(GL_FRAMEBUFFER_OES, 3, discardAttachments);
	}
	#endif*/

	/*#ifdef CONFIG_BASE_X11
	if(useSGIVidSync)
	{
		uint oldFrameTime = gfx_frameTime;
		glXGetVideoSyncSGI(&gfx_frameTime);
		logDMsg("gfx update, old %d now %d", );
		if(gfx_frameTime - oldFrameTime > 1)
			logDMsg("%d since last sync, after render", gfx_frameTime - oldFrameTime);
	}
	#endif*/
}

}
