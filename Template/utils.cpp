#include "stdafx.h"
#include "utils.h"

int64_t GetFolderSize(const wchar_t *pPath, PFUN_GetFolderSizeCB pFunCB/* = NULL*/,
	void *pContext/* = NULL*/)
{
	stack<wstring> s;
	s.push(pPath);
	HANDLE hFile = NULL;
	WIN32_FIND_DATA stNextInfo = { 0 };
	CFileStatus stFileStatus = { 0 };
	int64_t s64Size = 0;
	while (!s.empty()) {
		wstring csWStrStatckPath = s.top();
		s.pop();
		wstring csWStrStatckPathComplete = csWStrStatckPath;
		csWStrStatckPathComplete.append(L"\\*.*");
		hFile = FindFirstFile(csWStrStatckPathComplete.c_str(), &stNextInfo);
		do
		{
			wstring csTmp = csWStrStatckPath;
			csTmp.append(L"\\").append(stNextInfo.cFileName);

			if (pFunCB != NULL)
			{
				pFunCB(csTmp.c_str(), &stNextInfo, pContext);
			}

			if (stNextInfo.dwFileAttributes  & FILE_ATTRIBUTE_DIRECTORY)
			{
				if (lstrcmp(stNextInfo.cFileName, L".") && lstrcmp(stNextInfo.cFileName, L".."))
				{
					//s.push(csTmp);
				}
			}
			else
			{
				if (CFile::GetStatus(csTmp.c_str(), stFileStatus))
				{
					s64Size += stFileStatus.m_size;
				}
			}
		} while (FindNextFile(hFile, &stNextInfo));
	}
	return s64Size;
}

int32_t Convert(const char *pStrSrc, wstring &csWscDest)
{
	if (pStrSrc == NULL)
	{
		return -1;
	}
	int s32SrcLen = strlen(pStrSrc);

	int s32UnicodeLen = MultiByteToWideChar(CP_ACP, 0, pStrSrc, -1, NULL, 0);

	wchar_t * pUnicode;
	pUnicode = new wchar_t[s32UnicodeLen + 1];

	memset(pUnicode, 0, (s32UnicodeLen + 1) * sizeof(wchar_t));

	MultiByteToWideChar(CP_ACP, 0, pStrSrc, -1, (LPWSTR)pUnicode,
		s32UnicodeLen);
	csWscDest = pUnicode;
	delete[] pUnicode;
	return 0;
}
int32_t Convert(const wchar_t *pwStrSrc, string &csStrDrst)
{
	if (pwStrSrc == NULL)
	{
		return -1;
	}
	int s32UnicodeSrcLen =  wcslen(pwStrSrc);

	int s32AnisLen = WideCharToMultiByte(CP_ACP, 0, pwStrSrc, -1, NULL, 0,
		NULL, NULL);

	char * pAnis;
	pAnis = new char[s32AnisLen + 1];

	memset(pAnis, 0, (s32AnisLen + 1) * sizeof(char));

	WideCharToMultiByte(CP_ACP, 0, pwStrSrc, -1, (LPSTR)pAnis,
		s32AnisLen, NULL, NULL);
	csStrDrst = pAnis;
	delete[] pAnis;
	return 0;
}

static bool FolderExists(CString s)
{
	DWORD attr;
	attr = GetFileAttributes(s);
	return (attr != (DWORD)(-1)) &&
		(attr & FILE_ATTRIBUTE_DIRECTORY);
}

bool SuperMkDir(CString P)
{
	int len = P.GetLength();
	if (len < 2)
		return false;

	if ('\\' == P[len - 1])
	{
		P = P.Left(len - 1);
		len = P.GetLength();
	}
	if (len <= 0)
		return false;

	if (len <= 3)
	{
		if (FolderExists(P))
			return true;
		else
			return false;
	}

	if (FolderExists(P))
		return true;



	CString Parent;
	Parent = P.Left(P.ReverseFind('\\'));

	bool Ret = false;
	if (Parent.GetLength() <= 0)
	{
		Ret = true;
	}
	else
	{
		Ret = SuperMkDir(Parent);
	}

	if (Ret)
	{
		SECURITY_ATTRIBUTES sa;
		sa.nLength = sizeof(SECURITY_ATTRIBUTES);
		sa.lpSecurityDescriptor = NULL;
		sa.bInheritHandle = 0;
		Ret = (CreateDirectory(P, &sa) == TRUE);
		return Ret;
	}
	else
		return false;
}


