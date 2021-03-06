#pragma once

#import <UIKit/UIKit.h>
#import <QuartzCore/QuartzCore.h>
#import <OpenGLES/EAGLDrawable.h>
#import "MainApp.hh"
#include "ICadeHelper.hh"
#include <imagine/input/Input.hh>

namespace Base
{
	extern UIApplication *sharedApp;
	extern MainApp *mainApp;
	extern BOOL displayLinkActive;
	extern EAGLContext *mainContext;
	extern CADisplayLink *displayLink;
	extern bool useMaxColorBits;
	extern bool isIPad;
}

namespace Input
{
	extern UITextField *vkbdField;
	extern Input::InputTextDelegate vKeyboardTextDelegate;
	extern IG::WindowRect textRect;
	extern ICadeHelper iCade;
}
