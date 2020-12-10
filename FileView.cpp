
#include "pch.h"
#include "framework.h"
#include "mainfrm.h"
#include "FileView.h"
#include "Resource.h"
#include "GraphShower.h"

#ifdef _DEBUG
#undef THIS_FILE
static char THIS_FILE[]=__FILE__;
#define new DEBUG_NEW
#endif

/////////////////////////////////////////////////////////////////////////////
// CFileView

CFileView::CFileView() noexcept
{
	m_bHeightOut = false;
	m_bWidhtOut = false;
	m_nShowType = ST_NORMAL;
	m_nXX = 0;
	m_nYY = 0;
	m_nXXMax = 0;
	m_nYYMax = 0;

	m_pBitmap = NULL;
	m_nPos = -1;
}

CFileView::~CFileView()
{
	::delete m_pBitmap;
}

BEGIN_MESSAGE_MAP(CFileView, CDockablePane)
	ON_WM_CREATE()
	ON_WM_SIZE()
	ON_WM_CONTEXTMENU()
	ON_COMMAND(ID_PROPERTIES, OnProperties)
	ON_COMMAND(ID_OPEN, OnFileOpen)
	ON_COMMAND(ID_OPEN_WITH, OnFileOpenWith)
	ON_COMMAND(ID_DUMMY_COMPILE, OnDummyCompile)
	ON_COMMAND(ID_EDIT_CUT, OnEditCut)
	ON_COMMAND(ID_EDIT_COPY, OnEditCopy)
	ON_COMMAND(ID_EDIT_CLEAR, OnEditClear)
	ON_WM_PAINT()
	ON_WM_SETFOCUS()
END_MESSAGE_MAP()


/////////////////////////////////////////////////////////////////////////////
// CWorkspaceBar 消息处理程序

int CFileView::OnCreate(LPCREATESTRUCT lpCreateStruct)
{
	if (CDockablePane::OnCreate(lpCreateStruct) == -1)
		return -1;

	CRect rectDummy;
	rectDummy.SetRectEmpty();

	// 创建视图: 
	const DWORD dwViewStyle = WS_CHILD | WS_VISIBLE | TVS_HASLINES | TVS_LINESATROOT | TVS_HASBUTTONS;

	if (!m_wndFileView.Create(dwViewStyle, rectDummy, this, 4))
	{
		TRACE0("未能创建文件视图\n");
		return -1;      // 未能创建
	}

	// 加载视图图像: 
	m_FileViewImages.Create(IDB_FILE_VIEW, 16, 0, RGB(255, 0, 255));
	m_wndFileView.SetImageList(&m_FileViewImages, TVSIL_NORMAL);

	m_wndToolBar.Create(this, AFX_DEFAULT_TOOLBAR_STYLE, IDR_EXPLORER);
	m_wndToolBar.LoadToolBar(IDR_EXPLORER, 0, 0, TRUE /* 已锁定*/);

	OnChangeVisualStyle();

	m_wndToolBar.SetPaneStyle(m_wndToolBar.GetPaneStyle() | CBRS_TOOLTIPS | CBRS_FLYBY);

	m_wndToolBar.SetPaneStyle(m_wndToolBar.GetPaneStyle() & ~(CBRS_GRIPPER | CBRS_SIZE_DYNAMIC | CBRS_BORDER_TOP | CBRS_BORDER_BOTTOM | CBRS_BORDER_LEFT | CBRS_BORDER_RIGHT));

	m_wndToolBar.SetOwner(this);

	// 所有命令将通过此控件路由，而不是通过主框架路由: 
	m_wndToolBar.SetRouteCommandsViaFrame(FALSE);

	// 填入一些静态树视图数据(此处只需填入虚拟代码，而不是复杂的数据)
	FillFileView();
	AdjustLayout();

	return 0;
}

void CFileView::OnSize(UINT nType, int cx, int cy)
{
	CDockablePane::OnSize(nType, cx, cy);
	AdjustLayout();
}

