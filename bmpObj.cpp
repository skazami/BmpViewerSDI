#include "common.h"
#include "bmpObj.h"


#define	DP_biBitCount		24


//--------------------------------------------------------------------------------
//--------------------------------------------------------------------------------
CBmpObj::CBmpObj()
{
	hBitmap     = NULL;
	hPalette    = NULL;
	lpBmpInfo   = NULL;
	hMemBmpInfo = NULL;
}

//--------------------------------------------------------------------------------
//--------------------------------------------------------------------------------
CBmpObj::~CBmpObj()
{
	this->ReleaseBitmap();
}

//--------------------------------------------------------------------------------
//--------------------------------------------------------------------------------
void CBmpObj::DesktopToFile( void )
{
	HANDLE	hFile;
	DWORD	writesize;
	PBYTE	frameBuffer, src;
	HDC		hDC, hComDC;
	HBITMAP	hDIB, hBmpOld;
	DWORD	rectH, rectW, j;
	DWORD	SizeWide;
	BITMAPFILEHEADER		bmpFH;		//ﾋﾞｯﾄﾏｯﾌﾟﾌｧｲﾙﾍｯﾀﾞ
	static struct {
		BITMAPINFOHEADER	bmpIH;		//ﾋﾞｯﾄﾏｯﾌﾟﾌｧｲﾙﾍｯﾀﾞ
	} bmpinfo;
	TCHAR	BmpFN[ MAX_PATH ];			//ﾌｧｲﾙ名

	//画面サイズを取得
	rectW = GetSystemMetrics( SM_CXSCREEN );
	rectH = GetSystemMetrics( SM_CYSCREEN );

	//　ビットマップファイルヘッダを作成
	bmpinfo.bmpIH.biSize          = sizeof( bmpinfo.bmpIH );
	bmpinfo.bmpIH.biWidth         = rectW;
	bmpinfo.bmpIH.biHeight        = rectH;
	bmpinfo.bmpIH.biPlanes        = 1;
	bmpinfo.bmpIH.biBitCount      = DP_biBitCount;
	bmpinfo.bmpIH.biCompression   = BI_RGB;
	bmpinfo.bmpIH.biSizeImage     = 0;
	bmpinfo.bmpIH.biXPelsPerMeter = 0;
	bmpinfo.bmpIH.biYPelsPerMeter = 0;
	bmpinfo.bmpIH.biClrUsed       = 0;
	bmpinfo.bmpIH.biClrImportant  = 0;

	//画面の内容を取得
	hDC = GetDC(NULL);
	hDIB = CreateDIBSection( hDC, (PBITMAPINFO)&bmpinfo.bmpIH, DIB_RGB_COLORS, (void **)&frameBuffer, NULL, 0 );
	if ( !hDIB ) {
		ReleaseDC( NULL, hDC );
		return;
	}
	hComDC = CreateCompatibleDC( hDC );
	if ( !hComDC ) {
		DeleteObject( hDIB );
		ReleaseDC( NULL, hDC );
		return;
	}
	hBmpOld = (HBITMAP)SelectObject( hComDC, hDIB );
	BitBlt( hComDC, 0, 0, rectW, rectH, hDC, 0, 0, SRCCOPY );

	SizeWide = rectW * DP_biBitCount / 8;

	// BITMAPFILEHEADERを作成
    bmpFH.bfType      = 0x4D42;
	bmpFH.bfSize      = sizeof( bmpFH ) + sizeof( bmpinfo ) + ( rectH * SizeWide );
    bmpFH.bfReserved1 = 0;
    bmpFH.bfReserved2 = 0;
    bmpFH.bfOffBits   = sizeof( bmpFH ) + sizeof( bmpinfo );

#if 0
	if( !GetSaveFileNameSetting() )
		goto RETURN;
#endif

//	wsprintf(BmpFN, _T("\\My Documents\\bitmap01.bmp"));
	wsprintf(BmpFN, _T("F:\\work\\Program\\Prog\\BmpViewerSDI\\bitmap01.bmp"));

	//ビットマップファイルに保存
	if ( (hFile = CreateFile( BmpFN, GENERIC_WRITE, 0, 0, CREATE_ALWAYS, FILE_ATTRIBUTE_NORMAL, 0 )) == INVALID_HANDLE_VALUE ){
		goto RETURN;
	}

	WriteFile( hFile, &bmpFH  , sizeof( bmpFH )  , &writesize, NULL );
	WriteFile( hFile, &bmpinfo, sizeof( bmpinfo ), &writesize, NULL );
	src = &frameBuffer[ 0 ];

	for ( j = 0; j < rectH; j++) {
		WriteFile( hFile, src, SizeWide, &writesize, NULL );
		src += SizeWide;
	}

	CloseHandle( hFile );

RETURN:
	SelectObject( hComDC, hBmpOld );
	DeleteObject( hDIB );
	DeleteDC( hComDC );
	ReleaseDC( NULL, hDC );

	return;
}

