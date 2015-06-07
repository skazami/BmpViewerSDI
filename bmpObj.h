#ifndef __BMPOBJ_H__
#define __BMPOBJ_H__

#include "common.h"

extern HINSTANCE hInst;

// ビットマップ操作用クラス
class CBmpObj
{
private:

	HBITMAP hBitmap;
	HPALETTE hPalette;

	HANDLE              hMemBmpInfo;
	LPBITMAPINFO        lpBmpInfo;
	BITMAPFILEHEADER	bmpFileHeader;

	HPALETTE SetPalette( void );
	int  ReadHeader( LPTSTR );

public:

	CBmpObj();
	virtual ~CBmpObj();

	void DesktopToFile( void );
	void LoadBitmapFromFile( LPTSTR );
	void CreateBitmap( LPBITMAPINFO, DWORD ); // TODO implement this method

	int ReleaseBitmap( void );

	HBITMAP GetBitmapHandle( void )
	{
		return this->hBitmap;
	}

	BITMAPFILEHEADER GetBitmapFileHeader(void)
	{
		return this->bmpFileHeader;
	}

	BITMAPINFOHEADER GetBitmapInfoHeader(void)
	{
		BITMAPINFOHEADER bih;

		if( this->lpBmpInfo != NULL )
		{
			bih = this->lpBmpInfo->bmiHeader;
		}
		else
		{
			ZeroMemory(&bih, sizeof(BITMAPINFOHEADER));
		}
		return bih;
	}

	LPRGBQUAD GetRgbQuadData(void)
	{
		return this->lpBmpInfo->bmiColors; // TODO （メモリ使用量的には不利になるが）できればコピーを渡したほうが良い。元のデータを保護するため。
	}
};

#endif // __BMPOBJ_H__
