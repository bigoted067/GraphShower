
#pragma once

#include "ViewTree.h"

class CFileViewToolBar : public CMFCToolBar
{
	virtual void OnUpdateCmdUI(CFrameWnd* /*pTarget*/, BOOL bDisableIfNoHndler)
	{
		CMFCToolBar::OnUpdateCmdUI((CFrameWnd*) GetOwner(), bDisableIfNoHndler);
	}

	virtual BOOL AllowShowOnList() const { return FALSE; }
};

class CFileView : public CDockablePane
{
// 构造
public:
	CFileView() noexcept;

	void AdjustLayout();
	void OnChangeVisualStyle();

// 特性
protected:

	CViewTree m_wndFileView;
	CImageList m_FileViewImages;
	CFileViewToolBar m_wndToolBar;

protected:
	void FillFileView();

// 实现
public:
	virtual ~CFileView();

	// 图像显示类型枚举
	enum ShowType
	{
		ST_NORMAL = 0,	// 原始大小
		ST_FIXWIDTH = 1,	// 适合宽度
		ST_FIXHEIGHT = 2,	// 适合高度
		ST_FIXSCREEN = 3		// 适合屏幕
	};
protected:
	afx_msg int OnCreate(LPCREATESTRUCT lpCreateStruct);
	afx_msg void OnSize(UINT nType, int cx, int cy);
	afx_msg void OnContextMenu(CWnd* pWnd, CPoint point);
	afx_msg void OnProperties(); 
	afx_msg void OnFileOpenWith();
	afx_msg void OnDummyCompile();
	afx_msg void OnEditCut();
	afx_msg void OnEditCopy();
	afx_msg void OnEditClear();
	afx_msg void OnPaint();
	afx_msg void OnSetFocus(CWnd* pOldWnd);
	afx_msg void OnFileOpen();

	DECLARE_MESSAGE_MAP()

	//添加
public:
	void OpenFile(const CString& strPath);//打开图像文件
	void PreDrawImage(void);			// 在内存中预先绘制图像
	CString GetFilePath(int nIndex);//获得指定索引文件的全路径
	void FixOffset();//修正图像的偏移坐标等信息
	CSize GetShowPicSize(Gdiplus::Image& image, int nShowType);// 获得指定模式下图像的大小
	void RefreshIndicator();			// 更新状态栏的指示器信息
	void ShowPicture(CDC* pDC, Gdiplus::Image& image, int nShowType);

public:
	bool	m_bHeightOut;				// 显示图像是否超出视图高度
	bool	m_bWidhtOut;				// 显示图像是否超出视图宽度
	int		m_nShowType;				// 显示类型

	int				m_nPos;				// 当前文件在容器中的索引
	Gdiplus::Bitmap* m_pBitmap;			// 双缓存绘图
private:
	CStringArray	m_FilesNameAry;		// 保存当前目录下所有文件名的容器
	CString			m_strPath;			// 当前打开目录
	int				m_nPicNum;			// 当前目录的文件个数
	int				m_nShowPicHeight;	// 显示图片的高度
	int				m_nShowPicWidth;	// 显示图片的宽度

	int				m_nXX;				// 图片X坐标的偏移量
	int				m_nYY;				// 图片Y坐标的偏移量
	int				m_nXXMax;			// X坐标最大偏移量
	int				m_nYYMax;			// Y坐标最大偏移量
public:
	
	
};

