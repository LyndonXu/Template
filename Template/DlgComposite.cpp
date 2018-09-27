// DlgComposite.cpp : 实现文件
//

#include "stdafx.h"
#include "Template.h"
#include "DlgComposite.h"
#include "afxdialogex.h"
#include "utils.h"

#include "DlgTemp.h"

#ifdef _DEBUG
#define new DEBUG_NEW
#endif


const LPCTSTR c_pCursorType[_Point_On_Rect_Reserved] =
{
	IDC_ARROW,
	IDC_SIZEALL,
	IDC_SIZENESW,
	IDC_SIZENESW,
	IDC_SIZENS,
	IDC_SIZENS,
	IDC_SIZENWSE,
	IDC_SIZENWSE,
	IDC_SIZEWE,
	IDC_SIZEWE,
	IDC_HAND,
	IDC_HAND,
};
const EMMouseStatus c_emMouseStatus[_Point_On_Rect_Reserved] =
{
	_Mouse_PIC_New,
	_Mouse_PIC_RectMove,
	_Mouse_PIC_RectRightUp,
	_Mouse_PIC_RectLeftDown,
	_Mouse_PIC_RectUp,
	_Mouse_PIC_RectDown,
	_Mouse_PIC_RectLeftUp,
	_Mouse_PIC_RectRightDown,
	_Mouse_PIC_RectLeft,
	_Mouse_PIC_RectRight,
	_Mouse_PIC_RectPoint,
	_Mouse_PIC_RectLine,
};

// CDlgComposite 对话框

IMPLEMENT_DYNAMIC(CDlgComposite, CDialogEx)

CDlgComposite::CDlgComposite(CWnd* pParent /*=NULL*/)
	: CDialogEx(IDD_DLG_Composite, pParent)
	, m_csScanFloder(L"")
	, m_csSaveFile(L"")
	, m_u32CompsiteCount(3)
	, m_u32BmpWidth(0)
	, m_u32BmpHeight(0)
	, m_pBmpBuf32BitMaxCount(NULL)
	, m_u32CompsiteBmpWidth(0)
	, m_u32CompsiteBmpHeight(0)
	, m_u32CompsiteBmpWidthMaxCount(0)
	, m_u32CompsiteBmpHeightMaxCount(0)
	, m_u32Rotate(_Rotate_270)

	, m_pCompsiteDC(NULL)
	, m_pOldBMP(NULL)
	, m_pCompsiteBMPForDC(NULL)
	, m_emMouseStatus(_Mouse_UP)


	, m_pDlgTemp(NULL)

{

}

CDlgComposite::~CDlgComposite()
{
}

void CDlgComposite::DoDataExchange(CDataExchange* pDX)
{
	CDialogEx::DoDataExchange(pDX);
	DDX_Control(pDX, IDC_STATIC_PICShow, m_csStaticPIC);
}


BEGIN_MESSAGE_MAP(CDlgComposite, CDialogEx)
	ON_BN_CLICKED(IDC_BTN_Prev, &CDlgComposite::OnBnClickedBtnPrev)
	ON_WM_PAINT()
	ON_WM_SIZE()
	ON_WM_GETMINMAXINFO()
	ON_CBN_SELCHANGE(IDC_COMBO_Count, &CDlgComposite::OnCbnSelchangeComboCount)
	ON_BN_CLICKED(IDC_BTN_Next, &CDlgComposite::OnBnClickedBtnNext)
	ON_WM_TIMER()
	ON_WM_LBUTTONDOWN()
	ON_WM_LBUTTONUP()
	ON_WM_MOUSEMOVE()
	ON_WM_LBUTTONDBLCLK()
	ON_BN_CLICKED(IDC_BTN_Preview, &CDlgComposite::OnBnClickedBtnPreview)
END_MESSAGE_MAP()


int32_t GetFolderSizeCB(const wchar_t *pName, WIN32_FIND_DATA *pInfo, void *pContext)
{
	if (pContext == NULL)
	{
		return -1;
	}
	CDlgComposite *pThis = (CDlgComposite *)pContext;

	return pThis->GetFolderSizeCBInner(pName, pInfo);
}

namespace std
{
	bool operator < (const StFileInfo &left, const StFileInfo &right)
	{
		return left.u64Time < right.u64Time;
	}
}

int32_t CDlgComposite::GetFolderSizeCBInner(const wchar_t *pName,
	WIN32_FIND_DATA *pInfo)
{
	if (wcsstr(pInfo->cFileName, L".bmp") != NULL)
	{
		SYSTEMTIME stTime;
		swscanf_s(pInfo->cFileName, L"%hd_%hd_%hd_%hd_%hd_%hd_%hd.bmp",
			&(stTime.wYear),
			&(stTime.wMonth),
			&(stTime.wDay),
			&(stTime.wHour),
			&(stTime.wMinute),
			&(stTime.wSecond),
			&(stTime.wMilliseconds));
		CTime csTime(stTime);

		uint64_t u64Time = csTime.GetTime();
		u64Time = u64Time * 1000 + stTime.wMilliseconds;

		StFileInfo csInfo = { pInfo->cFileName,  u64Time };

		m_csSetFile.insert(csInfo);
	}

	return 0;
}


