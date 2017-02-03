// PathFinder2D.cpp : Defines the entry point for the application.
//
#include "stdafx.h"
#include "resource.h"

#include "stdio.h" //for sprintf
#include "mmsystem.h" //lib: winmm.lib, for playsound

#include "commdlg.h"  //cm dlg

#include <windows.h>								// Header File For Windows

//
#include "PathFinder2D.h"

//
#include "Setup.h" //all the Generic-purpose functions that all algs share.
#include "Generic.h" //generic interface to all Setups

CSetup *Setup=NULL;
CGeneric *Generic=NULL;

//
HDC hdctmp;
HBITMAP hbmtmp;
HANDLE hOld;
BITMAPV4HEADER bi;
LPBYTE pwBits;
int TIMER_RATE;
int MINIMUM_CLIENT_WIDTH;
int MINIMUM_CLIENT_HEIGHT;

//
bool runflag;
bool zoom;
bool presearch_view;
bool drawmode;

//
bool pendown;
bool brushmode;
int elevation;
int lastx, lasty;

int mutex=0;

//
enum
{
	TIMER_ID=1,
		SCREEN_WIDTH=1000,
		SCREEN_HEIGHT=620
};

//
HINSTANCE hInst;
#define MAX_LOADSTRING 100
TCHAR szTitle[MAX_LOADSTRING];
TCHAR szWindowClass[MAX_LOADSTRING];

// Foward declarations of functions included in this code module:
ATOM				MyRegisterClass(HINSTANCE hInstance);
BOOL				InitInstance(HINSTANCE, int);
LRESULT CALLBACK	WndProc(HWND, UINT, WPARAM, LPARAM);
LRESULT CALLBACK	About(HWND, UINT, WPARAM, LPARAM);

///////////////////////////////////////////////////////////////////////////
///////////////////////////////////////////////////////////////////////////
///////////////////////////////////////////////////////////////////////////


