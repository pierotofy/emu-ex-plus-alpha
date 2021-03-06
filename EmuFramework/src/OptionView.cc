/*  This file is part of EmuFramework.

	Imagine is free software: you can redistribute it and/or modify
	it under the terms of the GNU General Public License as published by
	the Free Software Foundation, either version 3 of the License, or
	(at your option) any later version.

	Imagine is distributed in the hope that it will be useful,
	but WITHOUT ANY WARRANTY; without even the implied warranty of
	MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.  See the
	GNU General Public License for more details.

	You should have received a copy of the GNU General Public License
	along with EmuFramework.  If not, see <http://www.gnu.org/licenses/> */

#include <OptionView.hh>
#include <EmuApp.hh>
#include <FilePicker.hh>
#include <algorithm>

void BiosSelectMenu::onSelectFile(const char* name, const Input::Event &e)
{
	logMsg("size %d", (int)sizeof(*biosPathStr));
	snprintf(*biosPathStr, sizeof(*biosPathStr), "%s/%s", FsSys::workDir(), name);
	if(onBiosChangeD) onBiosChangeD();
	workDirStack.pop();
	viewStack.popAndShow();
}

void BiosSelectMenu::init(FsSys::cPath *biosPathStr, int (*fsFilter)(const char *name, int type), bool highlightFirst)
{
	var_selfs(biosPathStr);
	var_selfs(fsFilter);
	init(highlightFirst);
}

void BiosSelectMenu::init(bool highlightFirst)
{
	assert(biosPathStr);
	choiceEntry[0].init("Select File"); choiceEntryItem[0] = &choiceEntry[0];
	choiceEntry[0].onSelect() =
		[this](TextMenuItem &, const Input::Event &e)
		{
			workDirStack.push();
			chdirFromFilePath(*biosPathStr);
			auto &fPicker = *allocModalView<EmuFilePicker>(window());
			fPicker.init(!e.isPointer(), false, fsFilter);
			fPicker.onSelectFile() =
				[this](FSPicker &picker, const char* name, const Input::Event &e)
				{
					onSelectFile(name, e);
					picker.dismiss();
				};
			fPicker.onClose() =
				[](FSPicker &picker, const Input::Event &e)
				{
					picker.dismiss();
					workDirStack.pop();
				};
			modalViewController.pushAndShow(fPicker);
		};
	choiceEntry[1].init("Unset"); choiceEntryItem[1] = &choiceEntry[1];
	choiceEntry[1].onSelect() =
		[this](TextMenuItem &, const Input::Event &e)
		{
			viewStack.popAndShow();
			strcpy(*biosPathStr, "");
			if(onBiosChangeD) onBiosChangeD();
		};
	BaseMenuView::init(choiceEntryItem, sizeofArray(choiceEntry), highlightFirst);
}

void OptionView::autoSaveStateInit()
{
	static const char *str[] =
	{
		"Off", "On Exit",
		"15mins", "30mins"
	};
	int val = 0;
	switch(optionAutoSaveState.val)
	{
		bcase 1: val = 1;
		bcase 15: val = 2;
		bcase 30: val = 3;
	}
	autoSaveState.init(str, val, sizeofArray(str));
}

void OptionView::fastForwardSpeedinit()
{
	static const char *str[] =
	{
		"3x", "4x", "5x",
		"6x", "7x", "8x"
	};
	int val = 0;
	if(optionFastForwardSpeed >= MIN_FAST_FORWARD_SPEED && optionFastForwardSpeed <= 7)
	{
		val = optionFastForwardSpeed - MIN_FAST_FORWARD_SPEED;
	}
	fastForwardSpeed.init(str, val, sizeofArray(str));
}


static void uiVisibiltyInit(const Byte1Option &option, MultiChoiceSelectMenuItem &menuItem)
{
	static const char *str[] =
	{
		"Off", "In Game", "On",
	};
	int val = 2;
	if(option < 2)
		val = option;
	menuItem.init(str, val, sizeofArray(str));
}

void OptionView::frameSkipInit()
{
	static const char *str[] =
	{
		"Auto", "0",
		#if defined(CONFIG_BASE_IOS) || defined(CONFIG_BASE_X11)
		"1", "2", "3", "4"
		#endif
	};
	int baseVal = -1;
	int val = int(optionFrameSkip);
	if(optionFrameSkip.val == EmuSystem::optionFrameSkipAuto)
		val = -1;
	frameSkip.init(str, val, sizeofArray(str), baseVal);
}

void OptionView::audioRateInit()
{
	static const char *str[] =
	{
		"22KHz", "32KHz", "44KHz", "48KHz"
	};
	int rates = 3;
	if(Audio::supportsRateNative(48000))
	{
		logMsg("supports 48KHz");
		rates++;
	}

	int val = 2; // default to 44KHz
	switch(optionSoundRate)
	{
		bcase 22050: val = 0;
		bcase 32000: val = 1;
		bcase 48000: val = 3;
	}

	audioRate.init(str, val, rates);
}

#ifdef CONFIG_BLUETOOTH_SCAN_SECS
void OptionView::btScanSecsInit()
{
	static const char *str[] =
	{
		"2secs", "4secs", "6secs", "8secs", "10secs"
	};

	int val = 1;
	switch(BluetoothAdapter::scanSecs)
	{
		bcase 2: val = 0;
		bcase 4: val = 1;
		bcase 6: val = 2;
		bcase 8: val = 3;
		bcase 10: val = 4;
	}
	btScanSecs.init(str, val, sizeofArray(str));
}
#endif

