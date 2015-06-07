// BmpViewerSDI.cpp : �R���\�[�� �A�v���P�[�V�����̃G���g�� �|�C���g���`���܂��B
//

#include "stdafx.h"
#include "BmpViewerSDI.h"

#include <stdio.h>
#include <commctrl.h>
#include <commdlg.h>


#define     DELDC(p)   { if (p) { DeleteDC (p);   (p)=NULL; } }
#define     DELOBJ(p)  { if (p) { DeleteObject(p); (p)=NULL; } }

#define MAX_LOADSTRING 100


// �O���[�o���ϐ� :
HINSTANCE hInst;								// ���݂̃C���^�[�t�F�C�X
TCHAR szTitle[MAX_LOADSTRING];					// �^�C�g�� �o�[�̃e�L�X�g
TCHAR szWindowClass[MAX_LOADSTRING];			// ���C�� �E�B���h�E �N���X��

CBmpObj cbo;
CBmpObj cboRot;

CBmpObj cbmpObj[4];


TCHAR				szFileName[MAX_PATH];
POINTS				prevPoint, Point;
POINTS				topleftPoint={0, 0};
//DWORD				displayFlag=IDM_ORIGSIZE;
HMENU				hMenuCB;
HWND				hwndMain;

HBITMAP				hBitmap        = NULL;
HBITMAP				hBitmapRot     = NULL;
HBITMAP				hBitmapCurrent = NULL;
HBITMAP             hBackBitmap    = NULL;
HDC                 hdcMem         = NULL;
HDC                 hdcMem2        = NULL;

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

	wcex.style			= CS_HREDRAW | CS_VREDRAW | CS_DBLCLKS;
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

				DELOBJ(hBitmapRot);
				first_rotate = TRUE;
				bOpenFile = TRUE;

#if 1
				TCHAR			szDefaultDir[MAX_PATH];

				wsprintf(szDefaultDir, _T("\\My Documents"));

				if ( !CallOpenDialog(hWnd, &Ofn, szDefaultDir ) ){
					// �G���[�܂��̓L�����Z�����̏���
					break;
				}

				wsprintf(szFileName, _T("%s"), Ofn.lpstrFile);
#else
				wsprintf(szFileName, _T("F:\\work\\Program\\Prog\\BmpViewerSDI\\3692796\.bmp"));