//
void RefreshMenu(HWND hWnd)
{
	LARGE_INTEGER freq;     // 64 bit int
	QueryPerformanceFrequency(&freq);     //  counts per second  divisor
	double ticks=1000.0*((double)Setup->bigtick.QuadPart/(double)freq.QuadPart);
	double gticks=1000.0*((double)Generic->bigtick.QuadPart/(double)freq.QuadPart);
	
	char tmp[512];
	sprintf(tmp,"%s  -  AI %d  -  Frame %d  -  Elapsed %f ms - Clock %I64d (Elapsed %f ms - Clock %I64d) %s %s",
		szTitle,
		Generic->ai,
		Setup->frame,
		ticks,
		(_int64)Setup->bigtick.QuadPart,
		gticks,
		(_int64)Generic->bigtick.QuadPart,
		runflag?"":" [PAUSED] ",
		Generic->isDone()?" [DONE] ":""
		);
	SetWindowText(hWnd,tmp);
	
	//
	HMENU hMenu=GetMenu(hWnd);
	
	//
	CheckMenuItem(hMenu,IDM_DEVELOPMENT,		(Generic->Alg==DEVELOPMENT_ALGORITHM)?MF_CHECKED:MF_UNCHECKED);
	CheckMenuItem(hMenu,IDM_ASTAR,				(Generic->Alg==ASTAR_ALGORITHM)?MF_CHECKED:MF_UNCHECKED);
	CheckMenuItem(hMenu,IDM_ASTAR_HEAPINTEGER,	(Generic->Alg==ASTAR_HEAPINTEGER_ALGORITHM)?MF_CHECKED:MF_UNCHECKED);
	CheckMenuItem(hMenu,IDM_ASTAR_HEAP,			(Generic->Alg==ASTAR_HEAP_ALGORITHM)?MF_CHECKED:MF_UNCHECKED);
	CheckMenuItem(hMenu,IDM_ASTAR_COMPLETE,		(Generic->Alg==ASTAR_COMPLETE_ALGORITHM)?MF_CHECKED:MF_UNCHECKED);
	CheckMenuItem(hMenu,IDM_ASTAR_LINKEDLIST,	(Generic->Alg==ASTAR_LINKEDLIST_ALGORITHM)?MF_CHECKED:MF_UNCHECKED);
	CheckMenuItem(hMenu,IDM_ASTAR_ARRAY,		(Generic->Alg==ASTAR_ARRAY_ALGORITHM)?MF_CHECKED:MF_UNCHECKED);
	CheckMenuItem(hMenu,IDM_DIJKSTRA,			(Generic->Alg==DIJKSTRA_ALGORITHM)?MF_CHECKED:MF_UNCHECKED);
	CheckMenuItem(hMenu,IDM_BREADTHFIRST,		(Generic->Alg==BREADTHFIRST_ALGORITHM)?MF_CHECKED:MF_UNCHECKED);
	CheckMenuItem(hMenu,IDM_BESTFIRST,			(Generic->Alg==BESTFIRST_ALGORITHM)?MF_CHECKED:MF_UNCHECKED);
	CheckMenuItem(hMenu,IDM_DEPTHFIRST,			(Generic->Alg==DEPTHFIRST_ALGORITHM)?MF_CHECKED:MF_UNCHECKED);
	CheckMenuItem(hMenu,IDM_DSTAR,				(Generic->Alg==DSTAR_ALGORITHM)?MF_CHECKED:MF_UNCHECKED);
	CheckMenuItem(hMenu,IDM_RIGHTHANDRULE,		(Generic->Alg==RIGHTHANDRULE_ALGORITHM)?MF_CHECKED:MF_UNCHECKED);
	
	//
	CheckMenuItem(hMenu,IDM_PRESEARCH,		(Setup->presearch_toggle)?MF_CHECKED:MF_UNCHECKED);
	CheckMenuItem(hMenu,IDM_PRESEARCH_VIEW,	(presearch_view)?MF_CHECKED:MF_UNCHECKED);
	
	//
	CheckMenuItem(hMenu,IDM_PAUSE_RESUME,	(!runflag)?MF_CHECKED:MF_UNCHECKED);
	
	//
	CheckMenuItem(hMenu,IDM_DRAWMODE,		(drawmode)?MF_CHECKED:MF_UNCHECKED);
	
	//
	CheckMenuItem(hMenu,IDM_ZOOM,			(zoom)?MF_CHECKED:MF_UNCHECKED);
	
	//
	CheckMenuItem(hMenu,IDM_GAMEMODE,		(Generic->gamemode)?MF_CHECKED:MF_UNCHECKED);
	
	//
	CheckMenuItem(hMenu,IDM_ROUTING_VIEW,	(Generic->routing_view)?MF_CHECKED:MF_UNCHECKED);
	
	//
	CheckMenuItem(hMenu,IDM_FPS_10S,		(TIMER_RATE==10000)?MF_CHECKED:MF_UNCHECKED);
	CheckMenuItem(hMenu,IDM_FPS_1,			(TIMER_RATE==1000)?MF_CHECKED:MF_UNCHECKED);
	CheckMenuItem(hMenu,IDM_FPS_10,			(TIMER_RATE==100)?MF_CHECKED:MF_UNCHECKED);
	CheckMenuItem(hMenu,IDM_FPS_30,			(TIMER_RATE==33)?MF_CHECKED:MF_UNCHECKED);
	CheckMenuItem(hMenu,IDM_FPS_60,			(TIMER_RATE==17)?MF_CHECKED:MF_UNCHECKED);
	
	//
	CheckMenuItem(hMenu,IDM_COLORSCHEME_WHITE,	(Setup->colorscheme==COLORSCHEME_WHITE)?MF_CHECKED:MF_UNCHECKED);
	CheckMenuItem(hMenu,IDM_COLORSCHEME_BLACK,	(Setup->colorscheme==COLORSCHEME_BLACK)?MF_CHECKED:MF_UNCHECKED);
	CheckMenuItem(hMenu,IDM_COLORSCHEME_GRAY,	(Setup->colorscheme==COLORSCHEME_GRAY)?MF_CHECKED:MF_UNCHECKED);
	CheckMenuItem(hMenu,IDM_COLORSCHEME_PAPER,	(Setup->colorscheme==COLORSCHEME_PAPER)?MF_CHECKED:MF_UNCHECKED);
	
	//
	CheckMenuItem(hMenu,IDM_MEDIAN_1,		(Setup->median_terrain_cost==1)?MF_CHECKED:MF_UNCHECKED);
	CheckMenuItem(hMenu,IDM_MEDIAN_8,		(Setup->median_terrain_cost==8)?MF_CHECKED:MF_UNCHECKED);
	CheckMenuItem(hMenu,IDM_MEDIAN_16,		(Setup->median_terrain_cost==16)?MF_CHECKED:MF_UNCHECKED);
	CheckMenuItem(hMenu,IDM_MEDIAN_AUTO,	(Setup->median_terrain_cost==Setup->median_terrain_cost_auto)?MF_CHECKED:MF_UNCHECKED);
	
	//
	MENUITEMINFO mii;
	memset(&mii,0,sizeof(MENUITEMINFO));
	mii.cbSize=sizeof(MENUITEMINFO);
    mii.fMask=MIIM_TYPE;
    mii.fType=MFT_STRING; //
    mii.fState=0; 
    mii.wID=0; //
    mii.hSubMenu=NULL; 
    //HBITMAP hbmpChecked; 
    //HBITMAP hbmpUnchecked; 
    //mii.dwItemData=tmp; 
	
	sprintf(tmp,"Auto (%f)",Setup->median_terrain_cost_auto);
    mii.dwTypeData=tmp;
    mii.cch=strlen(tmp); 
	SetMenuItemInfo(hMenu,IDM_MEDIAN_AUTO,FALSE,&mii);
	
	sprintf(tmp,"Game Mode (%d AIs - WARNING)",MAX_AIS); //\t'
    mii.dwTypeData=tmp;
    mii.cch=strlen(tmp); 
	SetMenuItemInfo(hMenu,IDM_GAMEMODE,FALSE,&mii);
	
	//
	if(Setup->use_terrain==TRUE)
	{
		EnableMenuItem(hMenu,IDM_MEDIAN_1,			MF_ENABLED);
		EnableMenuItem(hMenu,IDM_MEDIAN_8,			MF_ENABLED);
		EnableMenuItem(hMenu,IDM_MEDIAN_16,			MF_ENABLED);
		EnableMenuItem(hMenu,IDM_MEDIAN_AUTO,		MF_ENABLED);
	}
	else
	{
		EnableMenuItem(hMenu,IDM_MEDIAN_1,			MF_GRAYED);
		EnableMenuItem(hMenu,IDM_MEDIAN_8,			MF_GRAYED);
		EnableMenuItem(hMenu,IDM_MEDIAN_16,			MF_GRAYED);
		EnableMenuItem(hMenu,IDM_MEDIAN_AUTO,		MF_GRAYED);
	}
	
	
	//
	CheckMenuItem(hMenu,IDM_DIRECTIONS4,		(Setup->directions==4)?MF_CHECKED:MF_UNCHECKED);
	CheckMenuItem(hMenu,IDM_DIRECTIONS8,		(Setup->directions==8)?MF_CHECKED:MF_UNCHECKED);
	
	//
	CheckMenuItem(hMenu,IDM_ITERATIONS_1,		(Setup->iterations_per_frame==1)?MF_CHECKED:MF_UNCHECKED);
	CheckMenuItem(hMenu,IDM_ITERATIONS_10,		(Setup->iterations_per_frame==10)?MF_CHECKED:MF_UNCHECKED);
	CheckMenuItem(hMenu,IDM_ITERATIONS_100,		(Setup->iterations_per_frame==100)?MF_CHECKED:MF_UNCHECKED);
	CheckMenuItem(hMenu,IDM_ITERATIONS_1000,	(Setup->iterations_per_frame==1000)?MF_CHECKED:MF_UNCHECKED);
	CheckMenuItem(hMenu,IDM_ITERATIONS_10000,	(Setup->iterations_per_frame==10000)?MF_CHECKED:MF_UNCHECKED);
	CheckMenuItem(hMenu,IDM_ITERATIONS_FULL,	(Setup->iterations_per_frame==(WIDTH*HEIGHT))?MF_CHECKED:MF_UNCHECKED);
	
	//
	CheckMenuItem(hMenu,IDM_COST_0,				(Setup->use_terrain==false && Setup->cost==0.0f)?MF_CHECKED:MF_UNCHECKED);
	CheckMenuItem(hMenu,IDM_COST_0_5,			(Setup->use_terrain==false && Setup->cost==0.5f)?MF_CHECKED:MF_UNCHECKED);
	CheckMenuItem(hMenu,IDM_COST_0_25,			(Setup->use_terrain==false && Setup->cost==0.25f)?MF_CHECKED:MF_UNCHECKED);
	CheckMenuItem(hMenu,IDM_COST_0_125,			(Setup->use_terrain==false && Setup->cost==0.125f)?MF_CHECKED:MF_UNCHECKED);
	CheckMenuItem(hMenu,IDM_COST_1,				(Setup->use_terrain==false && Setup->cost==1.0f)?MF_CHECKED:MF_UNCHECKED);
	CheckMenuItem(hMenu,IDM_COST_2,				(Setup->use_terrain==false && Setup->cost==2.0f)?MF_CHECKED:MF_UNCHECKED);
	CheckMenuItem(hMenu,IDM_COST_4,				(Setup->use_terrain==false && Setup->cost==4.0f)?MF_CHECKED:MF_UNCHECKED);
	CheckMenuItem(hMenu,IDM_COST_8,				(Setup->use_terrain==false && Setup->cost==8.0f)?MF_CHECKED:MF_UNCHECKED);
	CheckMenuItem(hMenu,IDM_COST_16,			(Setup->use_terrain==false && Setup->cost==16.0f)?MF_CHECKED:MF_UNCHECKED);
	CheckMenuItem(hMenu,IDM_COST_BY_TERRAIN,	(Setup->use_terrain==TRUE)?MF_CHECKED:MF_UNCHECKED);
	
	//
	CheckMenuItem(hMenu,IDM_MANHATTAN_DISTANCE,			(Setup->distance_method==MANHATTAN_DISTANCE)?MF_CHECKED:MF_UNCHECKED);
	CheckMenuItem(hMenu,IDM_DIAGONAL_DISTANCE,			(Setup->distance_method==DIAGONAL_DISTANCE)?MF_CHECKED:MF_UNCHECKED);
	CheckMenuItem(hMenu,IDM_PYTHAGORAS_DISTANCE,		(Setup->distance_method==PYTHAGORAS_DISTANCE)?MF_CHECKED:MF_UNCHECKED);
	CheckMenuItem(hMenu,IDM_SIMPLE_PYTHAGORAS_DISTANCE,	(Setup->distance_method==SIMPLE_PYTHAGORAS_DISTANCE)?MF_CHECKED:MF_UNCHECKED);
	
	//
	CheckMenuItem(hMenu,IDM_DIAGONAL_COST_1,	(Setup->diagonal_cost==1.0f)?MF_CHECKED:MF_UNCHECKED);
	CheckMenuItem(hMenu,IDM_DIAGONAL_COST_1_4,	(Setup->diagonal_cost==1.4f)?MF_CHECKED:MF_UNCHECKED);
	
	
	//
	if(Setup->options.uniform_cost)
	{		
		EnableMenuItem(hMenu,IDM_COST_0,			MF_ENABLED);
		EnableMenuItem(hMenu,IDM_COST_0_5,			MF_ENABLED);
		EnableMenuItem(hMenu,IDM_COST_0_25,			MF_ENABLED);
		EnableMenuItem(hMenu,IDM_COST_0_125,		MF_ENABLED);
		EnableMenuItem(hMenu,IDM_COST_1,			MF_ENABLED);
		EnableMenuItem(hMenu,IDM_COST_2,			MF_ENABLED);
		EnableMenuItem(hMenu,IDM_COST_4,			MF_ENABLED);
		EnableMenuItem(hMenu,IDM_COST_8,			MF_ENABLED);
		EnableMenuItem(hMenu,IDM_COST_16,			MF_ENABLED);
	}
	else
	{
		EnableMenuItem(hMenu,IDM_COST_0,			MF_GRAYED);
		EnableMenuItem(hMenu,IDM_COST_0_5,			MF_GRAYED);
		EnableMenuItem(hMenu,IDM_COST_0_25,			MF_GRAYED);
		EnableMenuItem(hMenu,IDM_COST_0_125,		MF_GRAYED);
		EnableMenuItem(hMenu,IDM_COST_1,			MF_GRAYED);
		EnableMenuItem(hMenu,IDM_COST_2,			MF_GRAYED);
		EnableMenuItem(hMenu,IDM_COST_4,			MF_GRAYED);
		EnableMenuItem(hMenu,IDM_COST_8,			MF_GRAYED);
		EnableMenuItem(hMenu,IDM_COST_16,			MF_GRAYED);
	}
	//
	if(Setup->options.terrain_cost)
	{		
		EnableMenuItem(hMenu,IDM_COST_BY_TERRAIN,	MF_ENABLED);
	}
	else
	{
		EnableMenuItem(hMenu,IDM_COST_BY_TERRAIN,	MF_GRAYED);
	}
	// //diagonal_cost only matters with 8-point directions
	if( (Setup->options.terrain_cost || Setup->options.uniform_cost) && Setup->directions==8)
	{		
		EnableMenuItem(hMenu,IDM_DIAGONAL_COST_1,	MF_ENABLED);
		EnableMenuItem(hMenu,IDM_DIAGONAL_COST_1_4,	MF_ENABLED);
	}
	else
	{
		EnableMenuItem(hMenu,IDM_DIAGONAL_COST_1,	MF_GRAYED);
		EnableMenuItem(hMenu,IDM_DIAGONAL_COST_1_4,	MF_GRAYED);
	}
	
	//		
	if(Setup->options.distance)
	{
		EnableMenuItem(hMenu,IDM_PYTHAGORAS_DISTANCE,			MF_ENABLED);
		EnableMenuItem(hMenu,IDM_DIAGONAL_DISTANCE,				MF_ENABLED);
		EnableMenuItem(hMenu,IDM_MANHATTAN_DISTANCE,			MF_ENABLED);
		EnableMenuItem(hMenu,IDM_SIMPLE_PYTHAGORAS_DISTANCE,	MF_ENABLED);
	}
	else
	{
		EnableMenuItem(hMenu,IDM_PYTHAGORAS_DISTANCE,			MF_GRAYED);
		EnableMenuItem(hMenu,IDM_DIAGONAL_DISTANCE,				MF_GRAYED);
		EnableMenuItem(hMenu,IDM_MANHATTAN_DISTANCE,			MF_GRAYED);
		EnableMenuItem(hMenu,IDM_SIMPLE_PYTHAGORAS_DISTANCE,	MF_GRAYED);
	}
	
	//
	if(Setup->options.search_directions)
	{
		EnableMenuItem(hMenu,IDM_DIRECTIONS4,		MF_ENABLED);
		EnableMenuItem(hMenu,IDM_DIRECTIONS8,		MF_ENABLED);
	}
	else
	{		
		EnableMenuItem(hMenu,IDM_DIRECTIONS4,		MF_GRAYED);
		EnableMenuItem(hMenu,IDM_DIRECTIONS8,		MF_GRAYED);
	}		
	
	//
	if(runflag)
	{
//		SetClassLong(hWnd, GCL_HICON, (long)LoadIcon(hInst, (LPCTSTR)IDI_UNPAUSED));
//		SetClassLong(hWnd, GCL_HICONSM, (long)LoadIcon(hInst, (LPCTSTR)IDI_UNPAUSED));
		SetClassLongPtr(hWnd, GCLP_HICON, (long)LoadIcon(hInst, (LPCTSTR)IDI_UNPAUSED));
		SetClassLongPtr(hWnd, GCLP_HICONSM, (long)LoadIcon(hInst, (LPCTSTR)IDI_UNPAUSED));
	}
	else
	{
//		SetClassLong(hWnd, GCL_HICON, (long)LoadIcon(hInst, (LPCTSTR)IDI_PAUSED));
//		SetClassLong(hWnd, GCL_HICONSM, (long)LoadIcon(hInst, (LPCTSTR)IDI_PAUSED));
		SetClassLongPtr(hWnd, GCLP_HICON, (long)LoadIcon(hInst, (LPCTSTR)IDI_PAUSED));
		SetClassLongPtr(hWnd, GCLP_HICONSM, (long)LoadIcon(hInst, (LPCTSTR)IDI_PAUSED));
	}
}