enum { O_AUTO = -1, O_90, O_270, O_0, O_180 };

int convertOrientationMenuValueToOption(int val)
{
	if(val == O_AUTO)
		return Base::VIEW_ROTATE_AUTO;
	else if(val == O_90)
		return Base::VIEW_ROTATE_90;
	else if(val == O_270)
		return Base::VIEW_ROTATE_270;
	else if(val == O_0)
		return Base::VIEW_ROTATE_0;
	else if(val == O_180)
		return Base::VIEW_ROTATE_180;
	bug_exit("invalid value %d", val);
	return 0;
}

static void orientationInit(MultiChoiceSelectMenuItem &item, uint option)
{
	static const char *str[] =
	{
		#if defined(CONFIG_BASE_IOS) || defined(CONFIG_BASE_ANDROID) || CONFIG_ENV_WEBOS_OS >= 3
		"Auto",
		#endif

		#if defined(CONFIG_BASE_IOS) || defined(CONFIG_BASE_ANDROID) || (defined CONFIG_ENV_WEBOS && CONFIG_ENV_WEBOS_OS <= 2)
		"Landscape", "Landscape 2", "Portrait", "Portrait 2"
		#else
		"90 Left", "90 Right", "Standard", "Upside Down"
		#endif
	};
	int baseVal = 0;
	#if defined(CONFIG_BASE_IOS) || defined(CONFIG_BASE_ANDROID) || CONFIG_ENV_WEBOS_OS >= 3
	baseVal = -1;
	#endif
	uint initVal = O_AUTO;
	if(option == Base::VIEW_ROTATE_90)
		initVal = O_90;
	else if(option == Base::VIEW_ROTATE_270)
		initVal = O_270;
	else if(option == Base::VIEW_ROTATE_0)
		initVal = O_0;
	else if(option == Base::VIEW_ROTATE_180)
		initVal = O_180;
	item.init(str, initVal, sizeofArray(str), baseVal);
}

void OptionView::gameOrientationInit()
{
	orientationInit(gameOrientation, optionGameOrientation);
}

void OptionView::menuOrientationInit()
{
	orientationInit(menuOrientation, optionMenuOrientation);
}

void OptionView::aspectRatioInit()
{
	assert(sizeofArray(aspectRatioStr) >= EmuSystem::aspectRatioInfos);
	int val = 0;
	iterateTimes(EmuSystem::aspectRatioInfos, i)
	{
		aspectRatioStr[i] = EmuSystem::aspectRatioInfo[i].name;
		if(optionAspectRatio == EmuSystem::aspectRatioInfo[i].aspect)
		{
			val = i;
		}
	}
	aspectRatio.init(aspectRatioStr, val, EmuSystem::aspectRatioInfos);
}

#ifdef CONFIG_AUDIO_LATENCY_HINT
void OptionView::soundBuffersInit()
{
	static const char *str2[] = { "2", "3", "4", "5", "6", "7", "8", "9", "10", "11", "12" };
	static const char *str3[] = { "3", "4", "5", "6", "7", "8", "9", "10", "11", "12" };
	static_assert(sizeofArray(str2) == 12 - (OPTION_SOUND_BUFFERS_MIN-1) || sizeofArray(str3) == 12 - (OPTION_SOUND_BUFFERS_MIN-1), "incorrect sound buffers string array");
	soundBuffers.init((OPTION_SOUND_BUFFERS_MIN == 2) ? str2 : str3, std::max((int)optionSoundBuffers - (int)OPTION_SOUND_BUFFERS_MIN, 0), (OPTION_SOUND_BUFFERS_MIN == 2) ? sizeofArray(str2) : sizeofArray(str3));
}
#endif

void OptionView::zoomInit()
{
	static const char *str[] = { "100%", "90%", "80%", "70%", "Integer-only", "Integer-only (Height)" };
	int val = 0;
	switch(optionImageZoom.val)
	{
		bcase 100: val = 0;
		bcase 90: val = 1;
		bcase 80: val = 2;
		bcase 70: val = 3;
		bcase optionImageZoomIntegerOnly: val = 4;
		bcase optionImageZoomIntegerOnlyY: val = 5;
	}
	zoom.init(str, val, sizeofArray(str));
}

void OptionView::viewportZoomInit()
{
	static const char *str[] = { "100%", "95%", "90%", "85%" };
	int val = 0;
	switch(optionViewportZoom.val)
	{
		bcase 100: val = 0;
		bcase 95: val = 1;
		bcase 90: val = 2;
		bcase 85: val = 3;
	}
	viewportZoom.init(str, val, sizeofArray(str));
}

#ifdef CONFIG_GFX_OPENGL_SHADER_PIPELINE
void OptionView::imgEffectInit()
{
	static const char *str[] {"Off", "hq2x"};
	uint init = 0;
	switch(optionImgEffect)
	{
		bcase VideoImageEffect::HQ2X: init = 1;
	}
	imgEffect.init(str, init, sizeofArray(str));
}
#endif

