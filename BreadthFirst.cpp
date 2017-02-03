// BreadthFirst.cpp: implementation of the BreadthFirst class.
//
//////////////////////////////////////////////////////////////////////

#include "stdafx.h"
#include "BreadthFirst.h"

#include "stdio.h" //for sprintf


//////////////////////////////////////////////////////////////////////
// Construction/Destruction
//////////////////////////////////////////////////////////////////////

CBreadthFirst::CBreadthFirst()
{
	Setup=NULL;
	airoute=NULL;
}

CBreadthFirst::CBreadthFirst(CSetup *Setup, AIROUTE *airoute)
{
	this->Setup=Setup;
	this->airoute=airoute;
	
	GetOptions();
	UpdateSettings();
	UpdateWorld();
	Reset(airoute);
	
	// make the routing lookup table
	DXY[0].yx=-WIDTH;	DXY[0].route=2;
	DXY[1].yx=1;		DXY[1].route=3;
	DXY[2].yx=WIDTH;	DXY[2].route=0;
	DXY[3].yx=-1;		DXY[3].route=1;
	
	DXY[4].yx=-WIDTH+1;	DXY[4].route=6;
	DXY[5].yx=WIDTH+1;	DXY[5].route=7;
	DXY[6].yx=WIDTH-1;	DXY[6].route=4;
	DXY[7].yx=-WIDTH-1;	DXY[7].route=5;
	
	//in case a NO_ROUTE accidentally is passed for lookup
	DXY[8].yx=0;		DXY[8].route=0;
}

CBreadthFirst::~CBreadthFirst()
{
	
}

void CBreadthFirst::Reset(AIROUTE *airoute)
{
	//
	bigtick.QuadPart=0;
	LARGE_INTEGER tmp1;
	QueryPerformanceCounter(&tmp1);
	
	this->airoute=airoute;
	airoute->count=0;
	
	//
	frame=0;
	
	path_found=false;
	no_path=false;
	
	//
	for(int yx=0;yx<WIDTH*HEIGHT;yx++)
	{
		world[yx].state=UNKNOWN;
		world[yx].route=NO_ROUTE;
	}
	
	for(int n=0;n<=MAX_OPEN_NODES;n++)
		nodes[n].yx=0;
	
	//
	open_nodes=1;
	closed_nodes=0;
	
	//
	first_node=last_node=1;
	nodes[first_node].yx=startyx;
	world[startyx].route=NO_ROUTE;
	
	//
	LARGE_INTEGER tmp2;
	QueryPerformanceCounter(&tmp2);
	bigtick.QuadPart += tmp2.QuadPart - tmp1.QuadPart;
}


//
void CBreadthFirst::FindPath()
{
	if(path_found || no_path)
		return;
	if(
		(Setup->presearch_toggle && Setup->world[startyx>>YSHIFT][startyx&XMASK].group!=Setup->world[endyx>>YSHIFT][endyx&XMASK].group) ||
		world[startyx].terrain_cost==IMPASSABLE_TERRAIN_COST ||
		world[endyx].terrain_cost==IMPASSABLE_TERRAIN_COST
		)
	{
		no_path=true;
		return;
	}
	
	//
	LARGE_INTEGER tmp1;
	QueryPerformanceCounter(&tmp1);
	
	//
	int count=iterations_per_frame; //if we didn't introduce a set "count" to abort on, the function would proceed to solve the entire thing
	do
	{
		//open_successor_nodes
		expand_node_to_open();
		move_first_node_to_closed();
	} while(--count>0 && !path_found && !no_path);
	
	//
	frame+=(iterations_per_frame-count);
	
	//
	if(path_found || no_path) PackRoute();
	
	//
	LARGE_INTEGER tmp2;
	QueryPerformanceCounter(&tmp2);
	bigtick.QuadPart += tmp2.QuadPart - tmp1.QuadPart;
	
	Setup->frame=frame;
	Setup->bigtick.QuadPart=bigtick.QuadPart;
}