void CFileView::FillFileView()
{
	HTREEITEM hRoot = m_wndFileView.InsertItem(_T("FakeApp 文件"), 0, 0);
	m_wndFileView.SetItemState(hRoot, TVIS_BOLD, TVIS_BOLD);

	HTREEITEM hSrc = m_wndFileView.InsertItem(_T("FakeApp 源文件"), 0, 0, hRoot);

	m_wndFileView.InsertItem(_T("FakeApp.cpp"), 1, 1, hSrc);
	m_wndFileView.InsertItem(_T("FakeApp.rc"), 1, 1, hSrc);
	m_wndFileView.InsertItem(_T("FakeAppDoc.cpp"), 1, 1, hSrc);
	m_wndFileView.InsertItem(_T("FakeAppView.cpp"), 1, 1, hSrc);
	m_wndFileView.InsertItem(_T("MainFrm.cpp"), 1, 1, hSrc);
	m_wndFileView.InsertItem(_T("pch.cpp"), 1, 1, hSrc);

	HTREEITEM hInc = m_wndFileView.InsertItem(_T("FakeApp 头文件"), 0, 0, hRoot);

	m_wndFileView.InsertItem(_T("FakeApp.h"), 2, 2, hInc);
	m_wndFileView.InsertItem(_T("FakeAppDoc.h"), 2, 2, hInc);
	m_wndFileView.InsertItem(_T("FakeAppView.h"), 2, 2, hInc);
	m_wndFileView.InsertItem(_T("Resource.h"), 2, 2, hInc);
	m_wndFileView.InsertItem(_T("MainFrm.h"), 2, 2, hInc);
	m_wndFileView.InsertItem(_T("pch.h"), 2, 2, hInc);

	HTREEITEM hRes = m_wndFileView.InsertItem(_T("FakeApp 资源文件"), 0, 0, hRoot);

	m_wndFileView.InsertItem(_T("FakeApp.ico"), 2, 2, hRes);
	m_wndFileView.InsertItem(_T("FakeApp.rc2"), 2, 2, hRes);
	m_wndFileView.InsertItem(_T("FakeAppDoc.ico"), 2, 2, hRes);
	m_wndFileView.InsertItem(_T("FakeToolbar.bmp"), 2, 2, hRes);

	m_wndFileView.Expand(hRoot, TVE_EXPAND);
	m_wndFileView.Expand(hSrc, TVE_EXPAND);
	m_wndFileView.Expand(hInc, TVE_EXPAND);
}

void CFileView::OnContextMenu(CWnd* pWnd, CPoint point)
{
	CTreeCtrl* pWndTree = (CTreeCtrl*) &m_wndFileView;
	ASSERT_VALID(pWndTree);

	if (pWnd != pWndTree)
	{
		CDockablePane::OnContextMenu(pWnd, point);
		return;
	}

	if (point != CPoint(-1, -1))
	{
		// 选择已单击的项: 
		CPoint ptTree = point;
		pWndTree->ScreenToClient(&ptTree);

		UINT flags = 0;
		HTREEITEM hTreeItem = pWndTree->HitTest(ptTree, &flags);
		if (hTreeItem != nullptr)
		{
			pWndTree->SelectItem(hTreeItem);
		}
	}

	pWndTree->SetFocus();
	theApp.GetContextMenuManager()->ShowPopupMenu(IDR_POPUP_EXPLORER, point.x, point.y, this, TRUE);
}

void CFileView::AdjustLayout()
{
	if (GetSafeHwnd() == nullptr)
	{
		return;
	}

	CRect rectClient;
	GetClientRect(rectClient);

	int cyTlb = m_wndToolBar.CalcFixedLayout(FALSE, TRUE).cy;

	m_wndToolBar.SetWindowPos(nullptr, rectClient.left, rectClient.top, rectClient.Width(), cyTlb, SWP_NOACTIVATE | SWP_NOZORDER);
	m_wndFileView.SetWindowPos(nullptr, rectClient.left + 1, rectClient.top + cyTlb + 1, rectClient.Width() - 2, rectClient.Height() - cyTlb - 2, SWP_NOACTIVATE | SWP_NOZORDER);
}

void CFileView::OnProperties()
{
	AfxMessageBox(_T("属性...."));

}
/******************************************************************************
*	作用:		"打开"的命令响应函数
******************************************************A************************/
void CFileView::OnFileOpen()
{
	// TODO: 在此添加命令处理程序代码
	CFileDialog dlg(true, NULL, L"",
		OFN_HIDEREADONLY | OFN_OVERWRITEPROMPT | OFN_NOCHANGEDIR,
		L"所有图像文件（*.bmp;*.jpg)|*.bmp;*.jpg|所有文件（*.*)|*.*||");
	if (dlg.DoModal() == IDOK)
	{
		OpenFile(dlg.GetPathName());
		FixOffset();//根据显示模式，修正图像的偏移坐标等信息
		RefreshIndicator();//更新指示器
		Invalidate();//重新绘制view
	}
}


void CFileView::OnFileOpenWith()
{
	// TODO: 在此处添加命令处理程序代码
}

