// BmpViewerSDI.cpp : �R���\�[�� �A�v���P�[�V�����̃G���g�� �|�C���g���`���܂��B
//

#include "stdafx.h"
#include "BmpViewerSDI.h"

#include <stdio.h>
#include <commctrl.h>
#include <commdlg.h>


#include    <windows.h>
#include    <mmsystem.h>
#pragma     comment(lib,"winmm.lib")
#define     SAFE_DELETE(p)  { if (p) { delete (p); (p)=NULL; } }
#define     SAFE_DELDC(p)   { if (p) { DeleteDC (p);   (p)=NULL; } }
#define     SAFE_DELOBJ(p)  { if (p) { DeleteObject(p); (p)=NULL; } }

HBITMAP     hBackBitmap=NULL;   //Back Buffer HBitMap
HDC         hdcMem=NULL;
HDC         hdcMem2=NULL;

#define MAX_LOADSTRING 100


// �O���[�o���ϐ� :
HINSTANCE hInst;								// ���݂̃C���^�[�t�F�C�X
TCHAR szTitle[MAX_LOADSTRING];					// �^�C�g�� �o�[�̃e�L�X�g
TCHAR szWindowClass[MAX_LOADSTRING];			// ���C�� �E�B���h�E �N���X��

CBmpObj cbo;

TCHAR				szFileName[MAX_PATH];
POINT				prevPoint, Point;
POINT				topleftPoint={0, 0};
BITMAPFILEHEADER	bmpFileHeader;
BITMAPINFOHEADER	bmpInfoHeader;
//DWORD				displayFlag=IDM_ORIGSIZE;
HMENU				hMenuCB;
HWND				hwndMain;
HBITMAP				hBitmap=NULL, hBmpOld;
HBITMAP				hBitmapRot=NULL;

int                 first_rotate = TRUE;


// ���̃R�[�h ���W���[���Ɋ܂܂��֐��̐錾��]�����܂� :
ATOM				MyRegisterClass(HINSTANCE hInstance);
BOOL				InitInstance(HINSTANCE, int);
LRESULT CALLBACK	WndProc(HWND, UINT, WPARAM, LPARAM);
LRESULT CALLBACK	About(HWND, UINT, WPARAM, LPARAM);
// �ȉ�����֐��̐錾
BOOL				CallOpenDialog( HWND, LPOPENFILENAME, LPTSTR );
void  CreateBuf(HWND hWnd);
void  Render(void);



int APIENTRY _tWinMain(HINSTANCE hInstance,
                     HINSTANCE hPrevInstance,
                     LPTSTR    lpCmdLine,
                     int       nCmdShow)
{
 	// TODO: �����ɃR�[�h��}�����Ă��������B
	MSG msg;
	HACCEL hAccelTable;

	// �O���[�o������������������Ă��܂��B
	LoadString(hInstance, IDS_APP_TITLE, szTitle, MAX_LOADSTRING);
	LoadString(hInstance, IDC_BMPVIEWERSDI, szWindowClass, MAX_LOADSTRING);
	MyRegisterClass(hInstance);

	// �A�v���P�[�V�����̏����������s���܂��B:
	if (!InitInstance (hInstance, nCmdShow)) 
	{
		return FALSE;
	}

	hAccelTable = LoadAccelerators(hInstance, (LPCTSTR)IDC_BMPVIEWERSDI);

	// ���C�� ���b�Z�[�W ���[�v :
	while (GetMessage(&msg, NULL, 0, 0)) 
	{
		if (!TranslateAccelerator(msg.hwnd, hAccelTable, &msg)) 
		{
			TranslateMessage(&msg);
			DispatchMessage(&msg);
		}
	}

	return (int) msg.wParam;
}