void OptionView::overlayEffectInit()
{
	static const char *str[] = { "Off", "Scanlines", "Scanlines 2x", "CRT Mask", "CRT", "CRT 2x" };
	uint init = 0;
	switch(optionOverlayEffect)
	{
		bcase VideoImageOverlay::SCANLINES: init = 1;
		bcase VideoImageOverlay::SCANLINES_2: init = 2;
		bcase VideoImageOverlay::CRT: init = 3;
		bcase VideoImageOverlay::CRT_RGB: init = 4;
		bcase VideoImageOverlay::CRT_RGB_2: init = 5;
	}
	overlayEffect.init(str, init, sizeofArray(str));
}

void OptionView::overlayEffectLevelInit()
{
	static const char *str[] = { "10%", "25%", "33%", "50%", "66%", "75%", "100%" };
	uint init = 0;
	switch(optionOverlayEffectLevel)
	{
		bcase 25: init = 1;
		bcase 33: init = 2;
		bcase 50: init = 3;
		bcase 66: init = 4;
		bcase 75: init = 5;
		bcase 100: init = 6;
	}
	overlayEffectLevel.init(str, init, sizeofArray(str));
}

void OptionView::fontSizeInit()
{
	static const char *str[] =
	{
			"2", "2.5",
			"3", "3.5",
			"4", "4.5",
			"5", "5.5",
			"6", "6.5",
			"7", "7.5",
			"8", "8.5",
			"9", "9.5",
			"10", "10.50",
	};
	uint init = 0;
	switch(optionFontSize)
	{
		bcase 2500: init = 1;
		bcase 3000: init = 2;
		bcase 3500: init = 3;
		bcase 4000: init = 4;
		bcase 4500: init = 5;
		bcase 5000: init = 6;
		bcase 5500: init = 7;
		bcase 6000: init = 8;
		bcase 6500: init = 9;
		bcase 7000: init = 10;
		bcase 7500: init = 11;
		bcase 8000: init = 12;
		bcase 8500: init = 13;
		bcase 9000: init = 14;
		bcase 9500: init = 15;
		bcase 10000: init = 16;
		bcase 10500: init = 17;
	}
	fontSize.init(str, init, sizeofArray(str));
}

#ifdef CONFIG_BASE_ANDROID
void OptionView::relativePointerDecelInit()
{
	static const char *str[] = { "Low", "Med.", "High" };
	int init = 0;
	if(optionRelPointerDecel == optionRelPointerDecelLow)
		init = 0;
	if(optionRelPointerDecel == optionRelPointerDecelMed)
		init = 1;
	if(optionRelPointerDecel == optionRelPointerDecelHigh)
		init = 2;
	relativePointerDecel.init(str, init, sizeofArray(str));
}
#endif

#if defined CONFIG_BASE_ANDROID && CONFIG_ENV_ANDROID_MINSDK >= 9
void OptionView::processPriorityInit()
{
	static const char *str[] = { "Normal", "High", "Very High" };
	auto prio = Base::processPriority();
	int init = 0;
	if(optionProcessPriority.val == 0)
		init = 0;
	if(optionProcessPriority.val == -6)
		init = 1;
	if(optionProcessPriority.val == -14)
		init = 2;
	processPriority.init(str, init, sizeofArray(str));
}
#endif

static int dirFsFilter(const char *name, int type)
{
	return type == Fs::TYPE_DIR;
}

template <size_t S>
static void printPathMenuEntryStr(char (&str)[S])
{
	FsSys::cPath basenameTemp;
	string_printf(str, "Save Path: %s", strlen(optionSavePath) ? string_basename(optionSavePath, basenameTemp) : "Same as Game");
}

static class SavePathSelectMenu
{
public:
	PathChangeDelegate onPathChange;

	constexpr SavePathSelectMenu() {}

	void onClose(const Input::Event &e)
	{
		snprintf(optionSavePath, sizeof(FsSys::cPath), "%s", FsSys::workDir());
		logMsg("set save path %s", (char*)optionSavePath);
		if(onPathChange) onPathChange(optionSavePath);
		workDirStack.pop();
	}

	void init(bool highlightFirst)
	{
		static const char *str[] { "Set Custom Path", "Same as Game" };
		auto &multiChoiceView = *menuAllocator.allocNew<MultiChoiceView>("Save Path", Base::mainWindow());
		multiChoiceView.init(str, sizeofArray(str), highlightFirst);
		multiChoiceView.onSelect() =
			[this](int i, const Input::Event &e)
			{
				viewStack.popAndShow();
				if(i == 0)
				{
					workDirStack.push();
					FsSys::chdir(optionSavePath);
					auto &fPicker = *allocModalView<EmuFilePicker>(Base::mainWindow());
					fPicker.init(!e.isPointer(), true, dirFsFilter);
					fPicker.onClose() = [this](FSPicker &picker, const Input::Event &e)
						{
							onClose(e);
							picker.dismiss();
						};
					modalViewController.pushAndShow(fPicker);
				}
				else
				{
					strcpy(optionSavePath, "");
					if(onPathChange) onPathChange("");
				}
				return 0;
			};
		viewStack.pushAndShow(multiChoiceView, &menuAllocator);
	}
} pathSelectMenu;

void FirmwarePathSelector::onClose(const Input::Event &e)
{
	snprintf(optionFirmwarePath, sizeof(FsSys::cPath), "%s", FsSys::workDir());
	logMsg("set firmware path %s", (char*)optionFirmwarePath);
	if(onPathChange) onPathChange(optionFirmwarePath);
	workDirStack.pop();
}

