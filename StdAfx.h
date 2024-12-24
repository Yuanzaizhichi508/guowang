#if !defined(AFX_STDAFX_H__7788D43B_D613_4B67_8A98_E4B7B63444D3__INCLUDED_)
#define AFX_STDAFX_H__7788D43B_D613_4B67_8A98_E4B7B63444D3__INCLUDED_

#if _MSC_VER > 1000
#pragma once
#endif // _MSC_VER > 1000

#define VC_EXTRALEAN		// Exclude rarely-used stuff from Windows headers
#ifndef _WIN32_WINNT
#define _WIN32_WINNT 0x0501
#endif
#include <afxwin.h>         // MFC core and standard components
#include <afxext.h>         // MFC extensions
#include <afxdisp.h>        // MFC Automation classes
#include <afxdtctl.h>		// MFC support for Internet Explorer 4 Common Controls
#ifndef _AFX_NO_AFXCMN_SUPPORT
#include <afxcmn.h>			// MFC support for Windows Common Controls
#endif // _AFX_NO_AFXCMN_SUPPORT

#define   MAX_DEVICE_MUN  4  ///< 最多支持4台相机

#endif // STDAFX_H