void CFileView::OnDummyCompile()
{
	// TODO: 在此处添加命令处理程序代码
}

void CFileView::OnEditCut()
{
	// TODO: 在此处添加命令处理程序代码
}

void CFileView::OnEditCopy()
{
	// TODO: 在此处添加命令处理程序代码
}

void CFileView::OnEditClear()
{
	// TODO: 在此处添加命令处理程序代码
}

void CFileView::OnPaint()
{
	CPaintDC dc(this); // 用于绘制的设备上下文

	CRect rectTree;
	m_wndFileView.GetWindowRect(rectTree);
	ScreenToClient(rectTree);

	rectTree.InflateRect(1, 1);
	dc.Draw3dRect(rectTree, ::GetSysColor(COLOR_3DSHADOW), ::GetSysColor(COLOR_3DSHADOW));

	//绘制图片
	//CRect rect;
	//GetClientRect(rect);
	CDC* pDC = this->GetDC();
	CDC memDC;
	CBitmap MemBitmap;

	// 设备描述表初始化
	memDC.CreateCompatibleDC(NULL);

	// 建立与屏幕显示兼容的内存显示设备
	MemBitmap.CreateCompatibleBitmap(pDC, rectTree.Width(), rectTree.Height());

	// 选取空白位图
	memDC.SelectObject(MemBitmap);
	memDC.FillSolidRect(0, 0, rectTree.Width(), rectTree.Height(), RGB(255, 255, 255));


	if (m_nPos >= 0)
		ShowPicture(&memDC, *(m_pBitmap),m_nShowType);

	pDC->BitBlt(0, 0, rectTree.Width(), rectTree.Height(), &memDC, 0, 0, SRCCOPY);

	MemBitmap.DeleteObject();
	memDC.DeleteDC();
}

void CFileView::OnSetFocus(CWnd* pOldWnd)
{
	CDockablePane::OnSetFocus(pOldWnd);

	m_wndFileView.SetFocus();
}

void CFileView::OpenFile(const CString& strPath)
{
	m_FilesNameAry.RemoveAll();

	int nIndex = strPath.ReverseFind('\\');
	m_strPath = strPath.Left(nIndex);
	CString strFileName = strPath.Right(strPath.GetLength() - nIndex - 1);

	CMainFrame::GetAllFileNames(m_strPath, m_FilesNameAry);//获取指定目录下的所有图像文件名
	m_nPicNum = (int)m_FilesNameAry.GetCount();

	m_nPos = 0;

	while (m_nPos < m_nPicNum && m_FilesNameAry[m_nPos] != strFileName)
		m_nPos++;

	PreDrawImage();//内存中预先绘制图像
	Invalidate();
}

/******************************************************************************
*	作用:		在内存中预先绘制图像
******************************************************************************/
void CFileView::PreDrawImage(void)
{
	Gdiplus::Image image(GetFilePath(m_nPos));

	::delete m_pBitmap;

	m_pBitmap = ::new Gdiplus::Bitmap(image.GetWidth(), image.GetHeight(), PixelFormat32bppARGB);
	Gdiplus::Graphics* graph = Gdiplus::Graphics::FromImage(m_pBitmap);

	graph->DrawImage(&image, 0, 0, image.GetWidth(), image.GetHeight());
	SAFE_DELETE(graph);
}
//获得指定指引文件的全路径
CString CFileView::GetFilePath(int nIndex)
{
	ASSERT(nIndex >= 0 && nIndex < m_nPicNum);
	return m_strPath + L"\\" + m_FilesNameAry[nIndex];
}

/******************************************************************************
*	作用:		修正图像的偏移坐标等信息
*	备注:		以显示模式为依据
******************************************************************************/
void CFileView::FixOffset()
{
	using namespace Gdiplus;
	if (m_nPos < 0 || m_nPos >= m_nPicNum)
		return;

	Gdiplus::Image image(GetFilePath(m_nPos));

	if (image.GetLastStatus() != Ok)
		return;

	m_bHeightOut = false;
	m_bWidhtOut = false;

	CSize imgSize = GetShowPicSize(image, m_nShowType);
	CRect rect;

	GetClientRect(rect);

	switch (m_nShowType)
	{
	case ST_NORMAL:
		if (imgSize.cx > rect.Width())
		{
			m_nXXMax = imgSize.cx - rect.Width();
			m_bWidhtOut = true;
		}

		if (imgSize.cy > rect.Height())
		{
			m_nYYMax = imgSize.cy - rect.Height();
			m_bHeightOut = true;
		}
		break;

	case ST_FIXWIDTH:
		if (imgSize.cy > rect.Height())
		{
			m_nYYMax = imgSize.cy - rect.Height();
			m_bHeightOut = true;
		}
		break;

	case ST_FIXHEIGHT:
		if (imgSize.cx > rect.Width())
		{
			m_nXXMax = imgSize.cx - rect.Width();
			m_bWidhtOut = true;
		}
		break;

	case ST_FIXSCREEN:
	default:;
	}
}