void FirmwarePathSelector::init(const char *name, bool highlightFirst)
{
	static const char *str[] { "Set Custom Path", "Default" };
	auto &multiChoiceView = *menuAllocator.allocNew<MultiChoiceView>(name, Base::mainWindow());
	multiChoiceView.init(str, sizeofArray(str), highlightFirst);
	multiChoiceView.onSelect() =
		[this](int i, const Input::Event &e)
		{
			viewStack.popAndShow();
			if(i == 0)
			{
				workDirStack.push();
				FsSys::chdir(optionFirmwarePath);
				auto &fPicker = *allocModalView<EmuFilePicker>(Base::mainWindow());
				fPicker.init(!e.isPointer(), true, dirFsFilter);
				fPicker.onClose() = [this](FSPicker &picker, const Input::Event &e)
					{
						onClose(e);
						picker.dismiss();
					};
				modalViewController.pushAndShow(fPicker);
			}
			else
			{
				strcpy(optionFirmwarePath, "");
				if(onPathChange) onPathChange("");
			}
			return 0;
		};
	viewStack.pushAndShow(multiChoiceView, &menuAllocator);
}

void OptionView::loadVideoItems(MenuItem *item[], uint &items)
{
	name_ = "Video Options";
	if(!optionFrameSkip.isConst) { frameSkipInit(); item[items++] = &frameSkip; }
	if(!optionGameOrientation.isConst) { gameOrientationInit(); item[items++] = &gameOrientation; }
	aspectRatioInit(); item[items++] = &aspectRatio;
	imgFilter.init(optionImgFilter); item[items++] = &imgFilter;
	#ifdef CONFIG_GFX_OPENGL_SHADER_PIPELINE
	imgEffectInit(); item[items++] = &imgEffect;
	#endif
	overlayEffectInit(); item[items++] = &overlayEffect;
	overlayEffectLevelInit(); item[items++] = &overlayEffectLevel;
	zoomInit(); item[items++] = &zoom;
	viewportZoomInit(); item[items++] = &viewportZoom;
	#ifdef CONFIG_BASE_ANDROID
		#ifdef SUPPORT_ANDROID_DIRECT_TEXTURE
		if(Base::androidSDK() < 14)
		{
			assert(optionDirectTexture != OPTION_DIRECT_TEXTURE_UNSET);
			directTexture.init(optionDirectTexture, Gfx::supportsAndroidDirectTexture()); item[items++] = &directTexture;
		}
		#endif
		#if CONFIG_ENV_ANDROID_MINSDK >= 9
		if(!Config::MACHINE_IS_OUYA && Base::androidSDK() >= 14 && !optionSurfaceTexture.isConst)
		{
			surfaceTexture.init(optionSurfaceTexture); item[items++] = &surfaceTexture;
		}
		#endif
	if(!optionGLSyncHack.isConst)
	{
		glSyncHack.init(optionGLSyncHack); item[items++] = &glSyncHack;
	}
	#endif
	#ifdef EMU_FRAMEWORK_BEST_COLOR_MODE_OPTION
	bestColorModeHint.init(optionBestColorModeHint); item[items++] = &bestColorModeHint;
	#endif
	if(!optionDitherImage.isConst)
	{
		dither.init(optionDitherImage); item[items++] = &dither;
	}
	#ifdef CONFIG_BASE_MULTI_WINDOW
	//secondDisplay.init(false); item[items++] = &secondDisplay;
	#endif
}

void OptionView::loadAudioItems(MenuItem *item[], uint &items)
{
	name_ = "Audio Options";
	snd.init(optionSound); item[items++] = &snd;
	if(!optionSoundRate.isConst) { audioRateInit(); item[items++] = &audioRate; }
	#ifdef CONFIG_AUDIO_LATENCY_HINT
	soundBuffersInit(); item[items++] = &soundBuffers;
	#endif
#ifdef EMU_FRAMEWORK_STRICT_UNDERRUN_CHECK_OPTION
	sndUnderrunCheck.init(optionSoundUnderrunCheck); item[items++] = &sndUnderrunCheck;
	#endif
	#ifdef CONFIG_AUDIO_SOLO_MIX
	audioSoloMix.init(!optionAudioSoloMix); item[items++] = &audioSoloMix;
	#endif
}

void OptionView::loadInputItems(MenuItem *item[], uint &items)
{
	name_ = "Input Options";
	#ifdef CONFIG_BLUETOOTH_SCAN_SECS
	btScanSecsInit(); item[items++] = &btScanSecs;
	#endif
	#ifdef CONFIG_BLUETOOTH
	if(!optionKeepBluetoothActive.isConst)
	{
		keepBtActive.init(optionKeepBluetoothActive); item[items++] = &keepBtActive;
	}
	#endif
	#ifdef CONFIG_BLUETOOTH_SCAN_CACHE_USAGE
	btScanCache.init(optionBlueToothScanCache); item[items++] = &btScanCache;
	#endif
	#ifdef CONFIG_BASE_ANDROID
	if(Base::hasTrackball())
	{
		relativePointerDecelInit(); item[items++] = &relativePointerDecel;
	}
	#endif
}