//--------------------------------------------------------------------------------
//--------------------------------------------------------------------------------
void CBmpObj::LoadBitmapFromFile( LPTSTR szFileName )
{
	HANDLE	hFile=NULL;
	PBYTE	frameBuffer=NULL;

	this->ReleaseBitmap();

#if 1
	hBitmap = (HBITMAP)LoadImage(hInst, szFileName, IMAGE_BITMAP, 0, 0, LR_CREATEDIBSECTION|LR_LOADFROMFILE);
	ReadHeader(szFileName);
	SetPalette();
#else
	int		i;
	DWORD	readbytes;
	DWORD	rowbytes;

	//ビットマップをファイルから読み出し
	if ( (hFile = CreateFile( szFileName, GENERIC_READ, 0, NULL, OPEN_EXISTING, FILE_ATTRIBUTE_NORMAL, 0 )) == INVALID_HANDLE_VALUE ){
		return;
	}

	ReadFile( hFile, &bmpFileHeader, sizeof( BITMAPFILEHEADER ), &readbytes, NULL );
	ReadFile( hFile, &bmpInfoHeader, sizeof( BITMAPINFOHEADER ), &readbytes, NULL );

	if( (frameBuffer = (PBYTE)LocalAlloc(LPTR, sizeof(BYTE)*SizeImage(bmpInfoHeader))) == NULL ){
		goto RETURN;
	}

	rowbytes = SizeImage(bmpInfoHeader) / bmpInfoHeader.biHeight;
	for( i=bmpInfoHeader.biHeight-1; i>=0; i--)
	{
		ReadFile( hFile, frameBuffer + (i * rowbytes), rowbytes, &readbytes, NULL );
	}
	
#if 1
	{
		TCHAR buf[100];

		wsprintf(buf, _T("sizeof(BFH): %d\nsizeof(BIF): %d\nbiSizeImage: %d\nWidth: %d\nHeight: %d\nbitCount: %d\nInferred ImageSize: %d bytes"),
			sizeof( BITMAPFILEHEADER ), sizeof( BITMAPINFOHEADER ), bmpInfoHeader.biSizeImage,
			bmpInfoHeader.biWidth, bmpInfoHeader.biHeight, bmpInfoHeader.biBitCount,
			SizeImage(bmpInfoHeader));
		MessageBox(NULL, buf, _T(""), MB_OK);
	}
#endif

	// ビットマップのGDIオブジェクトを作成
	// カラービットマップのオブジェクト生成処理検証 → どうやらこれでは動作しない。調査保留。
	{
		hBitmap = CreateBitmap(bmpInfoHeader.biWidth, bmpInfoHeader.biHeight, bmpInfoHeader.biPlanes, bmpInfoHeader.biBitCount, frameBuffer);
		if ( !hBitmap ) {
			goto RETURN;
		}
	}

RETURN:
	CloseHandle( hFile );
	LocalFree(frameBuffer);
#endif
	return;
}