int32_t BufToBmp(FILE *pFile, uint8_t *Buf, int32_t s32Height, int32_t s32Width, int32_t s32BufOffset, bool boIsYP)
{

	BITMAPFILEHEADER stBmpHeader = { 0 };

	BITMAPINFOHEADER stInfoHeader = { 0 };

	stBmpHeader.bfType = 0x4D42;

	stBmpHeader.bfOffBits = sizeof(BITMAPFILEHEADER) + sizeof(BITMAPINFOHEADER);

	stInfoHeader.biSize = sizeof(BITMAPINFOHEADER);
	stInfoHeader.biWidth = s32Width;
	if (!boIsYP)
	{
		stInfoHeader.biHeight = 0 - s32Height;
	}
	else
	{
		stInfoHeader.biHeight = s32Height;
	}
	stInfoHeader.biPlanes = 1;
	stInfoHeader.biBitCount = 24;
	stInfoHeader.biCompression = 0;
	stInfoHeader.biSizeImage = (((s32Width * 3 + 3) / 4) * 4) * s32Height + 2;/* 54 ----> 56 */

	stInfoHeader.biXPelsPerMeter = stInfoHeader.biYPelsPerMeter = 0x1E40;

	uint8_t *pBmpBuf = (uint8_t *)calloc(1, stInfoHeader.biSizeImage);

	if (pBmpBuf == NULL)
	{
		return -1;
	}

	if (!boIsYP)
	{
		int32_t s32WidthDestTmp = ((s32Width * 3 + 3) / 4) * 4;
		int32_t s32WidthSrcTmp = s32Width * s32BufOffset;
		uint8_t *pTmpDest = pBmpBuf + s32WidthDestTmp * (s32Height - 1);
		uint8_t *pTmpSrc = Buf;
		for (int32_t i = 0; i < s32Height; i++)
		{
			for (int32_t j = 0; j < s32Width; j++)
			{
				memcpy(pTmpDest + j * 3, pTmpSrc + j * s32BufOffset, 3);
			}

			pTmpDest -= s32WidthDestTmp;
			pTmpSrc += s32WidthSrcTmp;
		}
	}
	else
	{
		int32_t s32WidthDestTmp = ((s32Width * 3 + 3) / 4) * 4;
		int32_t s32WidthSrcTmp = s32Width * s32BufOffset;
		uint8_t *pTmpDest = pBmpBuf;
		uint8_t *pTmpSrc = Buf;
		for (int32_t i = 0; i < s32Height; i++)
		{
			for (int32_t j = 0; j < s32Width; j++)
			{
				memcpy(pTmpDest + j * 3, pTmpSrc + j * s32BufOffset, 3);
			}

			pTmpDest += s32WidthDestTmp;
			pTmpSrc += s32WidthSrcTmp;
		}
	}

	fseek(pFile, 0, SEEK_SET);

	fwrite(&stBmpHeader, 1, sizeof(BITMAPFILEHEADER), pFile);
	fwrite(&stInfoHeader, 1, sizeof(BITMAPINFOHEADER), pFile);
	fwrite(pBmpBuf, 1, stInfoHeader.biSizeImage, pFile);

	free(pBmpBuf);

	return 0;
}