void Redraw(HWND hWnd)
{
	if(mutex==0)
	{
		mutex++;
		InvalidateRect ( hWnd,NULL, FALSE );
		mutex--;
	}
}

void update(HWND hWnd)
{
	Generic->ResetAI();
	Generic->UpdateSettings();
	Generic->UpdateWorld();
	Generic->Reset();
	Redraw(hWnd);
}

////////////////////////////////////////////////////////////////////////////
////////////////////////////////////////////////////////////////////////////
////////////////////////////////////////////////////////////////////////////
int APIENTRY WinMain(HINSTANCE hInstance,
                     HINSTANCE hPrevInstance,
                     LPSTR     lpCmdLine,
                     int       nCmdShow)
{
	hPrevInstance=hPrevInstance; //nix compiler warnings
	lpCmdLine=lpCmdLine; //nix compiler warnings
	
	// we're not paused... start off running.
	runflag=true;
	zoom=false;
	presearch_view=false;
	drawmode=false;
	pendown=false;
	brushmode=false;
	elevation=IMPASSABLE_TERRAIN_COST;
	lastx=lasty=0;
	
	//
	Setup=new CSetup();
	Setup->set_start(252,128);
	Setup->set_end(138,128);
	Setup->Load("labyrinth.tga");
	
	//
	Generic=new CGeneric(Setup);
	
	//
	MINIMUM_CLIENT_WIDTH=(WIDTH+4)*3;
	MINIMUM_CLIENT_HEIGHT=(HEIGHT*2+16);
	
	
	// TODO: Place code here.
	MSG msg;
	HACCEL hAccelTable;
	
	// Initialize global strings
	LoadString(hInstance, IDS_APP_TITLE, szTitle, MAX_LOADSTRING);
	LoadString(hInstance, IDC_PATHFINDER2D, szWindowClass, MAX_LOADSTRING);
	MyRegisterClass(hInstance);
	
	// Perform application initialization:
	if (!InitInstance (hInstance, nCmdShow)) 
	{
		return FALSE;
	}
	
	hAccelTable = LoadAccelerators(hInstance, (LPCTSTR)IDC_PATHFINDER2D);
	
	// Main message loop:
	while (GetMessage(&msg, NULL, 0, 0)) 
	{
		if (!TranslateAccelerator(msg.hwnd, hAccelTable, &msg)) 
		{
			TranslateMessage(&msg);
			DispatchMessage(&msg);
		}
	}
	
	//
	delete Generic;
	Generic=NULL;

	delete Setup;
	Setup=NULL;

	//	
	return msg.wParam;
}



//
//  FUNCTION: MyRegisterClass()
//
//  PURPOSE: Registers the window class.
//
//  COMMENTS:
//
//    This function and its usage is only necessary if you want this code
//    to be compatible with Win32 systems prior to the 'RegisterClassEx'
//    function that was added to Windows 95. It is important to call this function
//    so that the application will get 'well formed' small icons associated
//    with it.
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
	wcex.hIcon			= LoadIcon(hInstance, (LPCTSTR)IDI_PATHFINDER2D);
	wcex.hCursor		= LoadCursor(NULL, IDC_ARROW);
	wcex.hbrBackground	= (HBRUSH)(COLOR_WINDOW+1);
	wcex.lpszMenuName	= (LPCSTR)IDC_PATHFINDER2D;
	wcex.lpszClassName	= szWindowClass;
	wcex.hIconSm		= LoadIcon(wcex.hInstance, (LPCTSTR)IDI_SMALL);
	
	return RegisterClassEx(&wcex);
}

//
//   FUNCTION: InitInstance(HANDLE, int)
//
//   PURPOSE: Saves instance handle and creates main window
//
//   COMMENTS:
//
//        In this function, we save the instance handle in a global variable and
//        create and display the main program window.
//
BOOL InitInstance(HINSTANCE hInstance, int nCmdShow)
{
	HWND hWnd;
	
	hInst = hInstance; // Store instance handle in our global variable
	
	hWnd = CreateWindow(szWindowClass, szTitle,
		WS_OVERLAPPEDWINDOW,
		CW_USEDEFAULT, CW_USEDEFAULT,
		SCREEN_WIDTH, SCREEN_HEIGHT,
		NULL, NULL, hInstance, NULL);
	
	if (!hWnd)
	{
		return FALSE;
	}
	
	// center window
	RECT rt;
	GetWindowRect(hWnd,&rt);
	int width=rt.right-rt.left;
	int height=rt.bottom-rt.top;
	int desktopWidth = GetSystemMetrics(SM_CXSCREEN);
	int desktopHeight = GetSystemMetrics(SM_CYSCREEN);
	MoveWindow(hWnd,((desktopWidth>>1)-(width>>1)),((desktopHeight>>1)-(height>>1)), width, height, TRUE);
	
	//
	ShowWindow(hWnd, nCmdShow);
	UpdateWindow(hWnd);
	
	//
	ShowWindow(hWnd,SW_MAXIMIZE);
	
	//
	TIMER_RATE=100;
	SetTimer(hWnd, TIMER_ID, TIMER_RATE, NULL); //start the update timer
	
	//
	return TRUE;
}




///////////////////////////////////////////////////////////////////////////
///////////////////////////////////////////////////////////////////////////
///////////////////////////////////////////////////////////////////////////

//
void Zoom_Composite(LPBYTE pwBits,HWND hWnd)
{
	if(pwBits)
	{
		//
		RECT rt;
		GetClientRect(hWnd, &rt);
		const int clientwidth = (rt.right-rt.left);
		
		DWORD *spush,*s;
		DWORD *dpush,*d;
		s=spush=(DWORD *)pwBits+WIDTH-1+(HEIGHT-1)*clientwidth;
		d=dpush=(DWORD *)pwBits+WIDTH*2-1+(HEIGHT-1)*clientwidth*2;
		
		int y,x;
		for(y=HEIGHT-1;y>=0;y--)
		{
			for(x=WIDTH-1;x>=0;x--)
			{
				DWORD color=*s;
				*(d+0)=color;
				*(d+1)=color;
				*(d+clientwidth)=color;
				*(d+clientwidth+1)=color;
				d-=2;
				s--;
			}
			spush-=clientwidth;
			s=spush;
			dpush-=clientwidth<<1;
			d=dpush;
		}
	}
}

//
void Presearch_View(LPBYTE pwBits, HDC hdc,HWND hWnd)
{
	if(pwBits)
	{
		//
		RECT rt;
		GetClientRect(hWnd, &rt);
		int clientwidth = (rt.right-rt.left);
		int clientheight = (rt.bottom-rt.top);
		
		COLORREF background;
		COLORREF foreground;
		Setup->get_colorscheme_colors(background,foreground);
		HBRUSH hbrBkGnd = CreateSolidBrush(background);
		FillRect(hdc, &rt, hbrBkGnd);
		DeleteObject(hbrBkGnd);
		SetBkColor(hdc,background);
		SetTextColor(hdc,foreground);
		
		//
		TCHAR szStatusLine[1024];
		GetClientRect(hWnd, &rt);
		sprintf(szStatusLine,"%d groups of areas unpathable to each other",Setup->presearch_maxgroup);
		DrawText(hdc, szStatusLine, strlen(szStatusLine), &rt, DT_CENTER);
		
		// draw world map
		_RGBA *pbegin=(_RGBA *)pwBits+(clientwidth>>1)-(WIDTH)+ ((clientheight>>1)-(HEIGHT))*clientwidth;
		DWORD *tmp=(DWORD *)pbegin;
		_RGBA *ppush=pbegin;
		for(int y=0;y<HEIGHT;y++)
		{
			for(int x=0;x<WIDTH;x++)
			{
				BYTE group=Setup->world[y][x].group;
				DWORD d;
				if(group==NO_GROUP)
					d=0x0000ff00;
				else if(group==IMPASSABLE_GROUP)
					d=0x00ff0000;
				else
				{// hmm. this right?
					BYTE r=((group&1)<<7) + ((group& 8)<<2);
					BYTE g=((group&2)<<6) + ((group&16)<<1) + ((group&64)>>6);
					BYTE b=((group&4)<<5) + ((group&32)<<0) + ((group&128)>>8);
					d=(r<<16) | (g<<8) | b;
				}
				*tmp=d;
				*(tmp+1)=d;
				*(tmp+clientwidth)=d;
				*(tmp+clientwidth+1)=d;
				tmp+=2;
			}
			ppush+=(clientwidth<<1);
			tmp=(DWORD *)ppush;
		}
	}
}



