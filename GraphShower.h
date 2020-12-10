
// GraphShower.h: GraphShower 应用程序的主头文件
//
#pragma once

#ifndef __AFXWIN_H__
	#error "在包含此文件之前包含 'pch.h' 以生成 PCH"
#endif

#include "resource.h"       // 主符号


// CGraphShowerApp:
// 有关此类的实现，请参阅 GraphShower.cpp
//

class CGraphShowerApp : public CWinAppEx
{
public:
	CGraphShowerApp() noexcept;


// 重写
public:
	virtual BOOL InitInstance();
	virtual int ExitInstance();

// 实现
	UINT  m_nAppLook;
	BOOL  m_bHiColorIcons;

	virtual void PreLoadState();
	virtual void LoadCustomState();
	virtual void SaveCustomState();

	afx_msg void OnAppAbout();
	DECLARE_MESSAGE_MAP()

	//添加Gdiplus的变量
private:
	Gdiplus::GdiplusStartupInput m_GdiplusStartupInput;
	ULONG_PTR m_gdiplusToken;

};

extern CGraphShowerApp theApp;