int32_t LoadBmp(FILE *pFile, StRGB32Bit *pDest)
{
	if (pFile == NULL || pDest == NULL)
	{
		return -1;
	}

	fseek(pFile, 0, SEEK_SET);

	BITMAPFILEHEADER stBmpHeader = { 0 };

	BITMAPINFOHEADER stInfoHeader = { 0 };

	size_t u32Read = fread_s(&stBmpHeader, sizeof(BITMAPFILEHEADER), 1, sizeof(BITMAPFILEHEADER), pFile);
	if (u32Read != sizeof(BITMAPFILEHEADER))
	{
		return -1;
	}
	u32Read = fread_s(&stInfoHeader, sizeof(BITMAPINFOHEADER), 1, sizeof(BITMAPINFOHEADER), pFile);
	if (u32Read != sizeof(BITMAPINFOHEADER))
	{
		return -1;
	}

	if (stBmpHeader.bfType != 0x4D42)
	{
		return -1;
	}

	if (stInfoHeader.biBitCount != 24 && stInfoHeader.biBitCount != 32)
	{
		return -1;
	}

	int32_t s32BufOffset = stInfoHeader.biBitCount / 8;
	int32_t s32Pitch = (s32BufOffset * stInfoHeader.biWidth + 3) & (~0x03);
	bool boIsYP = false;
	if (stInfoHeader.biHeight < 0)
	{
		stInfoHeader.biHeight = 0 - stInfoHeader.biHeight;
	}
	else
	{
		boIsYP = true;
	}

	if (stInfoHeader.biSizeImage == 0)
	{
		stInfoHeader.biSizeImage = (((stInfoHeader.biWidth * s32BufOffset + 3) / 4) * 4) *
			stInfoHeader.biHeight;
	}
	uint8_t *pSrc = (uint8_t *)malloc(stInfoHeader.biSizeImage);
	if (pSrc == NULL)
	{
		return -1;
	}
	AutoFree csFreeSrcBuf(pSrc);

	if (pDest->s32Height != stInfoHeader.biHeight ||
		pDest->s32Width != stInfoHeader.biWidth)
	{
		if (pDest->pRGB != NULL)
		{
			free(pDest->pRGB);
			pDest->pRGB = NULL;
		}
	}

	if (pDest->pRGB == NULL)
	{
		pDest->s32Width = stInfoHeader.biWidth;
		pDest->s32Height = stInfoHeader.biHeight;

		pDest->pRGB = (UINT32 *)malloc(sizeof(UINT32) * 
			pDest->s32Width *
			pDest->s32Height);
	}
	if (pDest->pRGB == NULL)
	{
		return -1;
	}

	fseek(pFile, stBmpHeader.bfOffBits, SEEK_SET);
	u32Read = fread_s(pSrc, stInfoHeader.biSizeImage, 1, stInfoHeader.biSizeImage, pFile);
	if (u32Read != stInfoHeader.biSizeImage)
	{
		free(pDest->pRGB);
		pDest->pRGB = NULL;
		return -1;
	}

	if (!boIsYP)
	{
		uint8_t *pTmpDest = (uint8_t *)pDest->pRGB;
		uint8_t *pTmpSrc = pSrc + s32Pitch * (pDest->s32Height - 1);
		if (s32BufOffset == 4)
		{
			for (int32_t i = 0; i < pDest->s32Height; i++)
			{
				memcpy(pTmpDest, pTmpSrc, 4 * pDest->s32Width);
				pTmpDest += (4 * pDest->s32Width);
				pTmpSrc -= s32Pitch;
			}
		}
		else
		{
			for (int32_t i = 0; i < pDest->s32Height; i++)
			{

				for (int32_t j = 0; j < pDest->s32Width; j++)
				{
					memcpy(pTmpDest + j * 4, pTmpSrc + j * s32BufOffset, s32BufOffset);
				}
				pTmpDest += (4 * pDest->s32Width);
				pTmpSrc -= s32Pitch;
			}
		}
	}
	else
	{
		uint8_t *pTmpDest = (uint8_t *)pDest->pRGB;
		uint8_t *pTmpSrc = pSrc;
		if (s32BufOffset == 4)
		{
			for (int32_t i = 0; i < pDest->s32Height; i++)
			{
				memcpy(pTmpDest, pTmpSrc, 4 * pDest->s32Width);
				pTmpDest += (4 * pDest->s32Width);
				pTmpSrc += s32Pitch;
			}
		}
		else
		{
			for (int32_t i = 0; i < pDest->s32Height; i++)
			{
				for (int32_t j = 0; j < pDest->s32Width; j++)
				{
					memcpy(pTmpDest + j * 4, pTmpSrc + j * s32BufOffset, s32BufOffset);
				}
				pTmpDest += (4 * pDest->s32Width);
				pTmpSrc += s32Pitch;
			}
		}
	}

	return 0;

}

int32_t RGBCopy(StRGB32Bit *pDest, int32_t s32DestX, int32_t s32DestY,
	StRGB32Bit *pSrc, int32_t s32SrcX, int32_t s32SrcY,
	int32_t s32Width, int32_t s32Height, bool boYFlip/* = false*/)
{
	if (pDest == NULL)
	{
		return -1;
	}
	if (s32DestX < 0 || s32DestX >= pDest->s32Width)
	{
		return -1;
	}

	if (s32DestY < 0 || s32DestY >= pDest->s32Height)
	{
		return -1;
	}

	if (s32SrcX < 0 || s32Width <= 0 ||  s32SrcX >= pSrc->s32Width)
	{
		return -1;
	}


	if (s32SrcY < 0 || s32Height <= 0 || s32SrcY >= pSrc->s32Height)
	{
		return -1;
	}

	if (s32DestY + s32Height > pDest->s32Height)
	{
		s32Height = pDest->s32Height - s32DestY;
	}

	if (s32DestX + s32Width > pDest->s32Width)
	{
		s32Width = pDest->s32Width - s32DestX;
	}

	if (boYFlip)
	{
		uint32_t *pDestTmp = pDest->pRGB + s32DestY * pDest->s32Width + s32DestX;
		uint32_t *pSrcTmp = pSrc->pRGB + (s32SrcY + s32Height - 1) * pSrc->s32Width + s32SrcX;

		for (int32_t j = 0; j < s32Height; j++)
		{
			memcpy(pDestTmp, pSrcTmp, s32Width * 4);
			pDestTmp += pDest->s32Width;
			pSrcTmp -= pSrc->s32Width;
		}
	}
	else
	{
		uint32_t *pDestTmp = pDest->pRGB + s32DestY * pDest->s32Width + s32DestX;
		uint32_t *pSrcTmp = pSrc->pRGB + s32SrcY * pSrc->s32Width + s32SrcX;

		for (int32_t j = 0; j < s32Height; j++)
		{
			memcpy(pDestTmp, pSrcTmp, s32Width * 4);
			pDestTmp += pDest->s32Width;
			pSrcTmp += pSrc->s32Width;
		}

	}

	return 0;
}