BOOL CDlgComposite::OnInitDialog()
{
	CDialogEx::OnInitDialog();

	// TODO:  在此添加额外的初始化
	{
		m_pDlgTemp = new CDlgTemp;
		if (m_pDlgTemp != NULL)
		{
			((CDlgTemp *)m_pDlgTemp)->m_s32SetUpMode = 1;
			((CDlgTemp *)m_pDlgTemp)->m_csSaveFile = m_csSaveFile;
			m_pDlgTemp->Create(IDD_DLG_Temp, GetDesktopWindow());
			m_pDlgTemp->ShowWindow(SW_SHOW);
		}
	}


	ReBuildCtrls();

	if (m_csScanFloder.GetLength() == 0)
	{
		MessageBox(L"请设置工作目录");
		EndDialog(IDCANCEL);
		return TRUE;
	}
	else
	{
		GetFolderSize(m_csScanFloder.GetString(), GetFolderSizeCB, this);

		if (m_csSetFile.size() < m_u32CompsiteCount)
		{
			MessageBox(L"工作目录BMP数量过少");
			EndDialog(IDCANCEL);
			return TRUE;
		}
		m_IterCur = m_csSetFile.begin();
	}

	{
		ReloadBMP();
	}

	for (INT32 i = 0; i < 3; i++)
	{
		CString csStr;
		csStr.Format(L"%d张", i + 3);
		((CComboBox *)GetDlgItem(IDC_COMBO_Count))->InsertString(i, csStr);
	}
	((CComboBox *)GetDlgItem(IDC_COMBO_Count))->SetCurSel(0);

	GetDlgItem(IDC_BTN_Prev)->EnableWindow(FALSE);


	SetTimer(1, 40, NULL);

	return TRUE;  // return TRUE unless you set the focus to a control
				  // 异常: OCX 属性页应返回 FALSE
}


BOOL CDlgComposite::DestroyWindow()
{
	// TODO: 在此添加专用代码和/或调用基类

	if (m_pDlgTemp != NULL)
	{
		delete m_pDlgTemp;
		m_pDlgTemp = NULL;
	}

	if (m_pBmpBuf32BitMaxCount != NULL)
	{
		free(m_pBmpBuf32BitMaxCount);
		m_pBmpBuf32BitMaxCount = NULL;
	}

	if (m_pCompsiteDC != NULL)
	{
		m_pCompsiteDC->SelectObject(m_pOldBMP);
		delete m_pCompsiteDC;
		m_pCompsiteDC = NULL;
	}

	if (m_pCompsiteBMPForDC != NULL)
	{
		delete m_pCompsiteBMPForDC;
		m_pCompsiteBMPForDC = NULL;
	}



	return CDialogEx::DestroyWindow();
}

BOOL CDlgComposite::PreTranslateMessage(MSG* pMsg)
{
	// TODO: 在此添加专用代码和/或调用基类

	if (pMsg->message == WM_KEYDOWN && pMsg->wParam == VK_ESCAPE)
		return TRUE;
	if (pMsg->message == WM_KEYDOWN && pMsg->wParam == VK_RETURN)
		return TRUE;

	return CDialogEx::PreTranslateMessage(pMsg);
}


// CDlgComposite 消息处理程序


void CDlgComposite::OnPaint()
{
	CPaintDC dc(this); // device context for painting
					   // TODO: 在此处添加消息处理程序代码
					   // 不为绘图消息调用 CDialogEx::OnPaint()
}

void CDlgComposite::ReBuildCtrls(void)
{
	{
		CWnd *pPICShow = GetDlgItem(IDC_STATIC_PICShow);
		if (pPICShow != NULL && pPICShow->GetSafeHwnd() != NULL)
		{
			CRect csClient;
			GetWindowRect(&csClient);
			csClient.left += 8;
			csClient.top += 31;
			csClient.right -= 8;
			csClient.bottom -= 45;

			ScreenToClient(&csClient);

			pPICShow->MoveWindow(&csClient);
			pPICShow->Invalidate();
		}
	}

	{
#define ID_COUNT	4
		const UINT32 u32ID[ID_COUNT] = 
		{ 
			IDC_COMBO_Count, IDC_BTN_Prev, 
			IDC_BTN_Next, IDC_BTN_Preview,
		};
		CRect csClient;
		GetWindowRect(&csClient);
		for (UINT32 i = 0; i < ID_COUNT; i++)
		{
			CWnd *pCtrl = GetDlgItem(u32ID[i]);

			if (pCtrl == NULL || pCtrl->GetSafeHwnd() == NULL)
			{
				break;
			}
			CRect csRect;
			pCtrl->GetWindowRect(csRect);

			csRect.MoveToXY(csClient.left + 16 + i * (csClient.Width() - csRect.Width() - 32) / (ID_COUNT - 1),
				csClient.bottom - 35);

			ScreenToClient(&csRect);

			pCtrl->MoveWindow(csRect);
		}
#undef ID_COUNT
	}

}

void CDlgComposite::OnSize(UINT nType, int cx, int cy)
{
	CDialogEx::OnSize(nType, cx, cy);

	// TODO: 在此处添加消息处理程序代码
	ReBuildCtrls();
}


void CDlgComposite::OnGetMinMaxInfo(MINMAXINFO* lpMMI)
{
	// TODO: 在此添加消息处理程序代码和/或调用默认值
	lpMMI->ptMinTrackSize.x = 480;
	lpMMI->ptMinTrackSize.y = 320;

	CDialogEx::OnGetMinMaxInfo(lpMMI);
}


void CDlgComposite::OnCbnSelchangeComboCount()
{
	// TODO: 在此添加控件通知处理程序代码
	int sel = ((CComboBox *)GetDlgItem(IDC_COMBO_Count))->GetCurSel();
	if (sel != m_u32CompsiteCount)
	{
		m_u32CompsiteCount = sel + 3;
		ReloadBMP();
	}
}