#endif

				cbo.LoadBitmapFromFile(szFileName);

				if( cbo.GetBitmapHandle() != NULL )
				{
					SelectObject( hdcMem, cbo.GetBitmapHandle() );
					hBitmapCurrent = cbo.GetBitmapHandle();
				}

				{
					RECT rt;

					GetClientRect(hWnd, &rt);
					InvalidateRect(hWnd, &rt, FALSE);
				}

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

		if(hBitmapCurrent != NULL)
		{
			BITMAP bm;
			BOOL ret;
			RECT rt;

			GetObject(hBitmapCurrent, sizeof(BITMAP), &bm);

			GetClientRect(hWnd, &rt);

			// �o�b�t�@��ŕ`��i�w�i�͔��Œ�j
			ret = FillRect(hdcMem2,&rt,(HBRUSH)GetStockObject(WHITE_BRUSH));
			ret = BitBlt (hdcMem2, topleftPoint.x, topleftPoint.y, bm.bmWidth, bm.bmHeight, hdcMem,  0, 0, SRCCOPY);

			// �]��
			ret = BitBlt (hdc, 0, 0, rt.right, rt.bottom, hdcMem2,  0, 0, SRCCOPY);
		}

		EndPaint(hWnd, &ps);
		break;
	case WM_LBUTTONDOWN:
		prevPoint = MAKEPOINTS(lParam);
		bDragging = TRUE;
		break;
	case WM_LBUTTONUP:
		bDragging = FALSE;
		break;
	case WM_RBUTTONUP:
		if(cbo.GetBitmapHandle() != NULL)
		{
			LPVOID lp_void;
			LPBYTE lpSrc, lpDst;
			BITMAP bmSrc, bmDst;
			RECT rt;
			int x,y;
			HDC hdc;

			HANDLE hMem;
			DWORD  dwHeaderSize;
			LPBITMAPINFO lpBmi;

			hdc = GetDC(hWnd);

			int srcBitPos, srcBitBytePos, srcBitByteBitPos;
			int dstBitPos, dstBitBytePos, dstBitByteBitPos;
			int srcBit;
			LPBYTE writeBytePtr;

			if(first_rotate==TRUE)
			{
				GetObject(cbo.GetBitmapHandle(), sizeof(BITMAP), &bmSrc);
				lpSrc = (LPBYTE)bmSrc.bmBits;

				// ��90�x��]�����摜�p��BITMAP����
				{
					dwHeaderSize = sizeof(BITMAPINFOHEADER)+ sizeof(RGBQUAD)*cbo.GetBitmapInfoHeader().biClrUsed;
					hMem = GlobalAlloc(GHND, dwHeaderSize);
					lpBmi = (LPBITMAPINFO)GlobalLock(hMem);

					ZeroMemory(lpBmi, dwHeaderSize);

					lpBmi->bmiHeader = cbo.GetBitmapInfoHeader();

					// �c������ւ�
					lpBmi->bmiHeader.biWidth     = bmSrc.bmHeight;
					lpBmi->bmiHeader.biHeight    = bmSrc.bmWidth;

					memcpy(lpBmi->bmiColors, cbo.GetRgbQuadData(), sizeof(RGBQUAD)*cbo.GetBitmapInfoHeader().biClrUsed);

#if 0
					if( cbo.hPalette != NULL )
					{
						SelectPalette(hdc, cbo.hPalette, FALSE);
						RealizePalette(hdc);
					}
#endif

					cboRot.CreateBitmap(lpBmi, dwHeaderSize);

//					hBitmapRot = CreateDIBSection(hdc, lpBmi, DIB_RGB_COLORS, &lp_void, NULL, 0);
//					hBitmapRot = CreateDIBSection(NULL, lpBmi, DIB_RGB_COLORS, &lp_void, NULL, 0);
//					hBitmapRot = CreateDIBSection(hdc, (LPBITMAPINFO)&bmi, DIB_RGB_COLORS, &lp_void, NULL, 0);
//					hBitmapRot = CreateDIBSection(hdc, (LPBITMAPINFO)&bmi, DIB_PAL_COLORS, &lp_void, NULL, 0);

					GlobalUnlock(hMem);
					GlobalFree(hMem);
				}

//				GetObject(hBitmapRot, sizeof(BITMAP), &bmDst);
//				lpDst = (LPBYTE)lp_void;
				GetObject(cboRot.GetBitmapHandle(), sizeof(BITMAP), &bmDst);
				lpDst = (LPBYTE)bmDst.bmBits;

				switch ( bmDst.bmBitsPixel )
				{
				case 1:
					ZeroMemory(lpDst, bmDst.bmWidthBytes*bmDst.bmHeight);
					
					// ��90�x��]�ϊ�
					for(y=0;y<bmSrc.bmHeight;y++)
					{
						for(x=0;x<bmSrc.bmWidth;x++)
						{
							srcBitPos        = bmSrc.bmWidthBytes*8*y+x;
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
					break;

				case 24:
				case 32:
					for(y=0;y<bmSrc.bmHeight;y++)
					{
						for(x=0;x<bmSrc.bmWidth;x++)
						{
							memcpy(lpDst+((bmSrc.bmHeight *x + bmSrc.bmHeight-1-y)*3),lpSrc+ bmSrc.bmWidthBytes*y + x*3,3);
						}
					}
					break;
				default:
					break;
				}

				first_rotate = FALSE;
			}

			hBitmapCurrent = (hBitmapCurrent == cbo.GetBitmapHandle())?cboRot.GetBitmapHandle():cbo.GetBitmapHandle();
			SelectObject( hdcMem, hBitmapCurrent );

			ReleaseDC(hWnd, hdc);

			GetClientRect(hWnd, &rt);
			InvalidateRect(hWnd, &rt, FALSE);
		}
		break;
	case WM_MOUSEMOVE:
		if( bDragging )
		{
#if 1
			if(cbo.GetBitmapHandle() != NULL)
			{
				Point = MAKEPOINTS(lParam);

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

			prevPoint = Point;
		}
		break;

	case WM_LBUTTONDBLCLK:
		topleftPoint = MAKEPOINTS(lParam);

		{
			RECT rt;

			GetClientRect(hWnd, &rt);
			InvalidateRect(hWnd, &rt, FALSE);
		}
		 
		break;

#if 0
	case WM_ERASEBKGND:
		return TRUE;
#endif
	case WM_SIZE:
		DELOBJ(hBackBitmap);
		DELDC(hdcMem2);

		{
			HDC     hdc;
			RECT rt;

			GetClientRect(hWnd, &rt);

			hdc         = GetDC(hWnd);
			hdcMem2     = CreateCompatibleDC(hdc);
			hBackBitmap = CreateCompatibleBitmap(hdc, rt.right, rt.bottom);
			SelectObject( hdcMem2, hBackBitmap );

			ReleaseDC(hWnd, hdc);
		}
		break;
	case WM_DESTROY:
		DELOBJ(hBitmapRot);
		DELOBJ(hBackBitmap);

		DELDC(hdcMem);
		DELDC(hdcMem2);

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
	hdcMem2     = CreateCompatibleDC(hdc);
	hBackBitmap = CreateCompatibleBitmap(hdc, rt.right, rt.bottom);
	SelectObject( hdcMem2, hBackBitmap );

	ReleaseDC(hWnd, hdc);
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