//
void DrawMode(LPBYTE pwBits, HDC hdc,HWND hWnd)
{
	if(pwBits)
	{
		//
		RECT rt;
		GetClientRect(hWnd, &rt);
		const int clientwidth = (rt.right-rt.left);
		
		COLORREF background;
		COLORREF foreground;
		Setup->get_colorscheme_colors(background,foreground);
		HBRUSH hbrBkGnd = CreateSolidBrush(background);
		FillRect(hdc, &rt, hbrBkGnd);
		DeleteObject(hbrBkGnd);
		SetBkColor(hdc,background);
		SetTextColor(hdc,foreground);
		
		//
		_RGBA *p,*ppush,*pbegin;
		pbegin=(_RGBA *)pwBits;
		int y,x;
		int n;
		
		// draw world map
		p=ppush=pbegin;
		for(y=0;y<HEIGHT;y++)
		{
			for(x=0;x<WIDTH;x++)
			{
				_RGBA color;
				BYTE b=(BYTE)(255-(Setup->world[y][x].terrain_cost<<4));
				color.red=b;
				color.green=b;
				color.blue=b;
				*(p+0)=color;
				*(p+1)=color;
				*(p+clientwidth)=color;
				*(p+clientwidth+1)=color;
				p+=2;
			}
			ppush+=clientwidth<<1;
			p=ppush;
		}
		
		// draw elevations for selection
		p=ppush=pbegin+(WIDTH<<1);
		for(n=15;n>=0;n--)
		{
			BYTE b=(BYTE)((n<<4)+15);
			for(y=0;y<20;y++)
			{
				for(x=0;x<40;x++)
				{
					(p+x)->red=b;
					(p+x)->green=b;
					(p+x)->blue=b;
				}
				ppush+=clientwidth;
				p=ppush;
			}
		}
		
		// draw current elevation
		p=ppush=pbegin+(WIDTH<<1)+60+HEIGHT*clientwidth;
		BYTE b=(BYTE)((15-elevation)<<4);
		for(y=0;y<60;y++)
		{
			for(x=0;x<60;x++)
			{
				(p+x)->red=b;
				(p+x)->green=b;
				(p+x)->blue=b;
			}
			ppush+=clientwidth;
			p=ppush;
		}
		
		// draw line mode
		p=ppush=pbegin+(WIDTH<<1)+60+10*clientwidth;;
		for(y=0;y<1;y++)
		{
			for(x=0;x<60;x++)
			{
				(p+x)->red=0;
				(p+x)->green=255;
				(p+x)->blue=0;
			}
			ppush+=clientwidth;
			p=ppush;
		}
		// draw brush mode
		p=ppush=pbegin+(WIDTH<<1)+60+20*clientwidth;;
		for(y=0;y<20;y++)
		{
			for(x=0;x<60;x++)
			{
				(p+x)->red=0;
				(p+x)->green=255;
				(p+x)->blue=0;
			}
			ppush+=clientwidth;
			p=ppush;
		}
		
		//
		p=ppush=pbegin+(WIDTH<<1)+60+(brushmode*20)*clientwidth;;
		for(x=0;x<60;x++)
		{
			(p+x)->red=255;
			(p+x)->green=0;
			(p+x)->blue=0;
			(p+x+clientwidth*20)->red=255;
			(p+x+clientwidth*20)->green=0;
			(p+x+clientwidth*20)->blue=0;
		}
		for(y=0;y<20;y++)
		{
			(p+y*clientwidth)->red=255;
			(p+y*clientwidth)->green=0;
			(p+y*clientwidth)->blue=0;
			(p+60+y*clientwidth)->red=255;
			(p+60+y*clientwidth)->green=0;
			(p+60+y*clientwidth)->blue=0;
		}
	}
}



//
void PaintFunc_TooSmall(HDC hdc,HWND hWnd, int w, int h)
{
	if(pwBits)
	{
		//
		RECT rt;
		GetClientRect(hWnd, &rt);
		
		COLORREF background;
		COLORREF foreground;
		Setup->get_colorscheme_colors(background,foreground);
		HBRUSH hbrBkGnd = CreateSolidBrush(background);
		FillRect(hdc, &rt, hbrBkGnd);
		DeleteObject(hbrBkGnd);
		SetBkColor(hdc,background);
		SetTextColor(hdc,foreground);
		
		//
		TCHAR szStatusLine[1024];
		
		sprintf(szStatusLine,"Window has been resized too small.\n\n");
		DrawText(hdc, szStatusLine, strlen(szStatusLine), &rt, DT_CENTER);
		
		if(w>0)
		{
			rt.top+=16;
			sprintf(szStatusLine,"<-- width short by %d pixels -->",w);
			DrawText(hdc, szStatusLine, strlen(szStatusLine), &rt, DT_CENTER);
		}
		
		if(h>0)
		{
			rt.top+=16;
			sprintf(szStatusLine,">-- height short by %d pixels --<",h);
			DrawText(hdc, szStatusLine, strlen(szStatusLine), &rt, DT_CENTER);
		}
	}
}


//
void pixel(int x, int y)
{
	if(x>=0 && x<WIDTH && y>=0 && y<HEIGHT)
	{
		Setup->world[y][x].terrain_cost=elevation;
	}
}


//
void line(
		  int x1,
		  int y1,
		  int x2,
		  int y2
		  )
{
	int x, y;
	int dx, dy;
	int incx, incy;
	int balance;
	
	if (x2 >= x1)
	{
		dx = x2 - x1;
		incx = 1;
	}
	else
	{
		dx = x1 - x2;
		incx = -1;
	}
	
	if (y2 >= y1)
	{
		dy = y2 - y1;
		incy = 1;
	}
	else
	{
		dy = y1 - y2;
		incy = -1;
	}
	
	x = x1;
	y = y1;
	
	if (dx >= dy)
	{
		dy <<= 1;
		balance = dy - dx;
		dx <<= 1;
		
		while (x != x2)
		{
			pixel(x,y);
			if (balance >= 0)
			{
				y += incy;
				balance -= dx;
			}
			balance += dy;
			x += incx;
		} 
		pixel(x,y);
	}
	else
	{
		dx <<= 1;
		balance = dx - dy;
		dy <<= 1;
		
		while (y != y2)
		{
			pixel(x,y);
			if (balance >= 0)
			{
				x += incx;
				balance -= dy;
			}
			balance += dx;
			y += incy;
		}
		pixel(x,y);
	}
}