void CDlgComposite::OnBnClickedBtnPrev()
{
	// TODO: 在此添加控件通知处理程序代码

	if (m_IterCur != m_csSetFile.begin())
	{
		m_IterCur--;
		ReloadBMP();
		if (m_IterCur == m_csSetFile.begin())
		{
			GetDlgItem(IDC_BTN_Prev)->EnableWindow(FALSE);
		}

		GetDlgItem(IDC_BTN_Next)->EnableWindow(TRUE);
	}
}
void CDlgComposite::OnBnClickedBtnNext()
{
	// TODO: 在此添加控件通知处理程序代码
	CSetFileInfoIter iter = m_IterCur;
	if (iter == m_csSetFile.end())
	{
		return;
	}
	iter++;
	for (UINT32 i = 0; i < m_u32CompsiteCount; i++, iter++)
	{
		if (iter == m_csSetFile.end())
		{
			return;
		}
	}

	if (iter == m_csSetFile.end())
	{
		GetDlgItem(IDC_BTN_Next)->EnableWindow(FALSE);
	}

	GetDlgItem(IDC_BTN_Prev)->EnableWindow(TRUE);
	m_IterCur++;
	ReloadBMP();
}

INT32 CDlgComposite::ReloadBMP(void)
{
	CSetFileInfoIter iter = m_IterCur;
	if (iter == m_csSetFile.end())
	{
		return -1;
	}

	INT32 s32Width = 0;
	INT32 s32Height = 0;

	iter = m_IterCur;
	for (UINT32 i = 0; i < m_u32CompsiteCount; i++, iter++)
	{
		if (iter == m_csSetFile.end())
		{
			return -1;
		}
		FILE *pFile = NULL;
		CString csStr;
		csStr = m_csScanFloder + L"\\";
		csStr += iter->csName.c_str();
		int ret = _wfopen_s(&pFile, csStr.GetString(), L"rb");
		if (ret != 0)
		{
			return -1;
		}
		BITMAPFILEHEADER stBmpHeader = { 0 };

		BITMAPINFOHEADER stInfoHeader = { 0 };

		size_t u32Read = fread_s(&stBmpHeader, sizeof(BITMAPFILEHEADER), 1, sizeof(BITMAPFILEHEADER), pFile);
		if (u32Read != sizeof(BITMAPFILEHEADER))
		{
			fclose(pFile);
			return -1;
		}
		u32Read = fread_s(&stInfoHeader, sizeof(BITMAPINFOHEADER), 1, sizeof(BITMAPINFOHEADER), pFile);
		if (u32Read != sizeof(BITMAPINFOHEADER))
		{
			fclose(pFile);
			return -1;
		}
		fclose(pFile);

		if (stBmpHeader.bfType != 0x4D42)
		{
			return -1;
		}

		if (stInfoHeader.biBitCount != 24 && stInfoHeader.biBitCount != 32)
		{
			return -1;
		}

		if (s32Width == 0 || s32Height == 0)
		{
			s32Width = stInfoHeader.biWidth;
			s32Height = stInfoHeader.biHeight;
		}
		else
		{
			if (s32Width != stInfoHeader.biWidth || 
				s32Height != stInfoHeader.biHeight)
			{
				return -1;
			}
		}
	}

	if (s32Width < 0)
	{
		s32Width = 0 - s32Width;
	}

	if (s32Height < 0)
	{
		s32Height = 0 - s32Height;
	}
	
	if (m_u32BmpWidth != (UINT32)s32Width || m_u32BmpHeight != (UINT32)s32Height)
	{
		if (m_pBmpBuf32BitMaxCount != NULL)
		{
			free(m_pBmpBuf32BitMaxCount);
			m_pBmpBuf32BitMaxCount = NULL;
		}
	}
	m_u32BmpWidth = (UINT32)s32Width;
	m_u32BmpHeight = (UINT32)s32Height;
	if (m_pBmpBuf32BitMaxCount == NULL)
	{
		m_pBmpBuf32BitMaxCount = 
			(UINT32 *)malloc(sizeof(UINT32) * MAX_BMP_CNT * s32Width * s32Height);
	}
	if (m_pBmpBuf32BitMaxCount == NULL)
	{
		return -1;
	}

	if (m_u32Rotate == _Rotate_90 || m_u32Rotate == _Rotate_270)
	{
		m_u32CompsiteBmpHeight = m_u32BmpWidth;
		m_u32CompsiteBmpWidth = m_u32BmpHeight * m_u32CompsiteCount;

		m_u32CompsiteBmpHeightMaxCount = m_u32BmpWidth;
		m_u32CompsiteBmpWidthMaxCount = m_u32BmpHeight * MAX_BMP_CNT;

	}
	else
	{
		m_u32CompsiteBmpHeight = m_u32BmpHeight;
		m_u32CompsiteBmpWidth = m_u32BmpWidth* m_u32CompsiteCount;

		m_u32CompsiteBmpHeightMaxCount = m_u32BmpHeight;
		m_u32CompsiteBmpWidthMaxCount = m_u32BmpWidth * MAX_BMP_CNT;
	}

	iter = m_IterCur;
	for (UINT32 i = 0; i < m_u32CompsiteCount; i++, iter++)
	{
		FILE *pFile = NULL;
		CString csStr;
		csStr = m_csScanFloder + L"\\";
		csStr += iter->csName.c_str();
		int ret = _wfopen_s(&pFile, csStr.GetString(), L"rb");
		if (ret != 0)
		{
			return -1;
		}

		StRGB32Bit stRGB = { 0 };
		int32_t s32Ret = LoadBmp(pFile, &stRGB);
		fclose(pFile);
		if (s32Ret != 0)
		{
			return s32Ret;
		}
#if 0
		{
			ret = _wfopen_s(&pFile, L"f:\\test.bmp", L"wb+");
			if (ret == 0)
			{
				//fwrite(pBmpBuf, 1, (sizeof(UINT32) * s32Width * s32Height), pFile);
				BufToBmp(pFile, (uint8_t *)pBmpBuf, s32Height, s32Width, 4, false);
				fclose(pFile);
			}
		}
#endif
#if 0
		{
			for (UINT i = 0; i < stRGB.s32Height; i++)
			{
				g_u32Row[i] = *(stRGB.pRGB + i * stRGB.s32Width);
			}
			g_u32Row[0] = g_u32Row[0];
		}

#endif
		UINT32 *pBmpBuf = stRGB.pRGB;
		AutoFree csFreeBmpBuf(pBmpBuf);

		UINT32 *pBmpRotateBuf = NULL;
		AutoFree csFreeBmpRotateBuf;
		if (m_u32Rotate == _Rotate_0)
		{
			pBmpRotateBuf = pBmpBuf;
		}
		else
		{
			pBmpRotateBuf = (UINT32 *)malloc(sizeof(UINT32) * s32Width * s32Height);
			if (pBmpRotateBuf == NULL)
			{
				return -1;
			}
			csFreeBmpRotateBuf.SetMem(pBmpRotateBuf);
		}

		if (m_u32Rotate == _Rotate_90)
		{
			INT32 s32DestWidth = s32Height;
			INT32 s32DestHeight = s32Width;

			for (INT32 j = 0; j < s32DestHeight; j++)	/* Y */
			{
				for (INT32 i = 0; i < s32DestWidth; i++)	/* X */
				{
					pBmpRotateBuf[j * s32DestWidth + i] =
						//pBmpBuf[i * s32Width + s32Width - 1 - j];
						pBmpBuf[(s32Height - 1 - i) * s32Width + j];
				}
			}
#if 0
			{
				ret = _wfopen_s(&pFile, L"f:\\test_90.bmp", L"wb+");
				if (ret == 0)
				{
					//fwrite(pBmpBuf, 1, (sizeof(UINT32) * s32Width * s32Height), pFile);
					BufToBmp(pFile, (uint8_t *)pBmpRotateBuf, s32Width, s32Height, 4, false);
					fclose(pFile);
				}
			}
#endif
		}
		else if (m_u32Rotate == _Rotate_180)
		{
			INT32 s32DestWidth = s32Width;
			INT32 s32DestHeight = s32Height;
			for (INT32 j = 0; j < s32DestHeight; j++)	/* Y */
			{
				for (INT32 i = 0; i < s32DestWidth; i++)	/* X */
				{
					pBmpRotateBuf[j * s32DestWidth + i] =
						pBmpBuf[(s32Height - 1 - j) * s32Width + s32Width - 1 - i];
				}
			}
#if 0
			{
				ret = _wfopen_s(&pFile, L"f:\\test_180.bmp", L"wb+");
				if (ret == 0)
				{
					//fwrite(pBmpBuf, 1, (sizeof(UINT32) * s32Width * s32Height), pFile);
					BufToBmp(pFile, (uint8_t *)pBmpRotateBuf, s32Height, s32Width, 4, false);
					fclose(pFile);
				}
			}
#endif
		}
		else if (m_u32Rotate == _Rotate_270)
		{
			INT32 s32DestWidth = s32Height;
			INT32 s32DestHeight = s32Width;
			for (INT32 j = 0; j < s32DestHeight; j++)	/* Y */
			{
				for (INT32 i = 0; i < s32DestWidth; i++)	/* X */
				{
					pBmpRotateBuf[j * s32DestWidth + i] =
						pBmpBuf[i * s32Width + s32Width - 1 - j];
				}
			}
#if 0
			{
				ret = _wfopen_s(&pFile, L"f:\\test_270.bmp", L"wb+");
				if (ret == 0)
				{
					//fwrite(pBmpBuf, 1, (sizeof(UINT32) * s32Width * s32Height), pFile);
					BufToBmp(pFile, (uint8_t *)pBmpRotateBuf, s32Width, s32Height, 4, false);
					fclose(pFile);
				}
			}
#endif
#if 0
			{
				for (UINT i = 0; i < s32DestHeight; i++)
				{
					g_u32Row[i] = *(pBmpRotateBuf + i * s32DestWidth);
				}
				g_u32Row[0] = g_u32Row[0];
			}

#endif

		}
		

		if (m_u32Rotate == _Rotate_270 || m_u32Rotate == _Rotate_90)
		{
			StRGB32Bit stDest =
			{
				(int32_t)m_u32CompsiteBmpWidthMaxCount,
				(int32_t)m_u32CompsiteBmpHeightMaxCount,
				m_pBmpBuf32BitMaxCount,
			};
			StRGB32Bit stSrc =
			{
				s32Height,
				s32Width,
				pBmpRotateBuf,
			};

			RGBCopy(&stDest, s32Height * i, 0,
				&stSrc, 0, 0,
				stSrc.s32Width, stSrc.s32Height);
		}
		else
		{
			StRGB32Bit stDest =
			{
				(int32_t)m_u32CompsiteBmpWidthMaxCount,
				(int32_t)m_u32CompsiteBmpHeightMaxCount,
				m_pBmpBuf32BitMaxCount,
			};
			StRGB32Bit stSrc =
			{
				s32Width,
				s32Height,
				pBmpRotateBuf,
			};

			RGBCopy(&stDest, s32Width * i, 0,
				&stSrc, 0, 0,
				stSrc.s32Width, stSrc.s32Height);

		}
	}
#if 0
	{
	FILE *pFile = NULL;
	int ret = _wfopen_s(&pFile, L"f:\\test_c.bmp", L"wb+");
	if (ret == 0)
	{
		BufToBmp(pFile, (uint8_t *)m_pBmpBuf32BitMaxCount,
			m_u32CompsiteBmpHeightMaxCount,
			m_u32CompsiteBmpWidthMaxCount,
			4, true);
		fclose(pFile);
	}
	}
#endif

	if (m_pCompsiteDC != NULL && m_pCompsiteBMPForDC != NULL)
	{
		m_pCompsiteDC->SelectObject(m_pOldBMP);
		m_pOldBMP = NULL;
		delete m_pCompsiteDC;
		m_pCompsiteDC = NULL;

		delete m_pCompsiteBMPForDC;
		m_pCompsiteBMPForDC = NULL;
	}

	if (m_pCompsiteDC == NULL)
	{
		m_pCompsiteDC = new CDC;
	}

	if (m_pCompsiteBMPForDC == NULL)
	{
		m_pCompsiteBMPForDC = new CBitmap;
	}

	if (m_pCompsiteDC != NULL && m_pCompsiteBMPForDC != NULL)
	{
		m_pCompsiteDC->CreateCompatibleDC(NULL);

		m_pCompsiteBMPForDC->CreateBitmap(m_u32CompsiteBmpWidthMaxCount,
			m_u32CompsiteBmpHeightMaxCount, 1, 32,
			NULL);
		m_pOldBMP = m_pCompsiteDC->SelectObject(m_pCompsiteBMPForDC);

		BITMAPINFO bmpInfo;
		ZeroMemory(&bmpInfo, sizeof(BITMAPINFO));
		bmpInfo.bmiHeader.biSize = sizeof(BITMAPINFOHEADER);
		bmpInfo.bmiHeader.biWidth = m_u32CompsiteBmpWidthMaxCount;
		bmpInfo.bmiHeader.biHeight = 0 - m_u32CompsiteBmpHeightMaxCount;
		bmpInfo.bmiHeader.biPlanes = 1;
		bmpInfo.bmiHeader.biBitCount = 32;
		bmpInfo.bmiHeader.biCompression = BI_RGB;
		bmpInfo.bmiHeader.biSizeImage = 0;
		bmpInfo.bmiHeader.biXPelsPerMeter = 3000;
		bmpInfo.bmiHeader.biYPelsPerMeter = 3000;
		bmpInfo.bmiHeader.biClrUsed = 0;
		bmpInfo.bmiHeader.biClrImportant = 0;

#if 0
		{
			for (UINT i = 0; i < m_u32CompsiteBmpHeightMaxCount; i++)
			{
				g_u32Row[i] = *(m_pBmpBuf32BitMaxCount + i * m_u32CompsiteBmpWidthMaxCount);
			}
			g_u32Row[0] = g_u32Row[0];
		}

#endif

		::SetDIBits(m_pCompsiteDC->GetSafeHdc(),
			(HBITMAP)(m_pCompsiteBMPForDC->GetSafeHandle()),
			0, m_u32CompsiteBmpHeightMaxCount,
			m_pBmpBuf32BitMaxCount,
			&bmpInfo, DIB_RGB_COLORS);

#if 0
		{
			for (UINT i = 0; i < m_u32CompsiteBmpHeightMaxCount; i++)
			{
				g_u32Row[i] = *(m_pBmpBuf32BitMaxCount + i * m_u32CompsiteBmpWidthMaxCount);
			}
			g_u32Row[0] = g_u32Row[0];
		}

#endif

#if 0
		{
			CImage csImage;
			csImage.Attach((HBITMAP)(m_pCompsiteBMPForDC->GetSafeHandle()));
			csImage.Save(L"f:\\123.bmp");
		}
#endif
		ReDrawStaticPic();

	}
	else if (m_pCompsiteDC != NULL)
	{
		delete m_pCompsiteDC;
		m_pCompsiteDC = NULL;
	}
	else
	{
		delete m_pCompsiteBMPForDC;
		m_pCompsiteBMPForDC = NULL;
	}

	return 0;
}