//
//  �֐� : MyRegisterClass()
//
//  �ړI : �E�B���h�E �N���X��o�^���܂��B
//
//  �R�����g :
//
//    ���̊֐�����юg�����́A'RegisterClassEx' �֐����ǉ����ꂽ
//     Windows 95 ���O�� Win32 �V�X�e���ƌ݊�������ꍇ�ɂ̂ݕK�v�ł��B
//    �A�v���P�[�V�������A�֘A�t����ꂽ 
//    �������`���̏������A�C�R�����擾�ł���悤�ɂ���ɂ́A
//    ���̊֐����Ăяo���Ă��������B
//
ATOM MyRegisterClass(HINSTANCE hInstance)
{
	WNDCLASSEX wcex;

	wcex.cbSize = sizeof(WNDCLASSEX); 

	wcex.style			= CS_HREDRAW | CS_VREDRAW;
	wcex.lpfnWndProc	= (WNDPROC)WndProc;
	wcex.cbClsExtra		= 0;
	wcex.cbWndExtra		= 0;
	wcex.hInstance		= hInstance;
	wcex.hIcon			= LoadIcon(hInstance, (LPCTSTR)IDI_BMPVIEWERSDI);
	wcex.hCursor		= LoadCursor(NULL, IDC_ARROW);
	wcex.hbrBackground	= (HBRUSH)(COLOR_WINDOW+1);
	wcex.lpszMenuName	= (LPCTSTR)IDC_BMPVIEWERSDI;
	wcex.lpszClassName	= szWindowClass;
	wcex.hIconSm		= LoadIcon(wcex.hInstance, (LPCTSTR)IDI_SMALL);

	return RegisterClassEx(&wcex);
}

//
//   �֐� : InitInstance(HANDLE, int)
//
//   �ړI : �C���X�^���X �n���h����ۑ����āA���C�� �E�B���h�E���쐬���܂��B
//
//   �R�����g :
//
//        ���̊֐��ŁA�O���[�o���ϐ��ŃC���X�^���X �n���h����ۑ����A
//        ���C�� �v���O���� �E�B���h�E���쐬����ѕ\�����܂��B
//
BOOL InitInstance(HINSTANCE hInstance, int nCmdShow)
{
   HWND hWnd;

   hInst = hInstance; // �O���[�o���ϐ��ɃC���X�^���X�������i�[���܂��B

   hWnd = CreateWindow(szWindowClass, szTitle, WS_OVERLAPPEDWINDOW,
      CW_USEDEFAULT, 0, CW_USEDEFAULT, 0, NULL, NULL, hInstance, NULL);

   if (!hWnd)
   {
      return FALSE;
   }

   ShowWindow(hWnd, nCmdShow);
   UpdateWindow(hWnd);

   return TRUE;
}

