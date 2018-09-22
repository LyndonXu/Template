#pragma once

#include <stdint.h>
#include <string>
#include <stack>

#include <windows.h>

using namespace std;

#define PRINT(x, ...)	{char temp[512];sprintf_s(temp,512,"[%s, %d]: "##x, __FILE__, __LINE__, ##__VA_ARGS__);OutputDebugStringA(temp);}

class CritSec
{
public:
	CritSec() { InitializeCriticalSection(&m_CritSec); };
	~CritSec() { DeleteCriticalSection(&m_CritSec); };
	void Lock() { EnterCriticalSection(&m_CritSec); };
	void UnLock() { LeaveCriticalSection(&m_CritSec); };

private:
	CRITICAL_SECTION m_CritSec;
};

class AutoLock
{
public:
	AutoLock(CritSec * plock) {
		m_pLock = plock;
		m_pLock->Lock();
	};

	~AutoLock() {
		m_pLock->UnLock();
	};

private:
	CritSec * m_pLock;
};

class AutoFree
{
public:
	AutoFree() : m_pMem(NULL)
	{
	}
	AutoFree(void *pMem): m_pMem(NULL)
	{
		m_pMem = pMem;
	};

	~AutoFree()
	{
		if (m_pMem != NULL)
		{
			free(m_pMem);
		}
	};
	void SetMem(void *pMem)
	{
		m_pMem = pMem;
	}

private:
	void * m_pMem;
};

typedef struct _tagStRGB32Bit
{
	int32_t s32Width;
	int32_t s32Height;
	uint32_t *pRGB;
}StRGB32Bit;


typedef int32_t(*PFUN_GetFolderSizeCB)(const wchar_t *pName, WIN32_FIND_DATA *pInfo, void *pContext);
int64_t GetFolderSize(const wchar_t *pPath, PFUN_GetFolderSizeCB pFunCB = NULL,
	void *pContext = NULL);
int32_t Convert(const char *pStrSrc, wstring &csWscDest);
int32_t Convert(const wchar_t *pwStrSrc, string &csStrDrst);
bool SuperMkDir(CString P);
int32_t BufToBmp(FILE *pFile, uint8_t *Buf, int32_t s32Height,
	int32_t s32Width, int32_t s32BufOffset, bool boIsYP);
int32_t RGBCopy(StRGB32Bit *pDest, int32_t s32DesX, int32_t s32DesY,
	StRGB32Bit *pSrc, int32_t s32SrcX, int32_t s32SrcY,
	int32_t s32Width, int32_t s32Height);