//--------------------------------------------------------------------------------
//--------------------------------------------------------------------------------
void CBmpObj::CreateBitmap(LPBITMAPINFO lpBmi)
{
	LPVOID lp_void;
	DWORD dwHeaderSize;
	DWORD dwClrUsed;

	this->ReleaseBitmap();

	this->hBitmap = CreateDIBSection(NULL, lpBmi, DIB_RGB_COLORS, &lp_void, NULL, 0);

	if( lpBmi->bmiHeader.biClrUsed == 0 && lpBmi->bmiHeader.biBitCount < 16 )
	{
		dwClrUsed = 2 << lpBmi->bmiHeader.biBitCount;
	}
	else
	{
		dwClrUsed = lpBmi->bmiHeader.biClrUsed;
	}

	dwHeaderSize = sizeof(BITMAPINFOHEADER)+ sizeof(RGBQUAD)*dwClrUsed;
	hMemBmpInfo = GlobalAlloc(GMEM_FIXED, dwHeaderSize);
	this->lpBmpInfo = (LPBITMAPINFO)GlobalLock(hMemBmpInfo);

	memcpy(this->lpBmpInfo, lpBmi, dwHeaderSize);

	return;
}

//--------------------------------------------------------------------------------
//--------------------------------------------------------------------------------
int CBmpObj::ReadHeader(LPTSTR szFileName)
{
	int ret=0;
    HANDLE hFile = NULL;
    DWORD dwResult;

	BITMAPINFOHEADER bih;
	DWORD dwBmpInfoSize;

    hFile = CreateFile(szFileName, GENERIC_READ, 0, NULL, OPEN_EXISTING, FILE_ATTRIBUTE_NORMAL, NULL);
    if (hFile == INVALID_HANDLE_VALUE) {
        ret = -1;
		goto RETURN;
    }

    ReadFile(hFile, (LPBITMAPFILEHEADER)(&this->bmpFileHeader), sizeof(BITMAPFILEHEADER), &dwResult, NULL);

	if ( this->bmpFileHeader.bfType != 0x4D42 ) { // 0x4D42 -> 'BM'
        ret= -2;
		goto RETURN;
    }

    ReadFile(hFile, &bih, sizeof(BITMAPINFOHEADER), &dwResult, NULL);

	if( bih.biSize != 40 ) // Windows Bitmapのみ対応
	{
        ret= -3;
		goto RETURN;
	}

	if( bih.biClrUsed == 0 && bih.biBitCount < 16 )
	{
		bih.biClrUsed = 2 << bih.biBitCount;
	}

	dwBmpInfoSize = sizeof(BITMAPINFOHEADER)+ sizeof(RGBQUAD)*bih.biClrUsed;
	hMemBmpInfo = GlobalAlloc(GMEM_FIXED, dwBmpInfoSize);
	this->lpBmpInfo = (LPBITMAPINFO)GlobalLock(hMemBmpInfo);

	this->lpBmpInfo->bmiHeader = bih;

	SetFilePointer(hFile, sizeof(BITMAPFILEHEADER)+sizeof(BITMAPINFOHEADER), 0, FILE_BEGIN);
	ReadFile(hFile, this->lpBmpInfo->bmiColors, bih.biClrUsed * sizeof(RGBQUAD), &dwResult, NULL);

RETURN:
	if( hFile != NULL )
	{
	    CloseHandle(hFile);
	}
    return ret;
}