INT32 CDlgComposite::ReDrawStaticPic(void)
{
	if (m_pBmpBuf32BitMaxCount == NULL || m_pCompsiteDC == NULL)
	{
		m_csStaticPIC.Invalidate();
		return 0;
	}
	do
	{
		CDC *pDC = m_csStaticPIC.GetDC();
		if (pDC == NULL)
		{
			break;
		}
		CRect csRect;
		pDC->GetWindow()->GetWindowRect(&csRect);

		CDC csMemDC;
		csMemDC.CreateCompatibleDC(pDC);
		CBitmap csBitMap;
		csBitMap.CreateCompatibleBitmap(pDC, csRect.Width(), csRect.Height());

		CBitmap *pOldBMP = (CBitmap *)csMemDC.SelectObject(&csBitMap);

		double d64ImageRadio = (double)m_u32CompsiteBmpWidth / m_u32CompsiteBmpHeightMaxCount;
		double d64DrawRadio = (double)csRect.Width() / csRect.Height();
		
		CRect csDest = { 0, 0, csRect.Width(), csRect.Height() };
		if (d64ImageRadio > d64DrawRadio)
		{
			INT32 y = (INT32)(csRect.Width() / d64ImageRadio);
			y = csDest.Height() - y;
			csDest.top = y / 2;
			csDest.bottom = csDest.bottom - y / 2;
		}
		else
		{
			INT x = (INT32)(csRect.Height() * d64ImageRadio);
			x = csDest.Width() - x;
			csDest.left = x / 2;
			csDest.right = csDest.right - x / 2;
		}

		m_csDrawRectInPIC = csDest;

		TransparentBlt(csMemDC.GetSafeHdc(),
			csDest.left, csDest.top, csDest.Width(), csDest.Height(),
			m_pCompsiteDC->GetSafeHdc(),
			0, 0, m_u32CompsiteBmpWidth, m_u32CompsiteBmpHeightMaxCount,
			0);


		CPen csPen(PS_SOLID, 1, RGB(255, 0, 0));
		CPen *pOldPen = (CPen *)csMemDC.SelectObject(&csPen);
		csMemDC.SelectStockObject(NULL_BRUSH);
		csMemDC.SetBkMode(TRANSPARENT);

		if (m_csRegionRectInPIC.Width() != 0 && m_csRegionRectInPIC.Height() != 0)
		{
			if (m_emMouseStatus == _Mouse_UP)
			{
				m_csRegionRectInPIC.left = (LONG)(m_stFRelativeRect.f32Left * m_csDrawRectInPIC.Width()) + 
					m_csDrawRectInPIC.left;
				m_csRegionRectInPIC.right = (LONG)(m_stFRelativeRect.f32Right * m_csDrawRectInPIC.Width()) +
					m_csDrawRectInPIC.left;

				m_csRegionRectInPIC.top = (LONG)(m_stFRelativeRect.f32Top * m_csDrawRectInPIC.Height()) + 
					m_csDrawRectInPIC.top;

				m_csRegionRectInPIC.bottom = (LONG)(m_stFRelativeRect.f32Bottom * m_csDrawRectInPIC.Height()) +
					m_csDrawRectInPIC.top;
			}
			csMemDC.Rectangle(&m_csRegionRectInPIC);
		}


#if 0
		list<StCalibrationInfo>::iterator iter;
		float f32Radio = ((float)csRect.Width()) / (m_s32SourcePICWidth);
		for (iter = m_csPICCalibrationInfoList.begin();
			iter != m_csPICCalibrationInfoList.end(); iter++)
		{
			CRect csRectTmp = iter->csRect;
			//float f32Radio = ((float)csRect.Width()) / ((float)iter->csRectWnd.Width());
			ReComputeTheRect(&csRectTmp, f32Radio);

			csMemDC.Rectangle(&csRectTmp);
			CString csStrTmp;
			csStrTmp.Format("_%d", iter->emTargetPose);
			if (iter->emTargetType < _Target_Reserved)
			{
				csStrTmp = c_pTargetName[iter->emTargetType] + csStrTmp;
			}
			else
			{
				csStrTmp = iter->csTargetType.GetString() + csStrTmp;
			}
			csMemDC.DrawText(csStrTmp.GetString(), &csRectTmp, 0);
		}

		if (m_pPICCalibrationInfoNeedChange == NULL)
		{
			if ((m_stCalibrationInfo.csRect.Width() != 0 ||
				m_stCalibrationInfo.csRect.Height() != 0) &&
				(m_emMouseStatus < _Mouse_MOV_New))
			{
				CString csStrTmp;
				csStrTmp.Format("_%d", m_stCalibrationInfo.emTargetPose);
				csStrTmp = c_pTargetName[m_stCalibrationInfo.emTargetType] + csStrTmp;

				csMemDC.Rectangle(&m_stCalibrationInfo.csRect);
				csMemDC.DrawText(csStrTmp.GetString(),
					&(m_stCalibrationInfo.csRect), 0);//DT_CENTER | DT_VCENTER | DT_SINGLELINE);
			}
		}
#endif
		csMemDC.SelectObject(pOldPen);

		pDC->BitBlt(0, 0, csRect.Width(), csRect.Height(), &csMemDC,
			0, 0, SRCCOPY);

		csMemDC.SelectObject(pOldBMP);

		ReleaseDC(pDC);
	} while (0);

	return 0;
}