//
//  �֐� : WndProc(HWND, unsigned, WORD, LONG)
//
//  �ړI :  ���C�� �E�B���h�E�̃��b�Z�[�W���������܂��B
//
//  WM_COMMAND	- �A�v���P�[�V���� ���j���[�̏���
//  WM_PAINT	- ���C�� �E�B���h�E�̕`��
//  WM_DESTROY	- ���~���b�Z�[�W��\�����Ė߂�
//
//
LRESULT CALLBACK WndProc(HWND hWnd, UINT message, WPARAM wParam, LPARAM lParam)
{
	int wmId, wmEvent;
	PAINTSTRUCT ps;
	HDC hdc;
	static OPENFILENAME	Ofn;
	static int bOpenFile = FALSE;
	static int bDragging = FALSE;

	switch (message) 
	{
	case WM_CREATE:
		CreateBuf(hWnd);
		break;
	case WM_COMMAND:
		wmId    = LOWORD(wParam); 
		wmEvent = HIWORD(wParam); 
		// �I�����ꂽ���j���[�̉�� :
		switch (wmId)
		{
		case IDM_ABOUT:
			DialogBox(hInst, (LPCTSTR)IDD_ABOUTBOX, hWnd, (DLGPROC)About);
			break;
		case IDM_EXIT:
			DestroyWindow(hWnd);
			break;
		case IDM_OPEN:
			{
				topleftPoint.x = 0;
				topleftPoint.y = 0;

				TCHAR			szDefaultDir[MAX_PATH];
				RECT rt;

				wsprintf(szDefaultDir, _T("\\My Documents"));

				if ( !CallOpenDialog(hWnd, &Ofn, szDefaultDir ) ){
					// �G���[�܂��̓L�����Z�����̏���
					break;
				}

				if( bOpenFile == TRUE )
				{
					SAFE_DELOBJ(cbo.hBitmap);
					SAFE_DELOBJ(hBitmapRot);
				}
				first_rotate = TRUE;
				bOpenFile = TRUE;

				wsprintf(szFileName, _T("%s"), Ofn.lpstrFile);
				cbo.LoadBitmapFromFile(szFileName);

				hBmpOld = (HBITMAP)SelectObject( hdcMem, cbo.hBitmap );

				GetClientRect(hWnd, &rt);
				InvalidateRect(hWnd, &rt, FALSE);

			}
		default:
			return DefWindowProc(hWnd, message, wParam, lParam);
		}
		break;
	case WM_PAINT:
		hdc = BeginPaint(hWnd, &ps);
		// TODO: �`��R�[�h�������ɒǉ����Ă�������...
		if( !bOpenFile )
		{
			break;
		}

		if(cbo.hBitmap != NULL)
		{
			BITMAP bm;
			BOOL ret;
			RECT rt;

			GetClientRect(hWnd, &rt);
			GetObject(cbo.hBitmap, sizeof(BITMAP), &bm);

			hBmpOld = (HBITMAP)SelectObject( hdcMem, cbo.hBitmap );

			ret = FillRect(hdcMem2,&rt,(HBRUSH)GetStockObject(WHITE_BRUSH));
			ret = BitBlt (hdcMem2, topleftPoint.x, topleftPoint.y, bm.bmWidth, bm.bmHeight, hdcMem,  0, 0, SRCCOPY);
			ret = BitBlt (hdc, 0, 0, rt.right, rt.bottom, hdcMem2,  0, 0, SRCCOPY);

			SelectObject( hdcMem, hBmpOld );
		}

		EndPaint(hWnd, &ps);
		break;
	case WM_LBUTTONDOWN:
		prevPoint.x = LOWORD(lParam);
		prevPoint.y = HIWORD(lParam);
		bDragging = TRUE;
		break;
	case WM_LBUTTONUP:
		bDragging = FALSE;
		break;
	case WM_RBUTTONUP:
		if(cbo.hBitmap != NULL)
		{
			LPBYTE lp, lp_;
			BITMAP bm;
			RECT rt;
			int origWidth, origHeight;
			int x,y;
			HBITMAP swapbuf;
			static HDC hdcBuffer;

			if(first_rotate==TRUE)
			{
				GetObject(cbo.hBitmap, sizeof(BITMAP), &bm);
				lp = (LPBYTE)bm.bmBits; // lp�̓r�b�g�C���[�W�̐擪���w�����ƂɂȂ�

				hdc = GetDC(hWnd);

				// ��90�x��]�����摜�p��BITMAP����
				{
					BITMAPINFOHEADER bmiHeader;
					LPVOID           lp;
					BITMAPINFO       bmi;

					ZeroMemory(&bmiHeader, sizeof(BITMAPINFOHEADER));
					bmiHeader.biSize      = sizeof(BITMAPINFOHEADER);
					bmiHeader.biWidth     = bm.bmHeight;  // �c������ւ�
					bmiHeader.biHeight    = bm.bmWidth;   // �c������ւ�
					bmiHeader.biPlanes    = bm.bmPlanes;
					bmiHeader.biBitCount  = bm.bmBitsPixel;

					bmi.bmiHeader = bmiHeader;
					lp = (LPBYTE)bm.bmBits;

					hBitmapRot = CreateDIBSection(NULL, (LPBITMAPINFO)&bmi, DIB_RGB_COLORS, &lp, NULL, 0);
				}
				hdcBuffer = CreateCompatibleDC(hdc);

				GetObject(cbo.hBitmap, sizeof(BITMAP), &bm);
				origWidth  = bm.bmWidth;
				origHeight = bm.bmHeight;

				GetObject(hBitmapRot, sizeof(BITMAP), &bm);
				lp_ = (LPBYTE)bm.bmBits;
				for(y=0;y<origHeight;y++)
				{
					for(x=0;x<origWidth;x++)
					{
						// ��90�x�ϊ����i24bit�J���[���ߑł��j
						memcpy(lp_+((origHeight*x + origHeight-1-y)*3),lp+((origWidth*y+x)*3),3);
					}
				}

				SelectObject(hdcBuffer, hBitmapRot);

				GetClientRect(hWnd, &rt);

				DeleteDC(hdcBuffer);
				ReleaseDC(hWnd, hdc);

				first_rotate = FALSE;
			}

			swapbuf = cbo.hBitmap;
			cbo.hBitmap = hBitmapRot;
			hBitmapRot = swapbuf;

			GetClientRect(hWnd, &rt);
			InvalidateRect(hWnd, &rt, FALSE);
		}
		break;
	case WM_MOUSEMOVE:
		if( bDragging )
		{
#if 1
			if(cbo.hBitmap != NULL)
			{
				Point.x = LOWORD(lParam);
				Point.y = HIWORD(lParam);

				topleftPoint.x += Point.x - prevPoint.x;
				topleftPoint.y += Point.y - prevPoint.y;

#if 0
				{
					HDC hdc;
					hdc = GetDC(hWnd);
					TCHAR buf[100];
					_stprintf(buf, _T("mouse point \"x: %d   y: %d\""), LOWORD(lParam), HIWORD(lParam));
					DrawText(hdc, buf, -1, &rt, DT_LEFT);
					ReleaseDC(hWnd, hdc);
				}
#endif
				{
					RECT rt;

					GetClientRect(hWnd, &rt);
					InvalidateRect(hWnd, &rt, FALSE);
				}

			}
#endif
			prevPoint.x = Point.x;
			prevPoint.y = Point.y;
		}
		break;
#if 0
	case WM_ERASEBKGND:
		return TRUE;
#endif
	case WM_DESTROY:
		SAFE_DELOBJ(cbo.hBitmap);
		SAFE_DELOBJ(hBitmapRot);
		SAFE_DELOBJ(hBackBitmap);

		SAFE_DELDC(hdcMem);
		SAFE_DELDC(hdcMem2);

		PostQuitMessage(0);
		break;
	default:
		return DefWindowProc(hWnd, message, wParam, lParam);
	}
	return 0;
}

