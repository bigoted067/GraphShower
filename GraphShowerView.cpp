
// GraphShowerView.cpp: CGraphShowerView 类的实现
//

#include "pch.h"
#include "framework.h"
#include "mainfrm.h"
// SHARED_HANDLERS 可以在实现预览、缩略图和搜索筛选器句柄的
// ATL 项目中进行定义，并允许与该项目共享文档代码。
#ifndef SHARED_HANDLERS
#include "GraphShower.h"
#endif

#include "GraphShowerDoc.h"
#include "GraphShowerView.h"

#ifdef _DEBUG
#define new DEBUG_NEW
#endif


	// 图像显示类型枚举
enum ShowType
{
	ST_NORMAL = 0,	// 原始大小
	ST_FIXWIDTH = 1,	// 适合宽度
	ST_FIXHEIGHT = 2,	// 适合高度
	ST_FIXSCREEN = 3		// 适合屏幕
};

// CGraphShowerView

IMPLEMENT_DYNCREATE(CGraphShowerView, CView)

BEGIN_MESSAGE_MAP(CGraphShowerView, CView)
	// 标准打印命令
	ON_COMMAND(ID_FILE_PRINT, &CView::OnFilePrint)
	ON_COMMAND(ID_FILE_PRINT_DIRECT, &CView::OnFilePrint)
	ON_COMMAND(ID_FILE_PRINT_PREVIEW, &CGraphShowerView::OnFilePrintPreview)
	ON_WM_CONTEXTMENU()
	ON_WM_RBUTTONUP()
	ON_COMMAND(ID_FILE_OPEN, &CGraphShowerView::OnFileOpen)
	ON_COMMAND(ID_MODE_NORMAL, &CGraphShowerView::OnModeNormal)
	ON_COMMAND(ID_MODE_WIDTH, &CGraphShowerView::OnModeWidth)
	ON_COMMAND(ID_MODE_HEIGHT, &CGraphShowerView::OnModeHeight)
	ON_COMMAND(ID_MODE_SCREEN, &CGraphShowerView::OnModeScreen)
	ON_UPDATE_COMMAND_UI(ID_MODE_NORMAL, &CGraphShowerView::OnUpdateModeNormal)
	ON_UPDATE_COMMAND_UI(ID_MODE_WIDTH, &CGraphShowerView::OnUpdateModeWidth)
	ON_UPDATE_COMMAND_UI(ID_MODE_HEIGHT, &CGraphShowerView::OnUpdateModeHeight)
	ON_UPDATE_COMMAND_UI(ID_MODE_SCREEN, &CGraphShowerView::OnUpdateModeScreen)
END_MESSAGE_MAP()

// CGraphShowerView 构造/析构

CGraphShowerView::CGraphShowerView() noexcept
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

CGraphShowerView::~CGraphShowerView()
{
	::delete m_pBitmap;
}

BOOL CGraphShowerView::PreCreateWindow(CREATESTRUCT& cs)
{
	// TODO: 在此处通过修改
	//  CREATESTRUCT cs 来修改窗口类或样式

	return CView::PreCreateWindow(cs);
}

/******************************************************************************
*	作用:		视图重绘函数
******************************************************************************/
void CGraphShowerView::OnDraw(CDC* pDC)
{
	CGraphShowerDoc* pDoc = GetDocument();
	ASSERT_VALID(pDoc);
	if (!pDoc)
		return;

	CRect rect;
	GetClientRect(rect);

	CDC memDC;
	CBitmap MemBitmap;

	// 设备描述表初始化
	memDC.CreateCompatibleDC(NULL);

	// 建立与屏幕显示兼容的内存显示设备
	MemBitmap.CreateCompatibleBitmap(pDC, rect.Width(), rect.Height());

	// 选取空白位图
	memDC.SelectObject(MemBitmap);
	memDC.FillSolidRect(0, 0, rect.Width(), rect.Height(), RGB(255, 255, 255));

	if (m_nPos >= 0)
		ShowPicture(&memDC, *m_pBitmap, m_nShowType);


	pDC->BitBlt(0, 0, rect.Width(), rect.Height(), &memDC, 0, 0, SRCCOPY);

	MemBitmap.DeleteObject();
	memDC.DeleteDC();
}


// CGraphShowerView 打印


void CGraphShowerView::OnFilePrintPreview()
{
#ifndef SHARED_HANDLERS
	AFXPrintPreview(this);
#endif
}

BOOL CGraphShowerView::OnPreparePrinting(CPrintInfo* pInfo)
{
	// 默认准备
	return DoPreparePrinting(pInfo);
}

void CGraphShowerView::OnBeginPrinting(CDC* /*pDC*/, CPrintInfo* /*pInfo*/)
{
	// TODO: 添加额外的打印前进行的初始化过程
}

void CGraphShowerView::OnEndPrinting(CDC* /*pDC*/, CPrintInfo* /*pInfo*/)
{
	// TODO: 添加打印后进行的清理过程
}