void CDlgComposite::OnTimer(UINT_PTR nIDEvent)
{
	// TODO: 在此添加消息处理程序代码和/或调用默认值
	if (nIDEvent == 1)
	{
		ReDrawStaticPic();
	}
	CDialogEx::OnTimer(nIDEvent);
}


EMPointOnRectType PointInRect(POINT stPoint, RECT stRect)
{
	RECT stRectIn = { stRect.left + 3, stRect.top + 3, stRect.right - 3, stRect.bottom - 3 };
	RECT stRectOut = { stRect.left - 3, stRect.top - 3, stRect.right + 3, stRect.bottom + 3 };
	if (stPoint.x < stRectOut.left)
	{
		return _Point_On_Rect_NotIn;
	}
	else if ((stPoint.x >= stRectOut.left) && (stPoint.x <= stRectIn.left))
	{
		if (stPoint.y < stRectOut.top)
		{
			return _Point_On_Rect_NotIn;
		}
		else if ((stPoint.y >= stRectOut.top) && (stPoint.y <= stRectIn.top))
		{
			return _Point_On_Rect_SIZENWSE_NW;
		}
		else if ((stPoint.y > stRectIn.top) && (stPoint.y < stRectIn.bottom))
		{
			return _Point_On_Rect_SIZEWE_W;
		}
		else if ((stPoint.y >= stRectIn.bottom) && (stPoint.y <= stRectOut.bottom))
		{
			return _Point_On_Rect_SIZENESW_SW;
		}
		else
		{
			return _Point_On_Rect_NotIn;
		}
	}
	else if ((stPoint.x > stRectIn.left) && (stPoint.x < stRectIn.right))
	{
		if (stPoint.y < stRectOut.top)
		{
			return _Point_On_Rect_NotIn;
		}
		else if ((stPoint.y >= stRectOut.top) && (stPoint.y <= stRectIn.top))
		{
			return _Point_On_Rect_SIZENS_N;
		}
		else if ((stPoint.y > stRectIn.top) && (stPoint.y < stRectIn.bottom))
		{
			return _Point_On_Rect_SIZEALL;
		}
		else if ((stPoint.y >= stRectIn.bottom) && (stPoint.y <= stRectOut.bottom))
		{
			return _Point_On_Rect_SIZENS_S;
		}
		else
		{
			return _Point_On_Rect_NotIn;
		}
	}
	else if ((stPoint.x >= stRectIn.right) && (stPoint.x <= stRectOut.right))
	{
		if (stPoint.y < stRectOut.top)
		{
			return _Point_On_Rect_NotIn;
		}
		else if ((stPoint.y >= stRectOut.top) && (stPoint.y <= stRectIn.top))
		{
			return _Point_On_Rect_SIZENESW_NE;
		}
		else if ((stPoint.y > stRectIn.top) && (stPoint.y < stRectIn.bottom))
		{
			return _Point_On_Rect_SIZEWE_E;
		}
		else if ((stPoint.y >= stRectIn.bottom) && (stPoint.y <= stRectOut.bottom))
		{
			return _Point_On_Rect_SIZENWSE_SE;
		}
		else
		{
			return _Point_On_Rect_NotIn;
		}
	}
	else
	{
		return _Point_On_Rect_NotIn;
	}
}