// most often called function -- must be as fast as possible
inline void CBreadthFirst::expand_node_to_open()
{
	WORD parentyx=nodes[first_node].yx;
	
	for(register int d=0;d<directions;d++)
	{
		WORD yx=(WORD)(parentyx+DXY[d].yx);
		
		if(
			(world[yx].terrain_cost==IMPASSABLE_TERRAIN_COST) || //if elevation is high mountains we can't travel there
			(world[yx].state!=UNKNOWN) //already open
			)
			;
		else
		{
			//
			world[yx].state=CLOSED;
			world[yx].route=DXY[d].route;
			
			if(++last_node>=MAX_OPEN_NODES) last_node=1; //
			
			nodes[last_node].yx=yx; //
			
			open_nodes++; //
		}
	}
}

//
inline void CBreadthFirst::move_first_node_to_closed()
{
	world[ nodes[first_node].yx ].state=CLOSED;
	
	if(endyx==nodes[first_node].yx)
		path_found=true;
	else
	{
		if(++first_node>=MAX_OPEN_NODES) first_node=1; //
		closed_nodes++; //
		if(--open_nodes==0) no_path=true;
	}
}









///////////////////////////////////////////////////////////////////////////
///////////////////////////////////////////////////////////////////////////
///////////////////////////////////////////////////////////////////////////
// to make it easier to add many new Setups to patherfinder.cpp we use Setup.cpp 
// class as a holder for all general settings and world info.

// tells Setup.cpp what menu options should be enabled/disabled for this alg.
void CBreadthFirst::GetOptions()
{
	Setup->options.uniform_cost=false;
	Setup->options.terrain_cost=false;
	Setup->options.distance=false;
	Setup->options.search_directions=true;
}

// this transfers the world map from Setup.cpp to the internal state here.
void CBreadthFirst::UpdateWorld()
{
	LARGE_INTEGER tmp1;
	QueryPerformanceCounter(&tmp1);
	
	//
	for(int y=0;y<HEIGHT;y++)
	{
		for(int x=0;x<WIDTH;x++)
		{
			world[(y<<YSHIFT)+x].terrain_cost=Setup->world[y][x].terrain_cost;
		}
	}
	
	//
	LARGE_INTEGER tmp2;
	QueryPerformanceCounter(&tmp2);
	bigtick.QuadPart += tmp2.QuadPart - tmp1.QuadPart;
}

// this transfers the all the settings in Setup.cpp to here.
void CBreadthFirst::UpdateSettings()
{
	startyx=Setup->starty*WIDTH+Setup->startx;
	endyx=Setup->endy*WIDTH+Setup->endx;
	
	iterations_per_frame=Setup->iterations_per_frame;
	
	directions=Setup->directions;
}



///////////////////////////////////////////////////////////////////////////
// TYPE A
void CBreadthFirst::PackRoute()
{
	if(no_path)
	{
		airoute->count=0;
		return;
	}
	
	//
	memset(airoute->route,0,MAX_ROUTES); //clear routes
	
	//
	airoute->active=1;
	airoute->compression=0;
	
	//
	WORD yx=endyx;
	int start=MAX_ROUTES-1;
	BYTE route=NO_ROUTE;
	while(yx!=startyx)
	{
		route=DXY[world[yx].route].route;
		//		route=world[yx].route;
		yx+=DXY[DXY[route].route].yx;
		airoute->route[start]=route;
		if(--start<0) start=MAX_ROUTES-1;
	};
	
	airoute->start=start+1;
	airoute->count=MAX_ROUTES-airoute->start;
	
	//
	airoute->startyx=startyx;
	airoute->endyx=endyx;
	
	//
	airoute->walk_point=airoute->start;
	airoute->walk_runlength_step=0;
	
	//     
	if(airoute->start==0) airoute->count=0;
}




