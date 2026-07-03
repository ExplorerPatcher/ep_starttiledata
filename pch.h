// pch.h: This is a precompiled header file.
// Files listed below are compiled only once, improving build performance for future builds.
// This also affects IntelliSense performance, including code completion and many code browsing features.
// However, files listed here are ALL re-compiled if any one of them is updated between builds.
// Do not add files here that you will be updating frequently as this negates the performance advantage.

#ifndef PCH_H
#define PCH_H

// add headers that you want to pre-compile here
#include "framework.h"

#include <ppltasks.h>
#include <winstring.h>

#include <algorithm>
#include <functional>
#include <map>
#include <memory>
#include <string>
#include <unordered_map>
#include <vector>

#include <wil/winrt.h>

#include <wrl.h>

#include <Windows.Foundation.h>
#include <Windows.System.h>
#include <Windows.UI.Core.h>
#include <WindowsInternal.Shell.CDSProperties.h>
#include <WindowsInternal.Shell.UnifiedTile.h>

EXTERN_C IMAGE_DOS_HEADER __ImageBase;
#define HINST_THISCOMPONENT ((HINSTANCE)&__ImageBase)

#define NUKE_SHAREDSTARTLAYOUT 0

#endif //PCH_H