void OptionView::loadSystemItems(MenuItem *item[], uint &items)
{
	name_ = "System Options";
	autoSaveStateInit(); item[items++] = &autoSaveState;
	confirmAutoLoadState.init(optionConfirmAutoLoadState); item[items++] = &confirmAutoLoadState;
	confirmOverwriteState.init(optionConfirmOverwriteState); item[items++] = &confirmOverwriteState;
	printPathMenuEntryStr(savePathStr);
	savePath.init(savePathStr, true); item[items++] = &savePath;
	fastForwardSpeedinit(); item[items++] = &fastForwardSpeed;
	#if defined(CONFIG_INPUT_ANDROID) && CONFIG_ENV_ANDROID_MINSDK >= 9
	processPriorityInit(); item[items++] = &processPriority;
	#endif
}

void OptionView::loadGUIItems(MenuItem *item[], uint &items)
{
	name_ = "GUI Options";
	if(!optionMenuOrientation.isConst) { menuOrientationInit(); item[items++] = &menuOrientation; }
	if(!optionPauseUnfocused.isConst)
	{
		pauseUnfocused.init(optionPauseUnfocused); item[items++] = &pauseUnfocused;
	}
	if(!optionNotificationIcon.isConst)
	{
		notificationIcon.init(optionNotificationIcon); item[items++] = &notificationIcon;
	}
	if(!optionTitleBar.isConst)
	{
		navView.init(optionTitleBar); item[items++] = &navView;
	}
	if(!View::needsBackControlIsConst)
	{
		backNav.init(View::needsBackControl); item[items++] = &backNav;
	}
	rememberLastMenu.init(optionRememberLastMenu); item[items++] = &rememberLastMenu;
	if(!optionFontSize.isConst)
	{
		fontSizeInit(); item[items++] = &fontSize;
	}
	#ifndef CONFIG_ENV_WEBOS
	altGamepadConfirm.init(Input::swappedGamepadConfirm); item[items++] = &altGamepadConfirm;
	#endif
	if(!optionIdleDisplayPowerSave.isConst)
	{
		idleDisplayPowerSave.init(optionIdleDisplayPowerSave); item[items++] = &idleDisplayPowerSave;
	}
	if(!optionLowProfileOSNav.isConst)
	{
		uiVisibiltyInit(optionLowProfileOSNav, lowProfileOSNav);
		item[items++] = &lowProfileOSNav;
	}
	if(!optionHideOSNav.isConst)
	{
		uiVisibiltyInit(optionHideOSNav, hideOSNav);
		item[items++] = &hideOSNav;
	}
	if(!optionHideStatusBar.isConst)
	{
		uiVisibiltyInit(optionHideStatusBar, statusBar); item[items++] = &statusBar;
	}
	#ifdef EMU_FRAMEWORK_BUNDLED_GAMES
	showBundledGames.init(optionShowBundledGames); item[items++] = &showBundledGames;
	#endif
}

void OptionView::init(uint idx, bool highlightFirst)
{
	uint i = 0;
	switch(idx)
	{
		bcase 0: loadVideoItems(item, i);
		bcase 1: loadAudioItems(item, i);
		bcase 2: loadInputItems(item, i);
		bcase 3: loadSystemItems(item, i);
		bcase 4: loadGUIItems(item, i);
	}
	assert(i <= sizeofArray(item));
	BaseMenuView::init(item, i, highlightFirst);
}

