#include "common.h"
#include "bmpObj.h"


#define	DP_biBitCount		24


//--------------------------------------------------------------------------------
//--------------------------------------------------------------------------------
CBmpObj::CBmpObj()
{
	hBitmap     = NULL;
	hPalette    = NULL;
	lpRgbQuad   = NULL;
	hMemRgbQuad = NULL;
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
int CBmpObj::ReadHeader(LPTSTR szFileName)
{
	int ret=0;
    HANDLE hFile = NULL;
    DWORD dwResult;
    LPBITMAPFILEHEADER lpBfh;
    LPBITMAPINFOHEADER lpBih;

    hFile = CreateFile(szFileName, GENERIC_READ, 0, NULL, OPEN_EXISTING, FILE_ATTRIBUTE_NORMAL, NULL);
    if (hFile == INVALID_HANDLE_VALUE) {
        ret = -1;
		goto RETURN;
    }

	lpBfh = (LPBITMAPFILEHEADER)(&this->bmpFileHeader);
    ReadFile(hFile, (LPBITMAPFILEHEADER)lpBfh, sizeof(BITMAPFILEHEADER), &dwResult, NULL);

	if ( !(LOBYTE(lpBfh->bfType) == 'B' && HIBYTE(lpBfh->bfType) == 'M') ) {
        ret= -2;
		goto RETURN;
    }

	lpBih = (LPBITMAPINFOHEADER)(&this->bmpInfoHeader);
    ReadFile(hFile, (LPBITMAPINFOHEADER)lpBih, sizeof(BITMAPINFOHEADER), &dwResult, NULL);

	if( lpBih->biSize != 40 ) // Windows Bitmapのみ対応
	{
        ret= -3;
		goto RETURN;
	}

    // RGBQUADデータ用バッファ確保＆読み込み
	hMemRgbQuad = GlobalAlloc(GMEM_FIXED, lpBih->biClrUsed * sizeof(RGBQUAD));
	lpRgbQuad = (LPRGBQUAD)GlobalLock(hMemRgbQuad);

	SetFilePointer(hFile, sizeof(BITMAPFILEHEADER)+sizeof(BITMAPINFOHEADER), 0, FILE_BEGIN);
    ReadFile(hFile, lpRgbQuad, lpBih->biClrUsed * sizeof(RGBQUAD), &dwResult, NULL);

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

	lpBih = &this->bmpInfoHeader;

	if( lpBih->biClrUsed == 0 || lpBih->biBitCount > 8 )
	{
		return NULL;
	}

    hPal = GlobalAlloc(GMEM_FIXED, sizeof(LOGPALETTE) + lpBih->biClrUsed * sizeof(PALETTEENTRY));
    lpPal = (LPLOGPALETTE)GlobalLock(hPal);

    lpPal->palVersion = 0x300;
    lpPal->palNumEntries = (WORD)lpBih->biClrUsed;

    lpRGB = this->lpRgbQuad;

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

	if( hMemRgbQuad != NULL )
	{
	    GlobalUnlock(hMemRgbQuad);
		GlobalFree(hMemRgbQuad);
		hMemRgbQuad = NULL;
	}

	return 0;
}

//--------------------------------------------------------------------------------
// bmpファイルのヘッダ部を除いたファイルサイズ(単位:byte)を返す
// bmpのファイルフォーマット仕様に完全に沿っているわけではない(と思う)ので注意
//--------------------------------------------------------------------------------
DWORD CBmpObj::SizeImage( BITMAPINFOHEADER bih )
{
	DWORD sizeImage;

	if( bih.biSizeImage != 0 ){
		sizeImage = bih.biSizeImage;
	}else{
		sizeImage = (bih.biHeight * bih.biWidth * bih.biBitCount)/8;
	}

	return sizeImage;
}
