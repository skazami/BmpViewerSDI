#ifndef __BMPOBJ_H__
#define __BMPOBJ_H__

#include "common.h"

extern HINSTANCE hInst;

// �r�b�g�}�b�v����p�N���X
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

	// Bitmap�����E����֘A
	void LoadBitmapFromFile( LPTSTR );
	void CreateBitmap( LPBITMAPINFO );
	int  ReleaseBitmap( void );

	// Bitmap�Ǘ��p
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
		return this->lpBmpInfo->bmiColors; // TODO �i�������g�p�ʓI�ɂ͕s���ɂȂ邪�j�ł���΃R�s�[��n�����ق����ǂ��B���̃f�[�^��ی삷�邽�߁B
	}


	// Bitmap�ϊ�
	int RotateLeftRightAngle(HBITMAP);


	// ���̑��̋@�\
	void DesktopToFile( void );

};

#endif // __BMPOBJ_H__