///////////////////////////////////////////////////////////////////////////
///////////////////////////////////////////////////////////////////////////
///////////////////////////////////////////////////////////////////////////
/*
This is the function that paints the graphics to the screen.
Called by PathFinder.
Localized to respective Setup classes v1.6.
*/
//
void CBreadthFirst::Paint(LPBYTE pwBits, HDC hdc,HWND hWnd)
{
	if(pwBits)
	{
		//
		RECT rt;
		GetClientRect(hWnd, &rt);
		int clientwidth = (rt.right-rt.left);
		
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
		_RGBA *p,*ppush;
		_RGBA *pbegin=(_RGBA *)pwBits;
		int y,x;
		int yx;
		int n;
		register _RGBA *tmp;
		
		//
		int length=0;
		int route;
		yx=endyx;
		while(!(yx==startyx))
		{
			route=world[yx].route;
			if(route==NO_ROUTE)
				break;
			else
			{
				yx+=DXY[route].yx;
				length++;
			}
		}
		
		
		// statistics
		sprintf(szStatusLine,
			"-- Breadth-First --\nMAX_OPEN_NODES %d\nfirst node %d\nlast node %d\nspan %d\n\npath found? %s\nlength= %d\n",
			MAX_OPEN_NODES,
			first_node,
			last_node,
			(last_node-first_node),
			no_path?"NO PATH!":(path_found?"YES!":"Not yet..."),
			length
			);
		DrawText(hdc, szStatusLine, strlen(szStatusLine), &rt, DT_RIGHT);
		
		
		// composite
		GetClientRect(hWnd, &rt);
		rt.top=HEIGHT;
		rt.right=WIDTH;
		sprintf(szStatusLine,"shortest path");
		DrawText(hdc, szStatusLine, strlen(szStatusLine), &rt, DT_CENTER);
		p=ppush=pbegin;
		for(y=0;y<HEIGHT;y++)
		{
			for(x=0;x<WIDTH;x++)
			{
				BYTE b=(BYTE)(255-(world[(y<<YSHIFT)+x].terrain_cost<<4));
				p->red=b; p->green=b; p->blue=b;
				p++;
			}
			ppush+=clientwidth;
			p=ppush;
		}

		//start dot
		p=pbegin;
		for(y=-1;y<=1;y++)
		{
			for(x=-1;x<=1;x++)
			{
				tmp=(p+((startyx>>YSHIFT)+y)*clientwidth+(startyx&XMASK)+x);
				tmp->red=0;
				tmp->green=255;
				tmp->blue=0;
			}
		}
		//end goal dot
		for(y=-1;y<=1;y++)
		{
			for(x=-1;x<=1;x++)
			{
				tmp=(p+((endyx>>YSHIFT)+y)*clientwidth+(endyx&XMASK)+x);
				tmp->red=255;
				tmp->green=0;
				tmp->blue=0;
			}
		}

		if(!path_found)
		{
			p=pbegin;
			int a=first_node;
			int b=last_node;
			if(a<=b)
			{
				for(n=a;n<=b;n++)
				{
					tmp=(p+(nodes[n].yx>>YSHIFT)*clientwidth+(nodes[n].yx&XMASK));
					tmp->red=255;
					tmp->green=0;
					tmp->blue=0;
				}
			}
			else
			{
				for(n=1;n<=b;n++)
				{
					tmp=(p+(nodes[n].yx>>YSHIFT)*clientwidth+(nodes[n].yx&XMASK));
					tmp->red=255;
					tmp->green=0;
					tmp->blue=0;
				}
				for(n=a;n<=MAX_OPEN_NODES;n++)
				{
					tmp=(p+(nodes[n].yx>>YSHIFT)*clientwidth+(nodes[n].yx&XMASK));
					tmp->red=255;
					tmp->green=0;
					tmp->blue=0;
				}
			}
		}
		else
		{
			p=pbegin;
			yx=endyx;
			while(!(yx==startyx))
			{
				tmp=(p+(yx>>YSHIFT)*clientwidth+(yx&XMASK));
				tmp->red=255;
				tmp->green=0;
				tmp->blue=0;
				
				route=world[yx].route;
				if(route==NO_ROUTE)
					break;
				else
				{
					yx+=DXY[route].yx;
				}
			}
		}
		
		
		// open
		GetClientRect(hWnd, &rt);
		rt.top=HEIGHT*2+16;
		rt.left=0;
		rt.right=WIDTH;
		sprintf(szStatusLine,"open nodes %d",open_nodes);
		DrawText(hdc, szStatusLine, strlen(szStatusLine), &rt, DT_CENTER);
		p=pbegin+(HEIGHT+16)*clientwidth;
		int a=first_node;
		int b=last_node;
		if(a<=b)
		{
			for(n=a;n<=b;n++)
			{
				tmp=(p+(nodes[n].yx>>YSHIFT)*clientwidth+(nodes[n].yx&XMASK));
				tmp->red=128;
				tmp->green=255;
				tmp->blue=128;
			}
		}
		else
		{
			for(n=1;n<=b;n++)
			{
				tmp=(p+(nodes[n].yx>>YSHIFT)*clientwidth+(nodes[n].yx&XMASK));
				tmp->red=128;
				tmp->green=255;
				tmp->blue=128;
			}
			for(n=a;n<=MAX_OPEN_NODES;n++)
			{
				tmp=(p+(nodes[n].yx>>YSHIFT)*clientwidth+(nodes[n].yx&XMASK));
				tmp->red=128;
				tmp->green=255;
				tmp->blue=128;
			}
		}
		
		
		// closed
		GetClientRect(hWnd, &rt);
		rt.top=HEIGHT*2+16;
		rt.left=WIDTH+4;
		rt.right=WIDTH+4+WIDTH;
		sprintf(szStatusLine,"closed nodes %d",closed_nodes);
		DrawText(hdc, szStatusLine, strlen(szStatusLine), &rt, DT_CENTER);
		p=pbegin+WIDTH+4+(HEIGHT+16)*clientwidth;
		for(y=0;y<HEIGHT;y++)
		{
			for(x=0;x<WIDTH;x++)
			{
				if(world[(y<<YSHIFT)+x].state==CLOSED)
				{
					tmp=(p+y*clientwidth+x);
					tmp->red=128;
					tmp->green=255;
					tmp->blue=128;
				}
			}
		}
		
		
		// route
		GetClientRect(hWnd, &rt);
		rt.top=HEIGHT;
		rt.left=(WIDTH+4);
		rt.right=(WIDTH+4)+WIDTH;
		sprintf(szStatusLine,"route");
		DrawText(hdc, szStatusLine, strlen(szStatusLine), &rt, DT_CENTER);
		p=pbegin+(WIDTH+4);
		for(y=0;y<HEIGHT;y++)
		{
			for(x=0;x<WIDTH;x++)
			{
				tmp=(p+y*clientwidth+x);
				switch(world[(y<<YSHIFT)+x].route)
				{
				case 0: tmp->red=0;   tmp->green=0;   tmp->blue=255; break; // n
				case 1: tmp->red=127; tmp->green=0;   tmp->blue=255; break; // e
				case 2: tmp->red=255; tmp->green=0;   tmp->blue=0;   break; // s
				case 3: tmp->red=255; tmp->green=0;   tmp->blue=127; break; // w pink
					
				case 4: tmp->red=0;   tmp->green=127; tmp->blue=255; break; // ne
				case 5: tmp->red=127; tmp->green=127; tmp->blue=255; break; // se
				case 6: tmp->red=255; tmp->green=127; tmp->blue=0;   break; // sw
				case 7: tmp->red=255; tmp->green=127; tmp->blue=127; break; // nw
					
				default: tmp->red=0;  tmp->green=0;   tmp->blue=0;   break; //NO_ROUTE
				};
			}
		}
	}
}
