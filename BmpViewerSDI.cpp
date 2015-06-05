// BmpViewerSDI.cpp : コンソール アプリケーションのエントリ ポイントを定義します。
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


// グローバル変数 :
HINSTANCE hInst;								// 現在のインターフェイス
TCHAR szTitle[MAX_LOADSTRING];					// タイトル バーのテキスト
TCHAR szWindowClass[MAX_LOADSTRING];			// メイン ウィンドウ クラス名

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


// このコード モジュールに含まれる関数の宣言を転送します :
ATOM				MyRegisterClass(HINSTANCE hInstance);
BOOL				InitInstance(HINSTANCE, int);
LRESULT CALLBACK	WndProc(HWND, UINT, WPARAM, LPARAM);
LRESULT CALLBACK	About(HWND, UINT, WPARAM, LPARAM);
// 以下自作関数の宣言
BOOL				CallOpenDialog( HWND, LPOPENFILENAME, LPTSTR );
void  CreateBuf(HWND hWnd);
void  Render(void);



int APIENTRY _tWinMain(HINSTANCE hInstance,
                     HINSTANCE hPrevInstance,
                     LPTSTR    lpCmdLine,
                     int       nCmdShow)
{
 	// TODO: ここにコードを挿入してください。
	MSG msg;
	HACCEL hAccelTable;

	// グローバル文字列を初期化しています。
	LoadString(hInstance, IDS_APP_TITLE, szTitle, MAX_LOADSTRING);
	LoadString(hInstance, IDC_BMPVIEWERSDI, szWindowClass, MAX_LOADSTRING);
	MyRegisterClass(hInstance);

	// アプリケーションの初期化を実行します。:
	if (!InitInstance (hInstance, nCmdShow)) 
	{
		return FALSE;
	}

	hAccelTable = LoadAccelerators(hInstance, (LPCTSTR)IDC_BMPVIEWERSDI);

	// メイン メッセージ ループ :
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
//  関数 : MyRegisterClass()
//
//  目的 : ウィンドウ クラスを登録します。
//
//  コメント :
//
//    この関数および使い方は、'RegisterClassEx' 関数が追加された
//     Windows 95 より前の Win32 システムと互換させる場合にのみ必要です。
//    アプリケーションが、関連付けられた 
//    正しい形式の小さいアイコンを取得できるようにするには、
//    この関数を呼び出してください。
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
//   関数 : InitInstance(HANDLE, int)
//
//   目的 : インスタンス ハンドルを保存して、メイン ウィンドウを作成します。
//
//   コメント :
//
//        この関数で、グローバル変数でインスタンス ハンドルを保存し、
//        メイン プログラム ウィンドウを作成および表示します。
//
BOOL InitInstance(HINSTANCE hInstance, int nCmdShow)
{
   HWND hWnd;

   hInst = hInstance; // グローバル変数にインスタンス処理を格納します。

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
//  関数 : WndProc(HWND, unsigned, WORD, LONG)
//
//  目的 :  メイン ウィンドウのメッセージを処理します。
//
//  WM_COMMAND	- アプリケーション メニューの処理
//  WM_PAINT	- メイン ウィンドウの描画
//  WM_DESTROY	- 中止メッセージを表示して戻る
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
		// 選択されたメニューの解析 :
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
					// エラーまたはキャンセル時の処理
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
		// TODO: 描画コードをここに追加してください...
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
			LPBYTE lpSrc, lpDst;
			LPVOID lp_void;
			BITMAP bmSrc, bmDst;
			RECT rt;
			int x,y;
			HBITMAP swapbuf;
			HDC hdc;

			int i;
			HANDLE hMem;
			LPBITMAPINFO lpBmi;

			hdc = GetDC(hWnd);

			int srcBitPos, srcBitBytePos, srcBitByteBitPos;
			int dstBitPos, dstBitBytePos, dstBitByteBitPos;
			int srcBit;
			LPBYTE writeBytePtr;

			if(first_rotate==TRUE)
			{
				GetObject(cbo.hBitmap, sizeof(BITMAP), &bmSrc);
				lpSrc = (LPBYTE)bmSrc.bmBits; // lpはビットイメージの先頭を指す

				// 左90度回転した画像用のBITMAP生成
				{
					BITMAPINFOHEADER bmiHeader;
					BITMAPINFO       bmi;

					hMem = GlobalAlloc(GHND, sizeof(BITMAPINFOHEADER)+(sizeof(RGBQUAD)*(1<<bmSrc.bmBitsPixel)));
					lpBmi = (LPBITMAPINFO)GlobalLock(hMem);

					ZeroMemory(&bmiHeader, sizeof(BITMAPINFOHEADER));
					bmiHeader.biSize      = sizeof(BITMAPINFOHEADER);
					bmiHeader.biWidth     = bmSrc.bmHeight;  // 縦横入れ替え
					bmiHeader.biHeight    = bmSrc.bmWidth;   // 縦横入れ替え
					bmiHeader.biPlanes    = bmSrc.bmPlanes;
					bmiHeader.biBitCount  = bmSrc.bmBitsPixel;


#if 1
					lpBmi->bmiHeader = bmiHeader;
					lpBmi->bmiColors[0].rgbBlue  = 0x00;
					lpBmi->bmiColors[0].rgbGreen = 0x00;
					lpBmi->bmiColors[0].rgbRed   = 0x00;
					lpBmi->bmiColors[1].rgbBlue  = 0xFF;
					lpBmi->bmiColors[1].rgbGreen = 0xFF;
					lpBmi->bmiColors[1].rgbRed   = 0xFF;

#else
					bmi.bmiHeader = bmiHeader;
					bmi.bmiColors[0].rgbBlue  = 0x00;
					bmi.bmiColors[0].rgbGreen = 0x00;
					bmi.bmiColors[0].rgbRed   = 0x00;
					bmi.bmiColors[1].rgbBlue  = 0xFF;
					bmi.bmiColors[1].rgbGreen = 0xFF;
					bmi.bmiColors[1].rgbRed   = 0xFF;
#endif

//					hBitmapRot = CreateDIBSection(hdc, lpBmi, DIB_RGB_COLORS, &lp_void, NULL, 0);
					hBitmapRot = CreateDIBSection(NULL, lpBmi, DIB_RGB_COLORS, &lp_void, NULL, 0);
//					hBitmapRot = CreateDIBSection(hdc, (LPBITMAPINFO)&bmi, DIB_RGB_COLORS, &lp_void, NULL, 0);
//					hBitmapRot = CreateDIBSection(hdc, (LPBITMAPINFO)&bmi, DIB_PAL_COLORS, &lp_void, NULL, 0);

					GlobalUnlock(lpBmi);
					GlobalFree(hMem);
				}

				GetObject(hBitmapRot, sizeof(BITMAP), &bmDst);
				lpDst = (LPBYTE)lp_void;


				switch ( bmDst.bmBitsPixel )
				{
				case 1:
					ZeroMemory(lpDst, bmDst.bmWidthBytes*bmDst.bmHeight);
					
					//memset(lpDst, 0x00, bmDst.bmWidthBytes*bmDst.bmHeight);
					for(y=0;y<bmSrc.bmHeight;y++)
					{
						for(x=0;x<bmSrc.bmWidth;x++)
						{
#if 1
							srcBitPos        = bmSrc.bmWidthBytes*8*y+x;
							srcBitBytePos    = srcBitPos / 8;
							srcBitByteBitPos = 7-(srcBitPos % 8);
							srcBit = ( (*(lpSrc+srcBitBytePos)) >> srcBitByteBitPos ) & 0x1;

							dstBitPos        = bmDst.bmWidthBytes*8*x + bmDst.bmWidth-1-y;
							dstBitBytePos    = dstBitPos / 8;
							dstBitByteBitPos = 7-(dstBitPos % 8);

							writeBytePtr = lpDst + dstBitBytePos;

							*writeBytePtr |= srcBit <<dstBitByteBitPos;
#endif
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
							// 左90度変換式（24bitカラー決め打ち）
							memcpy(lpDst+((bmSrc.bmHeight *x + bmSrc.bmHeight-1-y)*3),lpSrc+ bmSrc.bmWidthBytes*y + x*3,3);
						}
					}
					break;
				default:
					break;
				}

				first_rotate = FALSE;
			}

			swapbuf = cbo.hBitmap;
			cbo.hBitmap = hBitmapRot;
			hBitmapRot = swapbuf;

			ReleaseDC(hWnd, hdc);

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
	case WM_SIZE:
		SAFE_DELOBJ(hBackBitmap);
		SAFE_DELDC(hdcMem2);

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

// デバイスコンテキストのバッファ作成と関連付け
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

// バージョン情報ボックスのメッセージ ハンドラです。
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
//	「Open...」ダイアログ
//---------------------------------------------------------------------------------------------
BOOL CallOpenDialog( HWND hWnd, LPOPENFILENAME lpOfn, LPTSTR szInitialDir )
{
    OPENFILENAME	ofn;
    static TCHAR	szFileName[MAX_PATH];
	static TCHAR	szFileTitle[MAX_PATH];
	static TCHAR	szFilter[]=_T("bmpﾌｧｲﾙ(*.bmp)\0*.bmp\0全てのﾌｧｲﾙ(*.*)\0*.*\0\0");

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

    //  「名前を付けて保存」ダイアログの起動(エラーチェック付き)
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