//--------------------------------------------------------------------------------
//--------------------------------------------------------------------------------
HPALETTE CBmpObj::SetPalette(void)
{
    LPBITMAPINFOHEADER lpBih;
    LPLOGPALETTE lpPal;
    LPRGBQUAD lpRGB;
    HANDLE hPal;
    WORD i;

	if( hBitmap == NULL )
	{
		return NULL;
	}

	lpBih = &this->lpBmpInfo->bmiHeader;

	if( lpBih->biClrUsed == 0 || lpBih->biBitCount > 8 )
	{
		return NULL;
	}

    hPal = GlobalAlloc(GMEM_FIXED, sizeof(LOGPALETTE) + lpBih->biClrUsed * sizeof(PALETTEENTRY));
    lpPal = (LPLOGPALETTE)GlobalLock(hPal);

    lpPal->palVersion = 0x300;
    lpPal->palNumEntries = (WORD)lpBih->biClrUsed;

	lpRGB = this->lpBmpInfo->bmiColors;

    for (i = 0; i < lpBih->biClrUsed; i++, lpRGB++) { // 構造体内の色の並び順が異なるため、単なるコピーではNG、、、
        lpPal->palPalEntry[i].peRed   = lpRGB->rgbRed;
        lpPal->palPalEntry[i].peGreen = lpRGB->rgbGreen;
        lpPal->palPalEntry[i].peBlue  = lpRGB->rgbBlue;
        lpPal->palPalEntry[i].peFlags = 0;
    }

    GlobalUnlock(hPal);
    hPalette = CreatePalette(lpPal);
    if (hPalette == NULL) {
        return NULL;
    }
    GlobalFree(hPal);

    return hPalette;
}

//--------------------------------------------------------------------------------
//--------------------------------------------------------------------------------
int CBmpObj::ReleaseBitmap( void )
{
	if( hBitmap != NULL ){
		DeleteObject(hBitmap);
		hBitmap = NULL;
	}

	if(hPalette != NULL){
		DeleteObject(hPalette);
		hPalette = NULL;
	}

	if( hMemBmpInfo != NULL )
	{
	    GlobalUnlock(hMemBmpInfo);
		GlobalFree(hMemBmpInfo);
		hMemBmpInfo = NULL;
	}

	return 0;
}


//--------------------------------------------------------------------------------
//--------------------------------------------------------------------------------
int CBmpObj::RotateLeftRightAngle(HBITMAP hBitmapDst)
{
	int x,y;
	LPBYTE lpSrc, lpDst;
	BITMAP bmSrc, bmDst;

	int srcBitPos, srcBitBytePos, srcBitByteBitPos;
	int dstBitPos, dstBitBytePos, dstBitByteBitPos;
	int srcBit;
	LPBYTE writeBytePtr;

	if( this->hBitmap == NULL )
	{
		return -1;
	}
	
	if( hBitmapDst == NULL )
	{
		return -2;
	}

	GetObject(this->hBitmap, sizeof(BITMAP), &bmSrc);
	lpSrc = (LPBYTE)bmSrc.bmBits;

	GetObject(hBitmapDst, sizeof(BITMAP), &bmDst);
	lpDst = (LPBYTE)bmDst.bmBits;

	switch ( bmDst.bmBitsPixel )
	{
	case 1:
		ZeroMemory(lpDst, bmDst.bmWidthBytes*bmDst.bmHeight);
					
		for(y=0;y<bmSrc.bmHeight;y++)
		{
			for(x=0;x<bmSrc.bmWidth;x++)
			{
				srcBitPos        = bmSrc.bmWidthBytes*8*y + x; // この時点では同一バイト内で上位下位が逆。srcBitByteBitPosで変換する
				srcBitBytePos    = srcBitPos / 8;
				srcBitByteBitPos = 7-(srcBitPos % 8);
				srcBit = ( (*(lpSrc+srcBitBytePos)) >> srcBitByteBitPos ) & 0x1;

				dstBitPos        = bmDst.bmWidthBytes*8*x + bmDst.bmWidth-1-y;
				dstBitBytePos    = dstBitPos / 8;
				dstBitByteBitPos = 7-(dstBitPos % 8);

				writeBytePtr = lpDst + dstBitBytePos;
				*writeBytePtr |= srcBit << dstBitByteBitPos;
			}
		}
		break;

	case 4:
	case 8:
		// TODO implement transform formula
		break;

	case 24:
	case 32:
		for(y=0;y<bmSrc.bmHeight;y++)
		{
			for(x=0;x<bmSrc.bmWidth;x++)
			{
				memcpy(lpDst+(bmDst.bmWidthBytes*x + (bmDst.bmWidth-1-y)*3),lpSrc+ bmSrc.bmWidthBytes*y + x*3,3);
			}
		}
		break;
	default:
		break;
	}

	return 0;
}