void CGraphShowerView::OnRButtonUp(UINT /* nFlags */, CPoint point)
{
	ClientToScreen(&point);
	OnContextMenu(this, point);
}

void CGraphShowerView::OnContextMenu(CWnd* /* pWnd */, CPoint point)
{
#ifndef SHARED_HANDLERS
	theApp.GetContextMenuManager()->ShowPopupMenu(IDR_POPUP_EDIT, point.x, point.y, this, TRUE);
#endif
}


// CGraphShowerView 诊断

#ifdef _DEBUG
void CGraphShowerView::AssertValid() const
{
	CView::AssertValid();
}

void CGraphShowerView::Dump(CDumpContext& dc) const
{
	CView::Dump(dc);
}

CGraphShowerDoc* CGraphShowerView::GetDocument() const // 非调试版本是内联的
{
	ASSERT(m_pDocument->IsKindOf(RUNTIME_CLASS(CGraphShowerDoc)));
	return (CGraphShowerDoc*)m_pDocument;
}
#endif //_DEBUG


// CGraphShowerView 消息处理程序


void CGraphShowerView::OnFileOpen()
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

void CGraphShowerView::OpenFile(const CString& strPath)
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

void CGraphShowerView::PreDrawImage(void)
{
	Gdiplus::Image image(GetFilePath(m_nPos));

	::delete m_pBitmap;

	m_pBitmap = ::new Gdiplus::Bitmap(image.GetWidth(), image.GetHeight(), PixelFormat32bppARGB);
	Gdiplus::Graphics* graph = Gdiplus::Graphics::FromImage(m_pBitmap);

	graph->DrawImage(&image, 0, 0, image.GetWidth(), image.GetHeight());
	SAFE_DELETE(graph);
}

CString CGraphShowerView::GetFilePath(int nIndex)
{
	ASSERT(nIndex >= 0 && nIndex < m_nPicNum);
	return m_strPath + L"\\" + m_FilesNameAry[nIndex];
}

//修正图像的偏移坐标等信息
void CGraphShowerView::FixOffset()
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

void CGraphShowerView::ClearOffset(void)
{
	m_nXX = m_nYY = m_nXXMax = m_nYYMax = 0;
}

CSize CGraphShowerView::GetShowPicSize(Gdiplus::Image& image, int nShowType)
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

void CGraphShowerView::RefreshIndicator()
{
	CMainFrame* pMain = (CMainFrame*)AfxGetMainWnd();

	CString strTemp;
	strTemp.Format(L"%d/%d", m_nPos + 1, m_nPicNum);
	pMain->UpdateIndicator(ID_INDICATOR_PICNUM, strTemp);
	pMain->UpdateIndicator(ID_INDICATOR_PICNAME, m_FilesNameAry[m_nPos]);
}

void CGraphShowerView::ShowPicture(CDC* pDC, Gdiplus::Image& image, int nShowType)
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

//原始大小显示图片
void CGraphShowerView::OnModeNormal()
{
	if (m_nShowType != ST_NORMAL)
	{
		m_nShowType = ST_NORMAL;
		ClearOffset();
		FixOffset();
		Invalidate();
	}
}

//适合宽度显示图片
void CGraphShowerView::OnModeWidth()
{
	if (m_nShowType != ST_FIXWIDTH)
	{
		m_nShowType = ST_FIXWIDTH;
		ClearOffset();
		FixOffset();
		Invalidate();
	}
}

//适合高度显示图片
void CGraphShowerView::OnModeHeight()
{
	if (m_nShowType != ST_FIXHEIGHT)
	{
		m_nShowType = ST_FIXHEIGHT;
		ClearOffset();
		FixOffset();
		Invalidate();
	}
}

//适合屏幕显示图片
void CGraphShowerView::OnModeScreen()
{
	if (m_nShowType != ST_FIXSCREEN)
	{
		m_nShowType = ST_FIXSCREEN;
		ClearOffset();
		FixOffset();
		Invalidate();
	}
}

//原始大小
void CGraphShowerView::OnUpdateModeNormal(CCmdUI* pCmdUI)
{
	pCmdUI->SetCheck(m_nShowType == ST_NORMAL);
}

//适合宽度
void CGraphShowerView::OnUpdateModeWidth(CCmdUI* pCmdUI)
{
	pCmdUI->SetCheck(m_nShowType == ST_FIXWIDTH);
}

//适合高度
void CGraphShowerView::OnUpdateModeHeight(CCmdUI* pCmdUI)
{
	pCmdUI->SetCheck(m_nShowType == ST_FIXHEIGHT);
}

//适合屏幕
void CGraphShowerView::OnUpdateModeScreen(CCmdUI* pCmdUI)
{
	pCmdUI->SetCheck(m_nShowType == ST_FIXSCREEN);
}