void CDlgComposite::OnLButtonDblClk(UINT nFlags, CPoint point)
{
	// TODO: 在此添加消息处理程序代码和/或调用默认值

	if (PtInRect(&m_csRegionRectInPIC, point))
	{
		OnBnClickedBtnPreview();
	}

	CDialogEx::OnLButtonDblClk(nFlags, point);
}


void CDlgComposite::OnLButtonDown(UINT nFlags, CPoint point)
{
	// TODO: 在此添加消息处理程序代码和/或调用默认值

	CRect csPicRect, csRectWindows;
	m_csStaticPIC.GetWindowRect(csPicRect);
	
	csRectWindows = m_csDrawRectInPIC;
	csRectWindows.MoveToXY(csPicRect.left + m_csDrawRectInPIC.left, 
		csPicRect.top + m_csDrawRectInPIC.top);

	if (PtInRect(&m_csDrawRectInPIC, point))
	{
		m_csPrevDownPoint = point;

		EMPointOnRectType emType = _Point_On_Rect_NotIn;
		emType = PointInRect(point, m_csRegionRectInPIC);

		if (emType == _Point_On_Rect_NotIn)
		{
			m_csRegionRectInPIC = CRect(point.x, point.y, point.x, point.y);
		}
		m_emMouseStatus = c_emMouseStatus[emType];

		ClipCursor(&csRectWindows);
	}


	CDialogEx::OnLButtonDown(nFlags, point);
}


