
// GraphShowerView.h: CGraphShowerView 类的接口
//

#pragma once


class CGraphShowerView : public CView
{
protected: // 仅从序列化创建
	CGraphShowerView() noexcept;
	DECLARE_DYNCREATE(CGraphShowerView)

// 特性
public:
	CGraphShowerDoc* GetDocument() const;

// 操作
public:

// 重写
public:
	virtual void OnDraw(CDC* pDC);  // 重写以绘制该视图
	virtual BOOL PreCreateWindow(CREATESTRUCT& cs);
protected:
	virtual BOOL OnPreparePrinting(CPrintInfo* pInfo);
	virtual void OnBeginPrinting(CDC* pDC, CPrintInfo* pInfo);
	virtual void OnEndPrinting(CDC* pDC, CPrintInfo* pInfo);

// 实现
public:
	virtual ~CGraphShowerView();
#ifdef _DEBUG
	virtual void AssertValid() const;
	virtual void Dump(CDumpContext& dc) const;
#endif

protected:
// 生成的消息映射函数
protected:
	afx_msg void OnFilePrintPreview();
	afx_msg void OnRButtonUp(UINT nFlags, CPoint point);
	afx_msg void OnContextMenu(CWnd* pWnd, CPoint point);
	DECLARE_MESSAGE_MAP()
public:
	afx_msg void OnFileOpen();

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
};

#ifndef _DEBUG  // GraphShowerView.cpp 中的调试版本
inline CGraphShowerDoc* CGraphShowerView::GetDocument() const
   { return reinterpret_cast<CGraphShowerDoc*>(m_pDocument); }
#endif