// �f�o�C�X�R���e�L�X�g�̃o�b�t�@�쐬�Ɗ֘A�t��
void  CreateBuf(HWND hWnd)
{
	HDC     hdc;
	RECT rt;

	GetClientRect(hWnd, &rt);

	hdc         = GetDC(hWnd);
	hdcMem      = CreateCompatibleDC(hdc);
	hdcMem2     = CreateCompatibleDC(hdcMem);
	hBackBitmap = CreateCompatibleBitmap(hdc, rt.right, rt.bottom);
	SelectObject( hdcMem2, hBackBitmap );
}

// �o�[�W�������{�b�N�X�̃��b�Z�[�W �n���h���ł��B
LRESULT CALLBACK About(HWND hDlg, UINT message, WPARAM wParam, LPARAM lParam)
{
	switch (message)
	{
	case WM_INITDIALOG:
		return TRUE;

	case WM_COMMAND:
		if (LOWORD(wParam) == IDOK || LOWORD(wParam) == IDCANCEL) 
		{
			EndDialog(hDlg, LOWORD(wParam));
			return TRUE;
		}
		break;
	}
	return FALSE;
}

//---------------------------------------------------------------------------------------------
//	�uOpen...�v�_�C�A���O
//---------------------------------------------------------------------------------------------
BOOL CallOpenDialog( HWND hWnd, LPOPENFILENAME lpOfn, LPTSTR szInitialDir )
{
    OPENFILENAME	ofn;
    static TCHAR	szFileName[MAX_PATH];
	static TCHAR	szFileTitle[MAX_PATH];
	static TCHAR	szFilter[]=_T("bmp̧��(*.bmp)\0*.bmp\0�S�Ă�̧��(*.*)\0*.*\0\0");

    memset( &ofn,  0x00, sizeof(OPENFILENAME) );
    memset( lpOfn, 0x00, sizeof(OPENFILENAME) );
	memset( szFileName,  0x00, sizeof(szFileName) );
	memset( szFileTitle, 0x00, sizeof(szFileTitle) );

//	SetDefaultFileName(hWnd, szFileName, szInitialDir);

    ofn.lStructSize		= sizeof(OPENFILENAME);
    ofn.hwndOwner		= hWnd;
    ofn.lpstrFilter		= szFilter;
	ofn.nFilterIndex	= 1;
    ofn.lpstrFile		= szFileName;
	ofn.nMaxFile		= MAX_PATH;
    ofn.lpstrFileTitle	= szFileTitle;
    ofn.nMaxFileTitle	= MAX_PATH;
	ofn.lpstrInitialDir	= szInitialDir;
	ofn.lpstrTitle		= _T("Open...");
    ofn.Flags			= OFN_FILEMUSTEXIST | OFN_HIDEREADONLY ;
	ofn.lpstrDefExt		= NULL;

    //  �u���O��t���ĕۑ��v�_�C�A���O�̋N��(�G���[�`�F�b�N�t��)
    if( GetOpenFileName( &ofn ) == FALSE ){
		switch(GetLastError()){
		case ERROR_INVALID_PARAMETER:
			MessageBox(hWnd, _T("ERROR_INVALID_PARAMETER"), _T(""), MB_OK | MB_ICONERROR);
			break;
		case ERROR_OUTOFMEMORY:
			MessageBox(hWnd, _T("ERROR_OUTOFMEMORY"), _T(""), MB_OK | MB_ICONERROR);
			break;
		}

		return FALSE;
	}

	*lpOfn = ofn;
	return TRUE;
}
