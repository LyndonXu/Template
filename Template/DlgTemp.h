#pragma once
#include "afxwin.h"
#include "PolygonCtrl.h"

typedef list<CPolygonCtrl> CListPolygonCtrl;
typedef list<CPolygonCtrl>::iterator CListPolygonCtrlIter;

// CDlgTemp 对话框

class CDlgTemp : public CDialogEx
{
	DECLARE_DYNAMIC(CDlgTemp)

public:
	CDlgTemp(CWnd* pParent = NULL);   // 标准构造函数
	virtual ~CDlgTemp();

// 对话框数据
#ifdef AFX_DESIGN_TIME
	enum { IDD = IDD_DLG_Temp };
#endif

protected:
	virtual void DoDataExchange(CDataExchange* pDX);    // DDX/DDV 支持
	virtual BOOL OnInitDialog();
	virtual BOOL DestroyWindow();

	DECLARE_MESSAGE_MAP()
public:
	afx_msg void OnPaint();
	afx_msg void OnSize(UINT nType, int cx, int cy);
	afx_msg void OnGetMinMaxInfo(MINMAXINFO* lpMMI);
	afx_msg void OnTimer(UINT_PTR nIDEvent);
	afx_msg void OnLButtonDown(UINT nFlags, CPoint point);
	afx_msg void OnLButtonUp(UINT nFlags, CPoint point);
	afx_msg void OnMouseMove(UINT nFlags, CPoint point);
	afx_msg void OnRButtonUp(UINT nFlags, CPoint point);


	void ReBuildCtrls(void);

public:
	CString m_csScanFloder;
	CString m_csLoadFile;

	UINT32	m_u32BmpWidth;
	UINT32	m_u32BmpHeight;
	UINT32	*m_pBmpBuf32Bit;

	EMMouseStatus m_emMouseStatus;
	INT32	m_s32OperatoinIndex;


public:
	CStatic m_csStaticPIC;
	CDC		*m_pBMPDC;
	CBitmap *m_pBMPForBMPDC;
	CBitmap *m_pOldBMP;
	CRect m_csDrawRectInPIC;
	CPoint m_csPrevDownPoint;
	CPoint m_csPrevDownMovePoint;


	//CPolygonCtrl m_csPolygonCtrl;
	CListPolygonCtrl m_csListPolygonCtrl;
	CPolygonCtrl *m_pPolygonCtrl;


public:
	INT32 ReloadBMP(void);
	INT32 ReBuildBMPDC(void);
	INT32 ReDrawStaticPic(void);

};