//
//  FUNCTION: WndProc(HWND, unsigned, WORD, LONG)
//
//  PURPOSE:  Processes messages for the main window.
//
//  WM_COMMAND	- process the application menu
//  WM_PAINT	- Paint the main window
//  WM_DESTROY	- post a quit message and return
//
//
LRESULT CALLBACK WndProc(HWND hWnd, UINT message, WPARAM wParam, LPARAM lParam)
{
	int wmId, wmEvent;
	PAINTSTRUCT ps;
	HDC hdc;
	
	switch (message) 
	{
		
	case WM_MBUTTONDOWN:
		{
			//			WORD fwKeys = wParam;        // key flags 
			WORD xPos = LOWORD(lParam);  // horizontal position of cursor 
			WORD yPos = HIWORD(lParam);  // vertical position of cursor 
			
			drawmode=!drawmode;
			runflag=!drawmode;
			Setup->blocking_walls();
			
			lastx=xPos>>1;
			lasty=yPos>>1;
			Redraw(hWnd);
		}
		break;
	case WM_MBUTTONUP:
		{
		}
		break;
		/*
		#ifndef WM_MOUSEWHEEL
		#error No WM_MOUSEWHEEL. Include latest Platform SDK if using VC++ 6.
		#else
		case WM_MOUSEWHEEL:
		{
		WORD fwKeys = LOWORD(wParam);    // key flags
		short zDelta = (short) HIWORD(wParam);    // wheel rotation
		WORD xPos = (short) LOWORD(lParam);    // horizontal position of pointer
		WORD yPos = (short) HIWORD(lParam);    // vertical position of pointer
		
		  Generic->ai+=(((zDelta)/WHEEL_DELTA)>0)?-1:1;
		  if(Generic->ai<0) Generic->ai=0;
		  if(Generic->ai>=MAX_AIS) Generic->ai=MAX_AIS-1;
		  
			Redraw(hWnd);
			}
			break;
			#endif
		*/
	case WM_LBUTTONDOWN:
		{
			if(!drawmode)
			{
				//			WORD fwKeys = wParam;        // key flags 
				WORD xPos = LOWORD(lParam);  // horizontal position of cursor 
				WORD yPos = HIWORD(lParam);  // vertical position of cursor 
				
				if(zoom)
				{ lastx=xPos>>1; lasty=yPos>>1; }
				else
				{ lastx=xPos; lasty=yPos; }
				
				if(zoom) {xPos=(WORD)(xPos>>1); yPos=(WORD)(yPos>>1); }
				
				if(xPos<WIDTH && yPos<HEIGHT && xPos>0 && yPos>0)
				{
					if(Setup->world[yPos][xPos].terrain_cost<15)
					{
						Setup->starty=(BYTE)yPos;
						Setup->startx=(BYTE)xPos;
						Setup->okToPath=false;
						Generic->UpdateSettings();
						Redraw(hWnd);
					}
				}
			}
			else
			{
				//			WORD fwKeys = wParam;        // key flags 
				WORD xPos = LOWORD(lParam);  // horizontal position of cursor 
				WORD yPos = HIWORD(lParam);  // vertical position of cursor 
				
				if(xPos>(WIDTH<<1) && xPos<(WIDTH<<1)+40)
				{
					elevation=(yPos/20);
					Redraw(hWnd);
				}
				else if(xPos>(WIDTH<<1)+60 && xPos<(WIDTH<<1)+60+60)
				{
					if(yPos>=0 && yPos<20)
					{
						brushmode=false;
						Redraw(hWnd);
					}
					else if(yPos>=20 && yPos<40)
					{
						brushmode=true;
						Redraw(hWnd);
					}
				}
				else if(xPos<(WIDTH<<1) && yPos<(HEIGHT<<1) && xPos>0 && yPos>0)
				{
					pendown=true;
					lastx=xPos>>1;
					lasty=yPos>>1;
				}
			}
		}
		break;
	case WM_LBUTTONUP:
		{
			pendown=false;
		}
		break;
		
	case WM_RBUTTONDOWN:
		{
			if(!drawmode)
			{
				//			WORD fwKeys = wParam;        // key flags 
				WORD xPos = LOWORD(lParam);  // horizontal position of cursor 
				WORD yPos = HIWORD(lParam);  // vertical position of cursor 
				
				if(zoom)
				{ lastx=xPos>>1; lasty=yPos>>1; }
				else
				{ lastx=xPos; lasty=yPos; }
				
				if(zoom) {xPos=(WORD)(xPos>>1); yPos=(WORD)(yPos>>1); }
				
				if(xPos<WIDTH && yPos<HEIGHT && xPos>0 && yPos>0)
				{
					if(Setup->world[yPos][xPos].terrain_cost<15)
					{
						Setup->endy=(BYTE)yPos;
						Setup->endx=(BYTE)xPos;
						Setup->okToPath=true;
						Generic->UpdateSettings();
						Generic->Reset();
						Redraw(hWnd);
					}
				}
			}
		}
		break;
	case WM_RBUTTONUP:
		{
			pendown=false;
		}
		break;
		
	case WM_MOUSEMOVE:
		{
			if(drawmode)
			{
				//			WORD fwKeys = wParam;        // key flags 
				WORD xPos = LOWORD(lParam);  // horizontal position of cursor 
				WORD yPos = HIWORD(lParam);  // vertical position of cursor 
				
				xPos=(WORD)(xPos>>1); yPos=(WORD)(yPos>>1);
				
				if(xPos<WIDTH-1 && yPos<HEIGHT-1 && xPos>1 && yPos>1)
				{
					if(pendown)
					{
						if(!brushmode)
							line(lastx,lasty,xPos,yPos);
						else
						{
							for(int y=-3;y<3;y++)
							{
								for(int x=-3;x<3;x++)
								{
									line(lastx+x,lasty+y,xPos+x,yPos+y);
								}
							}
						}
						lastx=xPos;
						lasty=yPos;
						Generic->UpdateWorld();
						Redraw(hWnd);
					}
				}
			}
		}
		break;
		
		//
	case WM_COMMAND:
		wmId    = LOWORD(wParam); 
		wmEvent = HIWORD(wParam); 
		// Parse the menu selections:
		switch (wmId)
		{
		case IDM_ABOUT:
			DialogBox(hInst, (LPCTSTR)IDD_ABOUTBOX, hWnd, (DLGPROC)About);
			break;
			
		case IDM_CONTENTS:
			WinHelp(hWnd,"pathfinder2d.hlp",HELP_CONTENTS,0);
			break;
		case IDM_INDEX:
			WinHelp(hWnd,"pathfinder2d.hlp",HELP_INDEX,0);
			break;
		case IDM_FIND:
			WinHelp(hWnd,"pathfinder2d.hlp",HELP_FINDER,0);
			break;
			
		case IDM_PAUSE_RESUME:
			runflag=!runflag;
			Redraw(hWnd);
			break;
			
		case IDM_REPATH:
			runflag=true;
			Generic->Reset();
			Redraw(hWnd);
			break;
			
		case IDM_UPDATE_AND_REPATH:
			runflag=true;
			update(hWnd);
			break;
			
		case IDM_ZOOM:
			if(!Generic->routing_view)
			{
				zoom=!zoom;
				Redraw(hWnd);
			}
			break;
			
		case IDM_DRAWMODE:
			if(drawmode)
				Setup->Presearch();
			drawmode=!drawmode;
			runflag=!drawmode;
			Setup->blocking_walls();
			Redraw(hWnd);
			break;
			
			//
		case IDM_PRESEARCH:
			Setup->presearch_toggle=!Setup->presearch_toggle;
			update(hWnd);
			break;
		case IDM_PRESEARCH_VIEW:
			presearch_view=!presearch_view;
			Redraw(hWnd);
			break;
			
			//
		case IDM_SAVE_SETTINGS:
			Setup->save_settings();
			break;
		case IDM_LOAD_SETTINGS:
			Setup->load_settings();
			Generic->Reset();
			Redraw(hWnd);
			break;
			
			//
		case IDM_GAMEMODE:
			if(Generic->gamemode)
				Generic->set_GameMode(false);
			else
				Generic->set_GameMode(true);
			Generic->Reset();
			Redraw(hWnd);
			break;
			
		case IDM_ROUTING_VIEW:
			Generic->routing_view=!Generic->routing_view;
			Redraw(hWnd);
			break;
			
			//
		case IDM_DEVELOPMENT:
			Generic->Switch_Algorithms(DEVELOPMENT_ALGORITHM);
			Generic->Reset();
			Redraw(hWnd);
			break;
		case IDM_ASTAR:
			Generic->Switch_Algorithms(ASTAR_ALGORITHM);
			Generic->Reset();
			Redraw(hWnd);
			break;
		case IDM_ASTAR_HEAPINTEGER:
			Generic->Switch_Algorithms(ASTAR_HEAPINTEGER_ALGORITHM);
			Generic->Reset();
			Redraw(hWnd);
			break;
		case IDM_ASTAR_HEAP:
			Generic->Switch_Algorithms(ASTAR_HEAP_ALGORITHM);
			Generic->Reset();
			Redraw(hWnd);
			break;
		case IDM_ASTAR_COMPLETE:
			Generic->Switch_Algorithms(ASTAR_COMPLETE_ALGORITHM);
			Generic->Reset();
			Redraw(hWnd);
			break;
		case IDM_ASTAR_ARRAY:
			Generic->Switch_Algorithms(ASTAR_ARRAY_ALGORITHM);
			Generic->Reset();
			Redraw(hWnd);
			break;
		case IDM_ASTAR_LINKEDLIST:
			Generic->Switch_Algorithms(ASTAR_LINKEDLIST_ALGORITHM);
			Generic->Reset();
			Redraw(hWnd);
			break;
		case IDM_DIJKSTRA:
			Generic->Switch_Algorithms(DIJKSTRA_ALGORITHM);
			Generic->Reset();
			Redraw(hWnd);
			break;
		case IDM_BREADTHFIRST:
			Generic->Switch_Algorithms(BREADTHFIRST_ALGORITHM);
			Generic->Reset();
			Redraw(hWnd);
			break;
		case IDM_BESTFIRST:
			Generic->Switch_Algorithms(BESTFIRST_ALGORITHM);
			Generic->Reset();
			Redraw(hWnd);
			break;
		case IDM_DEPTHFIRST:
			Generic->Switch_Algorithms(DEPTHFIRST_ALGORITHM);
			Generic->Reset();
			Redraw(hWnd);
			break;
		case IDM_BIBFS:
			Generic->Switch_Algorithms(BIBFS_ALGORITHM);
			Generic->Reset();
			Redraw(hWnd);
			break;
		case IDM_DSTAR:
			Generic->Switch_Algorithms(DSTAR_ALGORITHM);
			Generic->Reset();
			Redraw(hWnd);
			break;
		case IDM_RIGHTHANDRULE:
			Generic->Switch_Algorithms(RIGHTHANDRULE_ALGORITHM);
			Generic->Reset();
			Redraw(hWnd);
			break;
			
			//
		case IDM_COLORSCHEME_WHITE:
			Setup->colorscheme=COLORSCHEME_WHITE;
			Redraw(hWnd);
			break;
		case IDM_COLORSCHEME_BLACK:
			Setup->colorscheme=COLORSCHEME_BLACK;
			Redraw(hWnd);
			break;
		case IDM_COLORSCHEME_GRAY:
			Setup->colorscheme=COLORSCHEME_GRAY;
			Redraw(hWnd);
			break;
		case IDM_COLORSCHEME_PAPER:
			Setup->colorscheme=COLORSCHEME_PAPER;
			Redraw(hWnd);
			break;
			
			// update screen
		case IDM_FPS_60: //60fps
			TIMER_RATE=17;
			KillTimer(hWnd, TIMER_ID);
			SetTimer(hWnd, TIMER_ID, TIMER_RATE, NULL);
			Redraw(hWnd);
			break;
		case IDM_FPS_30:
			TIMER_RATE=33;
			KillTimer(hWnd, TIMER_ID);
			SetTimer(hWnd, TIMER_ID, TIMER_RATE, NULL);
			Redraw(hWnd);
			break;
		case IDM_FPS_10:
			TIMER_RATE=100;
			KillTimer(hWnd, TIMER_ID);
			SetTimer(hWnd, TIMER_ID, TIMER_RATE, NULL);
			Redraw(hWnd);
			break;
		case IDM_FPS_1:
			TIMER_RATE=1000;
			KillTimer(hWnd, TIMER_ID);
			SetTimer(hWnd, TIMER_ID, TIMER_RATE, NULL);
			Redraw(hWnd);
			break;
		case IDM_FPS_10S:
			TIMER_RATE=10000;
			KillTimer(hWnd, TIMER_ID);
			SetTimer(hWnd, TIMER_ID, TIMER_RATE, NULL);
			Redraw(hWnd);
			break;
			
			//
		case IDM_MEDIAN_1:
			Setup->median_terrain_cost=1.0f;
			Generic->UpdateSettings();
			Generic->Reset();
			Redraw(hWnd);
			break;
		case IDM_MEDIAN_8:
			Setup->median_terrain_cost=8.0f;
			Generic->UpdateSettings();
			Generic->Reset();
			Redraw(hWnd);
			break;
		case IDM_MEDIAN_16:
			Setup->median_terrain_cost=16.0f;
			Generic->UpdateSettings();
			Generic->Reset();
			Redraw(hWnd);
			break;
		case IDM_MEDIAN_AUTO:
			Setup->median_terrain_cost=Setup->median_terrain_cost_auto;
			Generic->UpdateSettings();
			Generic->Reset();
			Redraw(hWnd);
			break;
			
			//
		case IDM_DIAGONAL_COST_1:
			Setup->diagonal_cost=1.0f;
			Generic->UpdateSettings();
			Generic->Reset();
			Redraw(hWnd);
			break;
		case IDM_DIAGONAL_COST_1_4:
			Setup->diagonal_cost=1.4f;
			Generic->UpdateSettings();
			Generic->Reset();
			Redraw(hWnd);
			break;
			
			//
		case IDM_COST_0:
			Setup->use_terrain=false;
			Setup->cost=0.0f;
			Generic->UpdateSettings();
			Generic->Reset();
			Redraw(hWnd);
			break;
		case IDM_COST_0_5:
			Setup->use_terrain=false;
			Setup->cost=0.5f;
			Generic->UpdateSettings();
			Generic->Reset();
			Redraw(hWnd);
			break;
		case IDM_COST_0_25:
			Setup->use_terrain=false;
			Setup->cost=0.25f;
			Generic->UpdateSettings();
			Generic->Reset();
			Redraw(hWnd);
			break;
		case IDM_COST_0_125:
			Setup->use_terrain=false;
			Setup->cost=0.125f;
			Generic->UpdateSettings();
			Generic->Reset();
			Redraw(hWnd);
			break;
		case IDM_COST_1:
			Setup->use_terrain=false;
			Setup->cost=1.0f;
			Generic->UpdateSettings();
			Generic->Reset();
			Redraw(hWnd);
			break;
		case IDM_COST_2:
			Setup->use_terrain=false;
			Setup->cost=2.0f;
			Generic->UpdateSettings();
			Generic->Reset();
			Redraw(hWnd);
			break;
		case IDM_COST_4:
			Setup->use_terrain=false;
			Setup->cost=4.0f;
			Generic->UpdateSettings();
			Generic->Reset();
			Redraw(hWnd);
			break;
		case IDM_COST_8:
			Setup->use_terrain=false;
			Setup->cost=8.0f;
			Generic->UpdateSettings();
			Generic->Reset();
			Redraw(hWnd);
			break;
		case IDM_COST_16:
			Setup->use_terrain=false;
			Setup->cost=16.0f;
			Generic->UpdateSettings();
			Generic->Reset();
			Redraw(hWnd);
			break;
		case IDM_COST_BY_TERRAIN:
			Setup->use_terrain=true;
			Generic->UpdateSettings();
			Generic->Reset();
			Redraw(hWnd);
			break;
			
			//
		case IDM_PYTHAGORAS_DISTANCE:
			Setup->distance_method=PYTHAGORAS_DISTANCE;
			Generic->UpdateSettings();
			Generic->Reset();
			Redraw(hWnd);
			break;
		case IDM_MANHATTAN_DISTANCE:
			Setup->distance_method=MANHATTAN_DISTANCE;
			Generic->UpdateSettings();
			Generic->Reset();
			Redraw(hWnd);
			break;
		case IDM_DIAGONAL_DISTANCE:
			Setup->distance_method=DIAGONAL_DISTANCE;
			Generic->UpdateSettings();
			Generic->Reset();
			Redraw(hWnd);
			break;
		case IDM_SIMPLE_PYTHAGORAS_DISTANCE:
			Setup->distance_method=SIMPLE_PYTHAGORAS_DISTANCE;
			Generic->UpdateSettings();
			Generic->Reset();
			Redraw(hWnd);
			break;
			
			//
		case IDM_ITERATIONS_1:
			Setup->iterations_per_frame=1;
			Generic->UpdateSettings();
			break;
		case IDM_ITERATIONS_10:
			Setup->iterations_per_frame=10;
			Generic->UpdateSettings();
			break;
		case IDM_ITERATIONS_100:
			Setup->iterations_per_frame=100;
			Generic->UpdateSettings();
			break;
		case IDM_ITERATIONS_1000:
			Setup->iterations_per_frame=1000;
			Generic->UpdateSettings();
			break;
		case IDM_ITERATIONS_10000:
			Setup->iterations_per_frame=10000;
			Generic->UpdateSettings();
			break;
		case IDM_ITERATIONS_FULL:
			Setup->iterations_per_frame=WIDTH*HEIGHT;
			Generic->UpdateSettings();
			break;
			
			//
		case IDM_DEBUG12:
			Generic->set_GameMode(false);
			Setup->set_start(10,10);
			Setup->set_end(190,190);
			Setup->distance_method=MANHATTAN_DISTANCE;
			Setup->directions=4;
			Setup->iterations_per_frame=WIDTH*HEIGHT;
			Setup->use_terrain=true;
			Setup->diagonal_cost=1.0f;
			Setup->Load("mars200x200.tga");
			update(hWnd);
			break;
		case IDM_DEBUG11:
			Generic->set_GameMode(false);
			Setup->set_start(10,10);
			Setup->set_end(138,177);
			Setup->distance_method=MANHATTAN_DISTANCE;
			Setup->directions=4;
			Setup->iterations_per_frame=1000;
			Setup->use_terrain=true;
			Setup->Load("mars.tga");
			update(hWnd);
			break;
		case IDM_DEBUG10:
			Generic->set_GameMode(false);
			Setup->set_start(10,10);
			Setup->set_end((BYTE)(HEIGHT-20),(BYTE)(WIDTH-20));
			Setup->distance_method=MANHATTAN_DISTANCE;
			Setup->directions=4;
			Setup->iterations_per_frame=WIDTH*HEIGHT;
			Setup->use_terrain=true;
			Setup->Load("mars.tga");
			update(hWnd);
			break;
		case IDM_DEBUG9:
			Generic->set_GameMode(false);
			Setup->set_start(HEIGHT/8,WIDTH/2);
			Setup->set_end(HEIGHT-HEIGHT/8,WIDTH/2);
			Setup->distance_method=MANHATTAN_DISTANCE;
			Setup->directions=8;
			Setup->iterations_per_frame=WIDTH*HEIGHT;
			Setup->use_terrain=false;
			//Setup->cost=1.0f;
			Setup->Map_StraightLine();
			update(hWnd);
			break;
		case IDM_DEBUG8:
			Generic->set_GameMode(false);
			Setup->set_start(10,10);
			Setup->set_end(HEIGHT-10,WIDTH-10);
			Setup->directions=8;
			Setup->iterations_per_frame=WIDTH*HEIGHT;
			Setup->use_terrain=false;
			//Setup->cost=0.5f;
			Setup->Map_BoxOnBox();
			update(hWnd);
			break;
		case IDM_DEBUG7:
			Generic->set_GameMode(false);
			Setup->set_start(10,10);
			Setup->set_end((BYTE)(HEIGHT-20),(BYTE)(WIDTH-20));
			Setup->iterations_per_frame=WIDTH*HEIGHT;
			Setup->use_terrain=true;
			Setup->Map_PipeMaze();
			update(hWnd);
			break;
		case IDM_DEBUG6:
			Generic->set_GameMode(false);
			Setup->set_start(10,10);
			Setup->set_end(HEIGHT-10,WIDTH-10);
			Setup->distance_method=MANHATTAN_DISTANCE;
			Setup->directions=8;
			Setup->iterations_per_frame=WIDTH*HEIGHT;
			Setup->use_terrain=false;
			Setup->Load("clutter.tga");
			update(hWnd);
			break;
		case IDM_DEBUG5:
			Generic->set_GameMode(false);
			Setup->set_start(10,10);
			Setup->set_end((BYTE)(HEIGHT-20),(BYTE)(WIDTH-20));
			Setup->iterations_per_frame=WIDTH*HEIGHT;
			Setup->use_terrain=true;
			Setup->Map_Random();
			update(hWnd);
			break;
		case IDM_DEBUG4:
			Setup->directions=8;
			Generic->set_GameMode(false);
			Setup->set_start(2,2);
			Setup->set_end(HEIGHT-2,WIDTH-2);
			Setup->iterations_per_frame=WIDTH*HEIGHT;
			Setup->use_terrain=false;
			Setup->Map_Clear_Path();
			update(hWnd);
			break;
		case IDM_DEBUG3:
			Setup->directions=8;
			Generic->set_GameMode(false);
			Setup->set_start(2,WIDTH-2);
			Setup->set_end(HEIGHT-2,2);
			Setup->iterations_per_frame=WIDTH*HEIGHT;
			Setup->use_terrain=false;
			Setup->Map_Clear_Path();
			update(hWnd);
			break;
		case IDM_DEBUG2:
			Setup->directions=8;
			Generic->set_GameMode(false);
			Setup->set_start(HEIGHT>>1,2);
			Setup->set_end(HEIGHT>>1,WIDTH-2);
			Setup->iterations_per_frame=WIDTH*HEIGHT;
			Setup->use_terrain=false;
			Setup->Map_Clear_Path();
			update(hWnd);
			break;
		case IDM_DEBUG1:
			Setup->directions=8;
			Generic->set_GameMode(false);
			Setup->set_start(10,10);
			Setup->set_end(10,11);
			Setup->iterations_per_frame=WIDTH*HEIGHT;
			Setup->use_terrain=false;
			Setup->Map_Clear_Path();
			update(hWnd);
			break;
			
			//
		case IDM_ALTDEBUG12:
			Generic->set_GameMode(false);
			Setup->set_start(10,10);
			Setup->set_end(190,190);
			Setup->distance_method=PYTHAGORAS_DISTANCE;
			Setup->directions=8;
			Setup->iterations_per_frame=WIDTH*HEIGHT;
			Setup->use_terrain=true;
			Setup->diagonal_cost=1.4f;
			Setup->Load("mars200x200.tga");
			update(hWnd);
			break;
		case IDM_ALTDEBUG11:
			Setup->directions=8;
			Generic->set_GameMode(false);
			Setup->set_start(2,10);
			Setup->set_end(252,244);
			Setup->iterations_per_frame=WIDTH*HEIGHT;
			Setup->use_terrain=false;
			Setup->Load("square_maze.tga");
			update(hWnd);
			break;
		case IDM_ALTDEBUG10:
			Setup->directions=8;
			Generic->set_GameMode(false);
			Setup->set_start(30,2);
			Setup->set_end(224,240);
			Setup->iterations_per_frame=WIDTH*HEIGHT;
			Setup->use_terrain=false;
			Setup->Load("unicursal_maze.tga");
			update(hWnd);
			break;
		case IDM_ALTDEBUG9:
			Setup->directions=8;
			Generic->set_GameMode(false);
			Setup->set_start(4,4);
			Setup->set_end(250,240);
			Setup->iterations_per_frame=WIDTH*HEIGHT;
			Setup->use_terrain=false;
			Setup->Load("hex_maze.tga");
			update(hWnd);
			break;
		case IDM_ALTDEBUG8:
			Setup->directions=8;
			Generic->set_GameMode(false);
			Setup->set_start(252,128);
			Setup->set_end(138,128);
			Setup->iterations_per_frame=WIDTH*HEIGHT;
			Setup->use_terrain=false;
			Setup->Load("labyrinth.tga");
			update(hWnd);
			break;
		case IDM_ALTDEBUG7:
			Setup->directions=8;
			Generic->set_GameMode(false);
			Setup->set_start(72,252);
			Setup->set_end(128,128);
			Setup->iterations_per_frame=WIDTH*HEIGHT;
			Setup->use_terrain=false;
			Setup->Load("circle_maze.tga");
			update(hWnd);
			break;
		case IDM_ALTDEBUG5:
			Generic->set_GameMode(false);
			Setup->starty=rand()%(HEIGHT-1);
			Setup->startx=rand()%(WIDTH-1);
			Setup->endy=rand()%(HEIGHT-1);
			Setup->endx=rand()%(WIDTH-1);
			Setup->iterations_per_frame=WIDTH*HEIGHT;
			Setup->use_terrain=true;
			Setup->Map_Random();
			update(hWnd);
			break;
		case IDM_ALTDEBUG1:
			Generic->set_GameMode(false);
			Setup->starty=3;
			Setup->startx=3;
			Setup->endy=WIDTH-20;
			Setup->endx=HEIGHT-45;
			Setup->iterations_per_frame=WIDTH*HEIGHT;
			Setup->directions=4;
			Setup->use_terrain=false;
			Setup->Map_Gates();
			update(hWnd);
			break;
			
			//
		case IDM_CTRLDEBUG12:
			Setup->distance_method=MANHATTAN_DISTANCE;
			Setup->directions=8;
			Setup->iterations_per_frame=WIDTH*HEIGHT;
			Setup->use_terrain=true;
			Setup->Load("clutter.tga");
			Generic->set_GameMode(true);
			update(hWnd);
			break;
		case IDM_CTRLDEBUG5:
			Generic->set_GameMode(true);
			Setup->starty=rand()%(HEIGHT-1);
			Setup->startx=rand()%(WIDTH-1);
			Setup->endy=rand()%(HEIGHT-1);
			Setup->endx=rand()%(WIDTH-1);
			Setup->iterations_per_frame=WIDTH*HEIGHT;
			Setup->use_terrain=true;
			Setup->Map_Random();
			update(hWnd);
			break;
			
			//
		case IDM_MAP_OPEN:
			{
				char strFile[256]="\0";
				char strFileTitle[256]="\0";
				
				LPOPENFILENAME lpofn=new OPENFILENAME;
				if(lpofn)
				{
					lpofn->lStructSize=sizeof(OPENFILENAME);
					lpofn->hwndOwner=hWnd;
					lpofn->hInstance=hInst;
					lpofn->lpstrFilter="*.tga";
					lpofn->lpstrCustomFilter=NULL;
					lpofn->nMaxCustFilter=0; 
					lpofn->nFilterIndex=0;
					lpofn->lpstrFile=strFile;
					lpofn->nMaxFile=256;
					lpofn->lpstrFileTitle=strFileTitle;
					lpofn->nMaxFileTitle=256;
					lpofn->lpstrInitialDir=NULL;
					lpofn->lpstrTitle="Load 256-color 200x200 TGA Map";
					lpofn->Flags=OFN_FILEMUSTEXIST;
					lpofn->nFileOffset=0;
					lpofn->nFileExtension=NULL;
					lpofn->lpstrDefExt=NULL; 
					lpofn->lCustData=0;
					lpofn->lpfnHook=NULL; 
					lpofn->lpTemplateName=NULL ;
					
					if(GetOpenFileName(lpofn))
					{
						Setup->Load(lpofn->lpstrFile);
						update(hWnd);
					}
					
					delete lpofn;
				}
			}
			break;
			
			//
		case IDM_MAP_SAVE:
			{
				char strFile[256]="\0";
				char strFileTitle[256]="\0";
				
				LPOPENFILENAME lpofn=new OPENFILENAME;
				if(lpofn)
				{
					lpofn->lStructSize=sizeof(OPENFILENAME);
					lpofn->hwndOwner=hWnd;
					lpofn->hInstance=hInst;
					lpofn->lpstrFilter="*.tga";
					lpofn->lpstrCustomFilter=NULL;
					lpofn->nMaxCustFilter=0; 
					lpofn->nFilterIndex=0;
					lpofn->lpstrFile=strFile;
					lpofn->nMaxFile=256;
					lpofn->lpstrFileTitle=strFileTitle;
					lpofn->nMaxFileTitle=256;
					lpofn->lpstrInitialDir=NULL;
					lpofn->lpstrTitle="Save 256-color 200x200 TGA Map";
					lpofn->Flags=OFN_FILEMUSTEXIST;
					lpofn->nFileOffset=0;
					lpofn->nFileExtension=NULL;
					lpofn->lpstrDefExt=NULL; 
					lpofn->lCustData=0;
					lpofn->lpfnHook=NULL; 
					lpofn->lpTemplateName=NULL ;
					
					if(GetSaveFileName(lpofn))
					{
						Setup->Save(lpofn->lpstrFile);
						Redraw(hWnd);
					}
					
					delete lpofn;
				}
			}
			break;
			
		case IDM_MAP_MARS200X200:
			Setup->Load("mars200x200.tga");
			update(hWnd);
			break;
		case IDM_MAP_BOXONBOXNOG:
			Setup->Map_BoxOnBoxNoG();
			update(hWnd);
			break;
		case IDM_MAP_BOXONBOX:
			Setup->Map_BoxOnBox();
			update(hWnd);
			break;
		case IDM_MAP_MARS:
			Setup->Load("mars.tga");
			update(hWnd);
			break;
		case IDM_MAP_CLUTTER:
			Setup->Load("clutter.tga");
			update(hWnd);
			break;
		case IDM_MAP_SWIRL:
			Setup->Load("swirl.tga");
			update(hWnd);
			break;
		case IDM_MAP_CHI:
			Setup->Load("chi.tga");
			update(hWnd);
			break;
		case IDM_MAP_AUTHOR:
			Setup->Load("author.tga");
			update(hWnd);
			break;
		case IDM_MAP_X:
			Setup->Load("x.tga");
			update(hWnd);
			break;
		case IDM_MAP_WIGGLINGSNAKE:
			Setup->Load("wigglingsnake.tga");
			update(hWnd);
			break;
		case IDM_MAP_HILL:
			Setup->Load("hill.tga");
			update(hWnd);
			break;
		case IDM_MAP_ISLANDS:
			Setup->Load("islands.tga");
			update(hWnd);
			break;
		case IDM_MAP_ISLAND:
			Setup->Load("island.tga");
			update(hWnd);
			break;

		//
		case IDM_MAP_STRAIGHTLINE:
			Setup->Map_StraightLine();
			update(hWnd);
			break;
		case IDM_MAP_CHECKERBOARD:
			Setup->Map_CheckerBoard();
			update(hWnd);
			break;
		case IDM_MAP_GRID:
			Setup->Map_Grid();
			update(hWnd);
			break;
		case IDM_MAP_PIPEMAZE:
			Setup->Map_PipeMaze();
			update(hWnd);
			break;
		case IDM_MAP_RANDOM:
			Setup->Map_Random();
			update(hWnd);
			break;
		case IDM_MAP_CLEAR_PATH:
			Setup->Map_Clear_Path();
			update(hWnd);
			break;
		case IDM_MAP_NO_PATH:
			Setup->Map_No_Path();
			update(hWnd);
			break;
		case IDM_MAP_CRASHME:
			Setup->Map_CrashMe();
			update(hWnd);
			break;
		case IDM_MAP_FLOWER:
			Setup->Load("flower.tga");
			update(hWnd);
			break;
		case IDM_MAP_BIGBOX:
			Setup->Map_BigBox();
			update(hWnd);
			break;
		case IDM_MAP_RANDOMBOXES:
			Setup->Map_RandomBoxes();
			update(hWnd);
			break;
		case IDM_MAP_RANDOMOPENBOXES:
			Setup->Map_RandomOpenBoxes();
			update(hWnd);
			break;
		case IDM_MAP_RANDOMTERRAIN:
			Setup->Map_RandomTerrain();
			update(hWnd);
			break;
		case IDM_MAP_GATES:
			Setup->Map_Gates();
			update(hWnd);
			break;
		case IDM_MAP_ZIGZAG:
			Setup->Map_ZigZag();
			update(hWnd);
			break;

			//
		case IDM_MAP_LABYRINTH:
			Setup->Load("labyrinth.tga");
			update(hWnd);
			break;
		case IDM_MAP_SQUARE_MAZE:
			Setup->Load("square_maze.tga");
			update(hWnd);
			break;
		case IDM_MAP_CIRCLE_MAZE:
			Setup->Load("circle_maze.tga");
			update(hWnd);
			break;
		case IDM_MAP_HEX_MAZE:
			Setup->Load("hex_maze.tga");
			update(hWnd);
			break;
		case IDM_MAP_UNICURSAL_MAZE:
			Setup->Load("unicursal_maze.tga");
			update(hWnd);
			break;
		case IDM_MAP_PERFECT_MAZE:
			Setup->Load("perfect_maze.tga");
			update(hWnd);
			break;
		case IDM_MAP_BRAID_MAZE:
			Setup->Load("braid_maze.tga");
			update(hWnd);
			break;
		case IDM_MAP_CRACK_MAZE:
			Setup->Load("crack_maze.tga");
			update(hWnd);
			break;
		case IDM_MAP_CAVERN_MAZE:
			Setup->Load("cavern_maze.tga");
			update(hWnd);
			break;
		case IDM_MAP_DIAGONAL_MAZE:
			Setup->Load("diagonal_maze.tga");
			update(hWnd);
			break;
		case IDM_MAP_SPIRAL_MAZE:
			Setup->Load("spiral_maze.tga");
			update(hWnd);
			break;
		case IDM_MAP_SEGMENT_MAZE:
			Setup->Load("segment_maze.tga");
			update(hWnd);
			break;
		case IDM_MAP_REPLICA_LABYRINTH:
			Setup->Load("replica_labyrinth.tga");
			update(hWnd);
			break;
		case IDM_MAP_CRETAN_LABYRINTH:
			Setup->Load("cretan_labyrinth.tga");
			update(hWnd);
			break;
		case IDM_MAP_MAN_IN_THE_MIDDLE_LABYRINTH:
			Setup->Load("man_in_the_middle_labyrinth.tga");
			update(hWnd);
			break;
			
			//
		case IDM_DIRECTIONS4:
			Setup->directions=4;
			Generic->UpdateSettings();
			Generic->Reset();
			Redraw(hWnd);
			break;
		case IDM_DIRECTIONS8:
			Setup->directions=8;
			Generic->UpdateSettings();
			Generic->Reset();
			Redraw(hWnd);
			break;
			
			//
		case IDM_EXIT:
			DestroyWindow(hWnd);
			break;
			
		default:
			return DefWindowProc(hWnd, message, wParam, lParam);
		}
		break;
		
		case WM_ERASEBKGND:
			return 1;
			
		case WM_PAINT:
			{
				//
				RefreshMenu(hWnd);
				
				// begin to repaint the screen....
				hdc = BeginPaint(hWnd, &ps);
				RECT rt;
				GetClientRect(hWnd, &rt);
				
				// first, since we don't want the screen to flicker or use slow-as-hell GDI
				//primatives to try to do the graphics rendering, we crate a temporary memory buffer
				// to do all our rendering to....
				hdctmp=CreateCompatibleDC(hdc);
				
				// now prepare to give it a directly accessable RGBA memory buffer
				//that we can draw to directly as a block of memory.
				bi.bV4BitCount = 32;
				bi.bV4ClrImportant = 0;
				bi.bV4ClrUsed = 0;
				bi.bV4V4Compression = BI_RGB;
				bi.bV4Planes = 1;
				bi.bV4Size = sizeof(BITMAPV4HEADER);
				bi.bV4SizeImage = 0; //(rt.right-rt.left)*(rt.bottom-rt.top);
				bi.bV4Width = (rt.right-rt.left);
				bi.bV4Height = -(rt.bottom-rt.top);
				bi.bV4XPelsPerMeter = 0;
				bi.bV4YPelsPerMeter = 0;
				bi.bV4AlphaMask = 0;
				bi.bV4CSType = 0;
				
				// get the DIB....
				hbmtmp = CreateDIBSection(hdc, (BITMAPINFO *) &bi, DIB_RGB_COLORS, (LPVOID *)&pwBits, NULL, 0);
				hOld = SelectObject(hdctmp, hbmtmp); //and select it to draw on.
				
				// We render the screen here to a block of DIB memoory we created...
				BITMAPV4HEADER biinfo;
				//int binfobytes=
				GetObject(hbmtmp,sizeof(BITMAPV4HEADER),&biinfo);
				// first however we check to make sure the user didn't try to resize the window
				// smaller than what we're trying to directly draw to.
				if(biinfo.bV4Width>=MINIMUM_CLIENT_WIDTH && biinfo.bV4Height>=MINIMUM_CLIENT_HEIGHT)
				{
					//do it, if all's ok.
					if(drawmode)
						DrawMode(pwBits,hdctmp,hWnd);
					else if(presearch_view)
						Presearch_View(pwBits,hdctmp,hWnd);
					else
					{
						Generic->Paint(pwBits,hdctmp,hWnd);
						if(zoom && !Generic->routing_view) Zoom_Composite(pwBits,hWnd);
					}
				}
				else // if they shrank the screen this would normally crash the app hard
					// so instead render another screen instructing them to reenlarge it.
					PaintFunc_TooSmall(hdctmp,hWnd,MINIMUM_CLIENT_WIDTH-biinfo.bV4Width,MINIMUM_CLIENT_HEIGHT-biinfo.bV4Height);
				
				// Now we copy what we rendered from our buffer to the screen as fast
				//as possible to avoid flickering.
				BitBlt(hdc,rt.left,rt.top,rt.right-rt.left,rt.bottom-rt.top,hdctmp,0,0,SRCCOPY);
				
				// un allocate the memory we grabbed.
				SelectObject(hdctmp,hOld);
				DeleteObject(hbmtmp);
				DeleteDC(hdctmp);
				
				// we're done
				EndPaint(hWnd, &ps);
			}
			break;
			
			//
		case WM_TIMER:
			if(runflag)
			{
				if(!Generic->isDone()) //if we're not pathfinding, keep the cpu's low
				{
					Generic->FindPath();
					Redraw(hWnd);
				}
			}
			break;
			
		case WM_DESTROY:
			PostQuitMessage(0);
			KillTimer(hWnd, TIMER_ID);
			break;
			
		default:
			return DefWindowProc(hWnd, message, wParam, lParam);
	}
	return 0;
}

// Mesage handler for about box.
LRESULT CALLBACK About(HWND hDlg, UINT message, WPARAM wParam, LPARAM lParam)
{
	lParam=lParam;
	switch (message)
	{
	case WM_INITDIALOG:
		PlaySound(MAKEINTRESOURCE(IDR_WAVE_ABOUT), GetModuleHandle(NULL), SND_RESOURCE | SND_ASYNC);
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
