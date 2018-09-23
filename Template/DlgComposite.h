#pragma once



#include <set>
#include <string>
#include <algorithm>
#include "afxwin.h"

using namespace std;


#define MAX_BMP_CNT		5

enum
{
	_Rotate_0 = 0,
	_Rotate_90,
	_Rotate_180,
	_Rotate_270,
};


typedef enum _tagEMMouseStatus
{
	_Mouse_UP,
	_Mouse_PIC_New = 0x10,
	_Mouse_PIC_RectLeft,
	_Mouse_PIC_RectRight,
	_Mouse_PIC_RectUp,
	_Mouse_PIC_RectDown,
	_Mouse_PIC_RectLeftUp,
	_Mouse_PIC_RectRightUp,
	_Mouse_PIC_RectLeftDown,
	_Mouse_PIC_RectRightDown,
	_Mouse_PIC_RectMove,
	_Mouse_PIC_RectPoint,
	_Mouse_PIC_RectLine,
}EMMouseStatus;


typedef enum _tagEMPointOnRectType
{
	_Point_On_Rect_NotIn = 0,
	_Point_On_Rect_SIZEALL,			//Four-pointed arrow pointing north, south, east, and west
	_Point_On_Rect_SIZENESW_NE,		//Double-pointed arrow pointing northeast and southwest
	_Point_On_Rect_SIZENESW_SW,		//Double-pointed arrow pointing northeast and southwest
	_Point_On_Rect_SIZENS_N,		//Double-pointed arrow pointing north and south
	_Point_On_Rect_SIZENS_S,		//Double-pointed arrow pointing north and south
	_Point_On_Rect_SIZENWSE_NW,		//Double-pointed arrow pointing northwest and southeast
	_Point_On_Rect_SIZENWSE_SE,		//Double-pointed arrow pointing northwest and southeast
	_Point_On_Rect_SIZEWE_W,		//Double-pointed arrow pointing west and east
	_Point_On_Rect_SIZEWE_E,		//Double-pointed arrow pointing west and east
	_Point_On_Rect_POINT_HAND,		//Double-pointed arrow pointing west and east
	_Point_On_Rect_LINE_HAND,		//Double-pointed arrow pointing west and east

	_Point_On_Rect_Reserved,
}EMPointOnRectType;
extern const LPCTSTR c_pCursorType[_Point_On_Rect_Reserved];
extern const EMMouseStatus c_emMouseStatus[_Point_On_Rect_Reserved];


typedef struct _tagStFRelativeRect
{
	float f32Left;
	float f32Right;
	float f32Top;
	float f32Bottom;
}StFRelativeRect;

typedef struct _tagStFileInfo
{
	wstring csName;
	uint64_t u64Time;
}StFileInfo;

typedef set<StFileInfo> CSetFileInfo;
typedef set<StFileInfo>::iterator CSetFileInfoIter;

// CDlgComposite 对话框

class CDlgComposite : public CDialogEx
{
	DECLARE_DYNAMIC(CDlgComposite)

public:
	CDlgComposite(CWnd* pParent = NULL);   // 标准构造函数
	virtual ~CDlgComposite();

// 对话框数据
#ifdef AFX_DESIGN_TIME
	enum { IDD = IDD_DLG_Composite };
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
	afx_msg void OnCbnSelchangeComboCount();
	afx_msg void OnBnClickedBtnPrev();
	afx_msg void OnBnClickedBtnNext();


public:
	CString m_csScanFloder;
	CSetFileInfo m_csSetFile;

	CSetFileInfoIter m_IterCur;

	UINT32	m_u32CompsiteCount;
	UINT32	m_u32BmpWidth;
	UINT32	m_u32BmpHeight;
	UINT32	*m_pBmpBuf32BitMaxCount;
	UINT32	m_u32CompsiteBmpWidth;
	UINT32	m_u32CompsiteBmpHeight;

	UINT32	m_u32CompsiteBmpWidthMaxCount;
	UINT32	m_u32CompsiteBmpHeightMaxCount;

	UINT32	m_u32Rotate;
	EMMouseStatus m_emMouseStatus;

public:
	CStatic m_csStaticPIC;
	CDC *m_pCompsiteDC;
	CBitmap *m_pCompsiteBMPForDC;
	CBitmap *m_pOldBMP;

	CRect m_csDrawRectInPIC;

	CRect m_csRegionRectInPIC;
	CPoint m_csPrevDownPoint;

	StFRelativeRect m_stFRelativeRect;

public:
	int32_t GetFolderSizeCBInner(const wchar_t *pName, WIN32_FIND_DATA *pInfo);
	INT32 ReloadBMP(void);
	INT32 ReDrawStaticPic(void);


	void ReBuildCtrls(void);
	afx_msg void OnTimer(UINT_PTR nIDEvent);
	afx_msg void OnLButtonDown(UINT nFlags, CPoint point);
	afx_msg void OnLButtonUp(UINT nFlags, CPoint point);
	afx_msg void OnMouseMove(UINT nFlags, CPoint point);
};