void CDlgComposite::OnLButtonUp(UINT nFlags, CPoint point)
{
	// TODO: 在此添加消息处理程序代码和/或调用默认值
	if (m_emMouseStatus != _Mouse_UP)
	{
		m_stFRelativeRect.f32Left = (float)(m_csRegionRectInPIC.left - m_csDrawRectInPIC.left) /
			m_csDrawRectInPIC.Width();
		m_stFRelativeRect.f32Right = (float)(m_csRegionRectInPIC.right - m_csDrawRectInPIC.left) /
			m_csDrawRectInPIC.Width();

		m_stFRelativeRect.f32Top = (float)(m_csRegionRectInPIC.top - m_csDrawRectInPIC.top) /
			m_csDrawRectInPIC.Height();
		m_stFRelativeRect.f32Bottom = (float)(m_csRegionRectInPIC.bottom - m_csDrawRectInPIC.top) /
			m_csDrawRectInPIC.Height();
	}
	m_emMouseStatus = _Mouse_UP;

	ClipCursor(NULL);

	CDialogEx::OnLButtonUp(nFlags, point);
}


void CDlgComposite::OnMouseMove(UINT nFlags, CPoint point)
{
	// TODO: 在此添加消息处理程序代码和/或调用默认值
	if (m_emMouseStatus != _Mouse_UP)
	{
		//BuildTheCalibrationInfo(m_csStaticPIC, m_s32SourcePICWidth,
		//		m_pPICCalibrationInfoNeedChange, point);
		//ReDrawStaticPic();

		INT32 x = point.x;
		INT32 y = point.y;

		switch (m_emMouseStatus)
		{
		case _Mouse_PIC_New:
		{
				m_csRegionRectInPIC.right = x;
				m_csRegionRectInPIC.bottom = y;
			break;
		}
		case _Mouse_PIC_RectLeft:
		{
			m_csRegionRectInPIC.left = x;
			break;
		}
		case _Mouse_PIC_RectRight:
		{
			m_csRegionRectInPIC.right = x;
			break;
		}
		case _Mouse_PIC_RectUp:
		{
			m_csRegionRectInPIC.top = y;
			break;
		}
		case _Mouse_PIC_RectDown:
		{
			m_csRegionRectInPIC.bottom = y;
			break;
		}
		case _Mouse_PIC_RectLeftUp:
		{
			m_csRegionRectInPIC.left = x;
			m_csRegionRectInPIC.top = y;
			break;
		}
		case _Mouse_PIC_RectRightUp:
		{

			m_csRegionRectInPIC.right = x;
			m_csRegionRectInPIC.right = y;
			break;
		}
		case _Mouse_PIC_RectLeftDown:
		{

			m_csRegionRectInPIC.bottom = y;
			m_csRegionRectInPIC.left = x;

			break;
		}
		case _Mouse_PIC_RectRightDown:
		{
			m_csRegionRectInPIC.bottom = y;
			m_csRegionRectInPIC.right = x;

			break;
		}
		case _Mouse_PIC_RectMove:
		{
			m_csRegionRectInPIC.MoveToXY(
				m_csRegionRectInPIC.left + x - m_csPrevDownPoint.x,
				m_csRegionRectInPIC.top + y - m_csPrevDownPoint.y);
			m_csPrevDownPoint = point;
			break;
		}

		default:
			break;
		}

	}
	else
	{
		EMPointOnRectType emType = _Point_On_Rect_NotIn;
		emType = PointInRect(point, m_csRegionRectInPIC);
		
		SetCursor(LoadCursor(NULL, c_pCursorType[emType]));
		if (emType != _Point_On_Rect_NotIn)
		{
			SetCapture();
		}
		else
		{
			ReleaseCapture();
		}

	}
	CDialogEx::OnMouseMove(nFlags, point);
}




