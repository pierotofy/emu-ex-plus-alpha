include $(IMAGINE_PATH)/make/config.mk
export imagineLibExt := -jb
ios_arch ?= armv6 armv7
include $(buildSysPath)/shortcut/meta-builds/ios-release.mk
