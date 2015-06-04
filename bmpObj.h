#ifndef __BMPOBJ_H__
#define __BMPOBJ_H__

#include "common.h"

extern HINSTANCE hInst;

// �r�b�g�}�b�v����p�N���X
class CBmpObj
{
private:

	DWORD SizeImage( BITMAPINFOHEADER );

public:
	HBITMAP hBitmap;
	BITMAPFILEHEADER	bmpFileHeader;
	BITMAPINFOHEADER	bmpInfoHeader;

	CBmpObj();
	~CBmpObj();

	void DesktopToFile( void );
	void LoadBitmapFromFile( LPTSTR );
};

#endif // __BMPOBJ_H__
