#include "common.h"
#include "bmpObj.h"


#define	DP_biBitCount		24


//--------------------------------------------------------------------------------
//--------------------------------------------------------------------------------
CBmpObj::CBmpObj()
{
	hBitmap=NULL;
}

//--------------------------------------------------------------------------------
//--------------------------------------------------------------------------------
CBmpObj::~CBmpObj()
{
	if(hBitmap != NULL){
		DeleteObject(hBitmap);
	}
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
	BITMAPFILEHEADER		bmpFH;		//�ޯ�ϯ��̧��ͯ��
	static struct {
		BITMAPINFOHEADER	bmpIH;		//�ޯ�ϯ��̧��ͯ��
	} bmpinfo;
	TCHAR	BmpFN[ MAX_PATH ];			//̧�ٖ�

	//��ʃT�C�Y���擾
	rectW = GetSystemMetrics( SM_CXSCREEN );
	rectH = GetSystemMetrics( SM_CYSCREEN );

	//�@�r�b�g�}�b�v�t�@�C���w�b�_���쐬
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

	//��ʂ̓��e���擾
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

	// BITMAPFILEHEADER���쐬
    bmpFH.bfType      = 0x4D42;
	bmpFH.bfSize      = sizeof( bmpFH ) + sizeof( bmpinfo ) + ( rectH * SizeWide );
    bmpFH.bfReserved1 = 0;
    bmpFH.bfReserved2 = 0;
    bmpFH.bfOffBits   = sizeof( bmpFH ) + sizeof( bmpinfo );

#if 0
	if( !GetSaveFileNameSetting() )
		goto RETURN;
#endif

	wsprintf(BmpFN, _T("\\My Documents\\bitmap01.bmp"));

	//�r�b�g�}�b�v�t�@�C���ɕۑ�
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
	DWORD	readbytes;
	PBYTE	frameBuffer=NULL;
	DWORD	rowbytes;
	int		i;

	if( hBitmap != NULL ){
		DeleteObject(hBitmap);
	}

#if 1
	hBitmap = (HBITMAP)LoadImage(hInst, szFileName, IMAGE_BITMAP, 0, 0, LR_CREATEDIBSECTION|LR_LOADFROMFILE);
#else
	//�r�b�g�}�b�v���t�@�C������ǂݏo��
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

	// �r�b�g�}�b�v��GDI�I�u�W�F�N�g���쐬
	// �J���[�r�b�g�}�b�v�̃I�u�W�F�N�g������������ �� �ǂ���炱��ł͓��삵�Ȃ��B�����ۗ��B
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
// bmp�t�@�C���̃w�b�_�����������t�@�C���T�C�Y(�P��:byte)��Ԃ�
// bmp�̃t�@�C���t�H�[�}�b�g�d�l�Ɋ��S�ɉ����Ă���킯�ł͂Ȃ�(�Ǝv��)�̂Œ���
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
