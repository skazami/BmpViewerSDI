#ifndef __BMPOBJ_H__
#define __BMPOBJ_H__

#include "common.h"

extern HINSTANCE hInst;

// ビットマップ操作用クラス
class CBmpObj
{
private:

	HBITMAP hBitmap;

	DWORD SizeImage( BITMAPINFOHEADER );

	DWORD dwFileSize;
	DWORD dwOffBits;

	HANDLE hMemRgbQuad;

	HPALETTE SetPalette( void );
	int  ReadHeader( LPTSTR );

	BITMAPFILEHEADER	bmpFileHeader;
	BITMAPINFOHEADER	bmpInfoHeader;

public:
	HPALETTE hPalette;

	LPRGBQUAD           lpRgbQuad;

	CBmpObj();
	virtual ~CBmpObj();

	void DesktopToFile( void );
	void LoadBitmapFromFile( LPTSTR );
	void CreateBitmap( BITMAPFILEHEADER, BITMAPINFOHEADER );

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
		return this->bmpInfoHeader;
	}
};

#endif // __BMPOBJ_H__