void CFileView::ClearOffset(void)
{
	m_nXX = m_nYY = m_nXXMax = m_nYYMax = 0;
}

CSize CFileView::GetShowPicSize(Gdiplus::Image& image, int nShowType)
{
	using namespace Gdiplus;
	REAL width = (REAL)image.GetWidth();
	REAL height = (REAL)image.GetHeight();
	REAL c = 0.0f;

	CRect winRect;
	GetClientRect(winRect);

	switch (nShowType)
	{
	case ST_FIXWIDTH:	// 适合宽度模式
		width = (REAL)winRect.Width();
		c = width / image.GetWidth();
		height = c * image.GetHeight();
		break;

	case ST_FIXHEIGHT:	// 适合高度模式
		height = (REAL)winRect.Height();
		c = height / image.GetHeight();
		width = c * image.GetWidth();

		break;

	case ST_FIXSCREEN:	// 适合大小模式

		// 按适合宽度模式处理
		width = (REAL)winRect.Width();
		c = width / image.GetWidth();

		// 如果没有超界
		if (c * image.GetHeight() < winRect.Height())
			height = c * image.GetHeight();
		// 超界则采用适合高度模式
		else
		{
			height = (REAL)winRect.Height();
			c = height / image.GetHeight();
			width = c * image.GetWidth();
		}

		break;
	}

	return CSize((int)width, (int)height);
}

void CFileView::RefreshIndicator()
{
	CMainFrame* pMain = (CMainFrame*)AfxGetMainWnd();

	CString strTemp;
	strTemp.Format(L"%d/%d", m_nPos + 1, m_nPicNum);
	pMain->UpdateIndicator(ID_INDICATOR_PICNUM, strTemp);
	pMain->UpdateIndicator(ID_INDICATOR_PICNAME, m_FilesNameAry[m_nPos]);
}

void CFileView::ShowPicture(CDC* pDC, Gdiplus::Image& image, int nShowType)
{
	Gdiplus::Graphics graph(pDC->GetSafeHdc());

	CRect winRect;
	GetClientRect(winRect);

	Gdiplus::REAL x = 0.0;
	Gdiplus::REAL y = 0.0;
	CSize showPicSize = GetShowPicSize(image, m_nShowType);
	Gdiplus::REAL width = (Gdiplus::REAL)showPicSize.cx;
	Gdiplus::REAL height = (Gdiplus::REAL)showPicSize.cy;

	// 使图像居中
	if ((UINT)winRect.Width() > width)
		x = (winRect.Width() - width) / 2.0f;

	if ((UINT)winRect.Height() > height)
		y = ((UINT)winRect.Height() - height) / 2.0f;

	if (m_bWidhtOut)
		x += m_nXX;

	if (m_bHeightOut)
		y += m_nYY;

	m_nShowPicHeight = (int)height;
	m_nShowPicWidth = (int)width;

	graph.DrawImage(&image, x, y, width, height);
}

void CFileView::OnChangeVisualStyle()
{
	m_wndToolBar.CleanUpLockedImages();
	m_wndToolBar.LoadBitmap(theApp.m_bHiColorIcons ? IDB_EXPLORER_24 : IDR_EXPLORER, 0, 0, TRUE /* 锁定*/);

	m_FileViewImages.DeleteImageList();

	UINT uiBmpId = theApp.m_bHiColorIcons ? IDB_FILE_VIEW_24 : IDB_FILE_VIEW;

	CBitmap bmp;
	if (!bmp.LoadBitmap(uiBmpId))
	{
		TRACE(_T("无法加载位图: %x\n"), uiBmpId);
		ASSERT(FALSE);
		return;
	}

	BITMAP bmpObj;
	bmp.GetBitmap(&bmpObj);

	UINT nFlags = ILC_MASK;

	nFlags |= (theApp.m_bHiColorIcons) ? ILC_COLOR24 : ILC_COLOR4;

	m_FileViewImages.Create(16, bmpObj.bmHeight, nFlags, 0, 0);
	m_FileViewImages.Add(&bmp, RGB(255, 0, 255));

	m_wndFileView.SetImageList(&m_FileViewImages, TVSIL_NORMAL);
}