OptionView::OptionView(Base::Window &win):
	BaseMenuView("Options", win),
	// Video
	#ifdef CONFIG_BASE_ANDROID
		#ifdef SUPPORT_ANDROID_DIRECT_TEXTURE
		directTexture
		{
			"Direct Texture",
			[this](BoolMenuItem &item, const Input::Event &e)
			{
				if(!item.active)
				{
					popup.postError(Gfx::androidDirectTextureError());
					return;
				}
				item.toggle(*this);
				Gfx::setUseAndroidDirectTexture(item.on);
				optionDirectTexture = item.on;
				if(emuView.vidImg)
					emuView.reinitImage();
			}
		},
		#endif
		#if CONFIG_ENV_ANDROID_MINSDK >= 9
		surfaceTexture
		{
			"Fast CPU->GPU Copy",
			[this](BoolMenuItem &item, const Input::Event &e)
			{
				item.toggle(*this);
				optionSurfaceTexture = item.on;
				Gfx::setUseAndroidSurfaceTexture(item.on);
				#ifdef CONFIG_GFX_OPENGL_SHADER_PIPELINE
				// re-apply effect so any shaders are re-compiled
				emuView.vidImgEffect.setEffect(optionImgEffect);
				#endif
				if(emuView.vidImg)
				{
					emuView.reinitImage();
					#ifdef CONFIG_GFX_OPENGL_SHADER_PIPELINE
					emuView.placeEffect();
					#endif
				}
			}
		},
		#endif
	glSyncHack
	{
		"GPU Sync Hack",
		[this](BoolMenuItem &item, const Input::Event &e)
		{
			item.toggle(*this);
			glSyncHackEnabled = item.on;
		}
	},
	#endif
	frameSkip
	{
		"Frame Skip",
		[](MultiChoiceMenuItem &, int val)
		{
			if(val == -1)
			{
				optionFrameSkip.val = EmuSystem::optionFrameSkipAuto;
				logMsg("set auto frame skip");
			}
			else
			{
				optionFrameSkip.val = val;
				logMsg("set frame skip: %d", int(optionFrameSkip));
			}
			EmuSystem::configAudioPlayback();
		}
	},
	aspectRatio
	{
		"Aspect Ratio",
		[](MultiChoiceMenuItem &, int val)
		{
			optionAspectRatio.val = EmuSystem::aspectRatioInfo[val].aspect;
			logMsg("set aspect ratio: %u:%u", optionAspectRatio.val.x, optionAspectRatio.val.y);
			emuView.placeEmu();
		}
	},
	zoom
	{
		"Zoom",
		[](MultiChoiceMenuItem &, int val)
		{
			switch(val)
			{
				bcase 0: optionImageZoom.val = 100;
				bcase 1: optionImageZoom.val = 90;
				bcase 2: optionImageZoom.val = 80;
				bcase 3: optionImageZoom.val = 70;
				bcase 4: optionImageZoom.val = optionImageZoomIntegerOnly;
				bcase 5: optionImageZoom.val = optionImageZoomIntegerOnlyY;
			}
			logMsg("set image zoom: %d", int(optionImageZoom));
			emuView.placeEmu();
		}
	},
	viewportZoom
	{
		"Screen Area",
		[this](MultiChoiceMenuItem &, int val)
		{
			switch(val)
			{
				bcase 0: optionViewportZoom.val = 100;
				bcase 1: optionViewportZoom.val = 95;
				bcase 2: optionViewportZoom.val = 90;
				bcase 3: optionViewportZoom.val = 85;
			}
			logMsg("set viewport zoom: %d", int(optionViewportZoom));
			window().dispatchResize();
		}
	},
	imgFilter
	{
		"Image Interpolation", "None", "Linear",
		[this](BoolMenuItem &item, const Input::Event &e)
		{
			item.toggle(*this);
			optionImgFilter.val = item.on;
			if(emuView.disp.image()
				#ifdef CONFIG_GFX_OPENGL_SHADER_PIPELINE
				&& !emuView.vidImgEffect.effect()
				#endif
				)
				emuView.vidImg.setFilter(item.on);
		}
	},
	#ifdef CONFIG_GFX_OPENGL_SHADER_PIPELINE
	imgEffect
	{
		"Image Effect",
		[](MultiChoiceMenuItem &, int val)
		{
			uint setVal = 0;
			switch(val)
			{
				bcase 1: setVal = VideoImageEffect::HQ2X;
			}
			optionImgEffect.val = setVal;
			emuView.vidImgEffect.setEffect(setVal);
			if(emuView.disp.image())
			{
				emuView.compileDefaultPrograms();
				emuView.placeEffect();
			}
		}
	},
	#endif
	overlayEffect
	{
		"Overlay Effect",
		[](MultiChoiceMenuItem &, int val)
		{
			uint setVal = 0;
			switch(val)
			{
				bcase 1: setVal = VideoImageOverlay::SCANLINES;
				bcase 2: setVal = VideoImageOverlay::SCANLINES_2;
				bcase 3: setVal = VideoImageOverlay::CRT;
				bcase 4: setVal = VideoImageOverlay::CRT_RGB;
				bcase 5: setVal = VideoImageOverlay::CRT_RGB_2;
			}
			optionOverlayEffect.val = setVal;
			emuView.vidImgOverlay.setEffect(setVal);
			emuView.placeOverlay();
		}
	},
	overlayEffectLevel
	{
		"Overlay Effect Level",
		[](MultiChoiceMenuItem &, int val)
		{
			uint setVal = 10;
			switch(val)
			{
				bcase 1: setVal = 25;
				bcase 2: setVal = 33;
				bcase 3: setVal = 50;
				bcase 4: setVal = 66;
				bcase 5: setVal = 75;
				bcase 6: setVal = 100;
			}
			optionOverlayEffectLevel.val = setVal;
			emuView.vidImgOverlay.intensity = setVal/100.;
		}
	},
	#if defined EMU_FRAMEWORK_BEST_COLOR_MODE_OPTION
	bestColorModeHint
	{
		"Use Highest Color Mode",
		[this](BoolMenuItem &item, const Input::Event &e)
		{
			auto alertMsg = Config::envIsAndroid ? "This option takes effect next time you launch the app. "
					"Not all devices properly support high color modes so turn this off "
					"if you experience graphics problems."
					: "This option takes effect next time you launch the app. "
					"If off, it may improve performance at the cost of color accuracy.";
			auto onMsg = Config::envIsAndroid ? "Enable" : "Toggle";
			if((!item.on && Config::envIsAndroid) || !Config::envIsAndroid)
			{
				auto &ynAlertView = *allocModalView<YesNoAlertView>(window());
				ynAlertView.init(alertMsg, !e.isPointer(), onMsg, "Cancel");
				ynAlertView.onYes() =
					[this, &item](const Input::Event &e)
					{
						item.toggle(*this);
						optionBestColorModeHint = item.on;
					};
				modalViewController.pushAndShow(ynAlertView);
			}
			else
			{
				item.toggle(*this);
				optionBestColorModeHint = item.on;
			}
		}
	},
	#endif
	#ifdef CONFIG_BASE_MULTI_WINDOW
	secondDisplay
	{
		"2nd Window (for testing only)",
		[this](BoolMenuItem &item, const Input::Event &e)
		{
			item.toggle(*this);
			extern Base::Window secondWin;
			if(item.on)
				secondWin.init({0, 0}, {0, 0});
			else
				secondWin.deinit();
		}
	},
	#endif
	dither
	{
		"Dither Image",
		[this](BoolMenuItem &item, const Input::Event &e)
		{
			item.toggle(*this);
			optionDitherImage = item.on;
			Gfx::setDither(item.on);
		}
	},
	gameOrientation
	{
		"Orientation",
		[](MultiChoiceMenuItem &, int val)
		{
			optionGameOrientation.val = convertOrientationMenuValueToOption(val);
			logMsg("set game orientation: %s", Base::orientationToStr(int(optionGameOrientation)));
		}
	},
	// Audio
	snd
	{
		"Sound",
		[this](BoolMenuItem &item, const Input::Event &e)
		{
			item.toggle(*this);
			optionSound = item.on;
			if(!item.on)
				Audio::closePcm();
		}
	},
	#ifdef CONFIG_AUDIO_LATENCY_HINT
	soundBuffers
	{
		"Buffer Size In Frames",
		[](MultiChoiceMenuItem &, int val)
		{
			if(Audio::isOpen())
				Audio::closePcm();
			optionSoundBuffers = val+OPTION_SOUND_BUFFERS_MIN;
		}
	},
	#endif
	audioRate
	{
		"Sound Rate",
		[](MultiChoiceMenuItem &, int val)
		{
			uint rate = 44100;
			switch(val)
			{
				bcase 0: rate = 22050;
				bcase 1: rate = 32000;
				bcase 3: rate = 48000;
			}
			if(rate != optionSoundRate)
			{
				optionSoundRate = rate;
				EmuSystem::configAudioPlayback();
			}
		}
	},
	#ifdef EMU_FRAMEWORK_STRICT_UNDERRUN_CHECK_OPTION
	sndUnderrunCheck
	{
		"Strict Underrun Check",
		[this](BoolMenuItem &item, const Input::Event &e)
		{
			if(Audio::isOpen())
				Audio::closePcm();
			item.toggle(*this);
			optionSoundUnderrunCheck = item.on;
		}
	},
	#endif
	#ifdef CONFIG_AUDIO_SOLO_MIX
	audioSoloMix
	{
		"Mix With Other Apps",
		[this](BoolMenuItem &item, const Input::Event &e)
		{
			item.toggle(*this);
			optionAudioSoloMix = !item.on;
		}
	},
	#endif
	// Input
	altGamepadConfirm
	{
		"Alt Gamepad Confirm",
		[this](BoolMenuItem &item, const Input::Event &e)
		{
			item.toggle(*this);
			Input::swappedGamepadConfirm = item.on;
		}
	},
	#ifdef CONFIG_BLUETOOTH_SCAN_SECS
	btScanSecs
	{
		"Bluetooth Scan",
		[](MultiChoiceMenuItem &, int val)
		{
			switch(val)
			{
				bcase 0: BluetoothAdapter::scanSecs = 2;
				bcase 1: BluetoothAdapter::scanSecs = 4;
				bcase 2: BluetoothAdapter::scanSecs = 6;
				bcase 3: BluetoothAdapter::scanSecs = 8;
				bcase 4: BluetoothAdapter::scanSecs = 10;
			}
			logMsg("set bluetooth scan time %d", BluetoothAdapter::scanSecs);
		}
	},
	#endif
	#ifdef CONFIG_BLUETOOTH
	keepBtActive
	{
		"Background Bluetooth",
		[this](BoolMenuItem &item, const Input::Event &e)
		{
			item.toggle(*this);
			optionKeepBluetoothActive = item.on;
		}
	},
	#endif
	#ifdef CONFIG_BLUETOOTH_SCAN_CACHE_USAGE
	btScanCache
	{
		"Bluetooth Scan Cache",
		[this](BoolMenuItem &item, const Input::Event &e)
		{
			item.toggle(*this);
			optionBlueToothScanCache = item.on;
		}
	},
	#endif
	#ifdef CONFIG_BASE_ANDROID
	relativePointerDecel
	{
		"Trackball Sensitivity",
		[](MultiChoiceMenuItem &, int val)
		{
			#if defined(CONFIG_BASE_ANDROID)
			if(val == 0)
				optionRelPointerDecel.val = optionRelPointerDecelLow;
			else if(val == 1)
				optionRelPointerDecel.val = optionRelPointerDecelMed;
			else if(val == 2)
				optionRelPointerDecel.val = optionRelPointerDecelHigh;
			#endif
		}
	},
	#endif
	// System
	autoSaveState
	{
		"Auto-save State",
		[](MultiChoiceMenuItem &, int val)
		{
			switch(val)
			{
				bcase 0: optionAutoSaveState.val = 0;
				bcase 1: optionAutoSaveState.val = 1;
				bcase 2: optionAutoSaveState.val = 15;
				bcase 3: optionAutoSaveState.val = 30;
			}
			logMsg("set auto-savestate %d", optionAutoSaveState.val);
		}
	},
	confirmAutoLoadState
	{
		"Confirm Auto-load State",
		[this](BoolMenuItem &item, const Input::Event &e)
		{
			item.toggle(*this);
			optionConfirmAutoLoadState = item.on;
		}
	},
	confirmOverwriteState
	{
		"Confirm Overwrite State",
		[this](BoolMenuItem &item, const Input::Event &e)
		{
			item.toggle(*this);
			optionConfirmOverwriteState = item.on;
		}
	},
	savePath
	{
		"",
		[this](TextMenuItem &, const Input::Event &e)
		{
			pathSelectMenu.init(!e.isPointer());
			pathSelectMenu.onPathChange =
				[this](const char *newPath)
				{
					printPathMenuEntryStr(savePathStr);
					savePath.compile();
					EmuSystem::savePathChanged();
				};
			postDraw();
		}
	},
	fastForwardSpeed
	{
		"Fast Forward Speed",
		[this](MultiChoiceMenuItem &, int val)
		{
			optionFastForwardSpeed = val + MIN_FAST_FORWARD_SPEED;
		}
	},
	#if defined CONFIG_BASE_ANDROID && CONFIG_ENV_ANDROID_MINSDK >= 9
	processPriority
	{
		"Process Priority",
		[](MultiChoiceMenuItem &, int val)
		{
			if(val == 0)
				optionProcessPriority.val = 0;
			else if(val == 1)
				optionProcessPriority.val = -6;
			else if(val == 2)
				optionProcessPriority.val = -14;
			Base::setProcessPriority(optionProcessPriority);
		}
	},
	#endif
	// GUI
	pauseUnfocused
	{
		Config::envIsPS3 ? "Pause in XMB" : "Pause if unfocused",
		[this](BoolMenuItem &item, const Input::Event &e)
		{
			item.toggle(*this);
			optionPauseUnfocused = item.on;
		}
	},
	fontSize
	{
		"Font Size",
		[this](MultiChoiceMenuItem &, int val)
		{
			optionFontSize = 3000;
			switch(val)
			{
				bcase 0: optionFontSize = 2000;
				bcase 1: optionFontSize = 2500;
				// 2: 3000
				bcase 3: optionFontSize = 3500;
				bcase 4: optionFontSize = 4000;
				bcase 5: optionFontSize = 4500;
				bcase 6: optionFontSize = 5000;
				bcase 7: optionFontSize = 5500;
				bcase 8: optionFontSize = 6000;
				bcase 9: optionFontSize = 6500;
				bcase 10: optionFontSize = 7000;
				bcase 11: optionFontSize = 7500;
				bcase 12: optionFontSize = 8000;
				bcase 13: optionFontSize = 8500;
				bcase 14: optionFontSize = 9000;
				bcase 15: optionFontSize = 9500;
				bcase 16: optionFontSize = 10000;
				bcase 17: optionFontSize = 10500;
			}
			setupFont();
			EmuControls::setupVControllerVars();
			window().dispatchResize();
		}
	},
	notificationIcon
	{
		"Suspended App Icon",
		[this](BoolMenuItem &item, const Input::Event &e)
		{
			item.toggle(*this);
			optionNotificationIcon = item.on;
		}
	},
	statusBar
	{
		"Hide Status Bar",
		[](MultiChoiceMenuItem &, int val)
		{
			optionHideStatusBar = val;
			applyOSNavStyle(false);
		}
	},
	lowProfileOSNav
	{
		"Dim OS Navigation",
		[](MultiChoiceMenuItem &, int val)
		{
			optionLowProfileOSNav = val;
			applyOSNavStyle(false);
		}
	},
	hideOSNav
	{
		"Hide OS Navigation",
		[](MultiChoiceMenuItem &, int val)
		{
			optionHideOSNav = val;
			applyOSNavStyle(false);
		}
	},
	idleDisplayPowerSave
	{
		"Dim Screen If Idle",
		[this](BoolMenuItem &item, const Input::Event &e)
		{
			item.toggle(*this);
			optionIdleDisplayPowerSave = item.on;
			Base::setIdleDisplayPowerSave(item.on);
		}
	},
	navView
	{
		"Title Bar",
		[this](BoolMenuItem &item, const Input::Event &e)
		{
			item.toggle(*this);
			optionTitleBar = item.on;
			viewStack.setNavView(item.on ? &viewNav : 0);
			window().dispatchResize();
		}
	},
	backNav
	{
		"Title Back Navigation",
		[this](BoolMenuItem &item, const Input::Event &e)
		{
			item.toggle(*this);
			View::setNeedsBackControl(item.on);
			viewNav.setBackImage(View::needsBackControl ? &getAsset(ASSET_ARROW) : nullptr);
			window().dispatchResize();
		}
	},
	rememberLastMenu
	{
		"Remember Last Menu",
		[this](BoolMenuItem &item, const Input::Event &e)
		{
			item.toggle(*this);
			optionRememberLastMenu = item.on;
		}
	},
	#ifdef EMU_FRAMEWORK_BUNDLED_GAMES
	showBundledGames
	{
		"Show Bundled Games",
		[this](BoolMenuItem &item, const Input::Event &e)
		{
			item.toggle(*this);
			optionShowBundledGames = item.on;
			mainMenu().deinit();
			initMainMenu(window());
		}
	},
	#endif
	menuOrientation
	{
		"Orientation",
		[this](MultiChoiceMenuItem &, int val)
		{
			optionMenuOrientation.val = convertOrientationMenuValueToOption(val);
			Base::mainWindow().setValidOrientations(optionMenuOrientation);
			window().dispatchResize();
			logMsg("set menu orientation: %s", Base::orientationToStr(int(optionMenuOrientation)));
		}
	}
{}