void CDlgComposite::OnBnClickedBtnPreview()
{
	// TODO: 在此添加控件通知处理程序代码
	StFRelativeRect stRect = m_stFRelativeRect;
	if (stRect.f32Left < 0.0f)
	{
		stRect.f32Left = 0.0f;
	}

	if (stRect.f32Top < 0.0f)
	{
		stRect.f32Top = 0.0f;
	}

	if (stRect.f32Bottom > 1.0f)
	{
		stRect.f32Bottom = 1.0f;
	}

	if (stRect.f32Right > 1.0f)
	{
		stRect.f32Right = 1.0f;
	}

	if (stRect.f32Left > stRect.f32Right)
	{
		return;
	}

	if (stRect.f32Top > stRect.f32Bottom)
	{
		return;
	}

	CRect csRect;
	csRect.left = (LONG)(m_u32CompsiteBmpWidth * stRect.f32Left + 0.5f);
	csRect.right = (LONG)(m_u32CompsiteBmpWidth * stRect.f32Right + 0.5f);

	//csRect.top = (LONG)(m_u32CompsiteBmpHeight * (1.0 - stRect.f32Bottom) + 0.5f);
	//csRect.bottom = (LONG)(m_u32CompsiteBmpHeight * (1.0 - stRect.f32Top) + 0.5f);
	csRect.top = (LONG)(m_u32CompsiteBmpHeight * (stRect.f32Top) + 0.5f);
	csRect.bottom = (LONG)(m_u32CompsiteBmpHeight * (stRect.f32Bottom) + 0.5f);

	StRGB32Bit stRGB = { 0 };
	stRGB.s32Width = csRect.Width();
	stRGB.s32Height = csRect.Height();
	stRGB.pRGB = (uint32_t *)malloc(stRGB.s32Width * stRGB.s32Height * 4);
	if (stRGB.pRGB == NULL)
	{
		return;
	}
	StRGB32Bit stRGBSrc =
	{
		(int32_t)m_u32CompsiteBmpWidthMaxCount,
		(int32_t)m_u32CompsiteBmpHeightMaxCount,
		m_pBmpBuf32BitMaxCount,
	};
	RGBCopy(&stRGB, 0, 0, &stRGBSrc,
		csRect.left, csRect.top, csRect.Width(), csRect.Height());

	if (m_pDlgTemp != NULL)
	{
		((CDlgTemp *)m_pDlgTemp)->ReloadBMP(&stRGB);
	}

#if 0
	{
		FILE *pFile = NULL;
		INT ret = _wfopen_s(&pFile, L"f:\\DCLK.bmp", L"wb+");
		if (ret == 0)
		{
			BufToBmp(pFile, (uint8_t *)stRGB.pRGB,
				stRGB.s32Height, stRGB.s32Width, 4, true);
			fclose(pFile);
		}
	}
#endif

#if 0
	{
		for (UINT i = 0; i < stRGB.s32Height; i++)
		{
			g_u32Row[i] = *(stRGB.pRGB + i *  stRGB.s32Width);
		}
		g_u32Row[0] = g_u32Row[0];
	}

	{
		for (UINT i = 0; i < stRGBSrc.s32Height; i++)
		{
			g_u32Row[i] = *(stRGBSrc.pRGB + i *  stRGBSrc.s32Width);
		}
		g_u32Row[0] = g_u32Row[0];
	}
#endif

	free(stRGB.pRGB);

}
