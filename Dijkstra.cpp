// Dijkstra.cpp: implementation of the Dijkstra class.
//
//////////////////////////////////////////////////////////////////////

#include "stdafx.h"
#include "Dijkstra.h"

#include "stdio.h" //for sprintf


//////////////////////////////////////////////////////////////////////
// Construction/Destruction
//////////////////////////////////////////////////////////////////////

CDijkstra::CDijkstra()
{
	Setup=NULL;
	airoute=NULL;
}

CDijkstra::CDijkstra(CSetup *Setup, AIROUTE *airoute)
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

CDijkstra::~CDijkstra()
{
	
}

void CDijkstra::Reset(AIROUTE *airoute)
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
		world[yx].cost=0.0f;
	}
	
	for(int n=0;n<=MAX_OPEN_NODES;n++)
	{
		nodes[n].yx=0;
		nodes[n].cost=0.0f;
		nodes[n].prev=EMPTY_NODE; //n-1;
		nodes[n].next=EMPTY_NODE; //n+1;
	}
	
	//
	open_nodes=1;
	closed_nodes=0;
	
	//
	best_node=1;
	nodes[best_node].yx=startyx;
	nodes[best_node].cost=0;
	nodes[best_node].prev=best_node;
	nodes[best_node].next=best_node;
	world[startyx].route=NO_ROUTE;
	
	//
	LARGE_INTEGER tmp2;
	QueryPerformanceCounter(&tmp2);
	bigtick.QuadPart += tmp2.QuadPart - tmp1.QuadPart;
}

//
void CDijkstra::FindPath()
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
		add_nodes_to_open();
		move_best_node_to_closed();
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
inline void CDijkstra::add_nodes_to_open()
{
	WORD parentyx=nodes[best_node].yx;
	
	for(int d=0;d<directions;d++)
	{
		WORD yx=parentyx+DXY[d].yx;
		float cost_modifier=(d&4)?diagonal_cost:1.0f;
		
		if(
			(world[yx].terrain_cost==IMPASSABLE_TERRAIN_COST) || //if elevation is high mountains we can't travel there
			(world[yx].state!=UNKNOWN) //already open
			)
			;
		else
		{
			//
			world[yx].state=OPEN;
			world[yx].route=DXY[d].route;
			
			// what is the cost of this new node?
			float newcost;
			if(use_terrain)
				newcost=nodes[best_node].cost+(float)world[yx].terrain_cost*cost_modifier;
			else
				newcost=nodes[best_node].cost+cost*cost_modifier;
			
			WORD sorted_node=find_sorted_node_position(newcost);
			WORD free_node=find_free_node_for_add();
			
			// add C node after sorted_node
			nodes[free_node].yx=yx;
			nodes[free_node].cost=newcost;
			
			// rechain A + B nodes to place C inbetween
			// A : B
			// A : C : B
			// sorted-1 : sorted
			// sorted-1 : free  : sorted
			WORD prev=nodes[sorted_node].prev;
			nodes[prev].next=free_node;
			nodes[free_node].prev=prev;
			nodes[free_node].next=sorted_node;
			nodes[sorted_node].prev=free_node;
			
			//
			open_nodes++;
		}
	}
}

//
inline void CDijkstra::move_best_node_to_closed()
{
	world[ nodes[best_node].yx ].state=CLOSED;
	world[ nodes[best_node].yx ].cost=nodes[best_node].cost;
	
	if(endyx==nodes[best_node].yx)
		path_found=true;
	else
	{
		free_best_node(); //
		closed_nodes++; //
		if(--open_nodes==0) no_path=true;
	}
}

//
inline WORD CDijkstra::find_free_node_for_add()
{
	if(open_nodes>=MAX_OPEN_NODES) { no_path=true; return EMPTY_NODE; }
	
	//
	WORD free_node=best_node;
	while(nodes[free_node].next!=EMPTY_NODE)
	{
		if(--free_node<1) free_node=(WORD)MAX_OPEN_NODES;
	}
	
	return free_node;
}

//
inline void CDijkstra::free_best_node()
{
	WORD prev=nodes[best_node].prev;
	WORD next=nodes[best_node].next;
	
	nodes[best_node].prev=EMPTY_NODE;
	nodes[best_node].next=EMPTY_NODE;
	
	//
	nodes[prev].next=next;
	nodes[next].prev=prev;
	
	//
	best_node=next;
}

//
inline WORD CDijkstra::find_sorted_node_position(float newcost)
{
	//
	WORD sorted_node=best_node;
	while(
		newcost>=nodes[sorted_node].cost &&
		sorted_node!=nodes[best_node].prev
		)
		sorted_node=nodes[sorted_node].next;
	return sorted_node;
}








///////////////////////////////////////////////////////////////////////////
///////////////////////////////////////////////////////////////////////////
///////////////////////////////////////////////////////////////////////////
// to make it easier to add many new Setups to patherfinder.cpp we use Setup.cpp 
// class as a holder for all general settings and world info.

// tells Setup.cpp what menu options should be enabled/disabled for this alg.
void CDijkstra::GetOptions()
{
	Setup->options.uniform_cost=true;
	Setup->options.terrain_cost=true;
	Setup->options.distance=false;
	Setup->options.search_directions=true;
}

// to make it easier to add many new Setups to patherfinder.cpp we use Setup.cpp 
// class as a holder for all general settings and world info.
// this transfers the world map from Setup.cpp to the internal state here.
void CDijkstra::UpdateWorld()
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

// to make it easier to add many new Setups to patherfinder.cpp we use Setup.cpp 
// class as a holder for all general settings and world info.
// this transfers the all the settings in Setup.cpp to here.
void CDijkstra::UpdateSettings()
{
	startyx=Setup->starty*WIDTH+Setup->startx;
	endyx=Setup->endy*WIDTH+Setup->endx;
	
	use_terrain=Setup->use_terrain;
	cost=Setup->cost;
	diagonal_cost=Setup->diagonal_cost;
	
	iterations_per_frame=Setup->iterations_per_frame;
	
	directions=Setup->directions;
}



///////////////////////////////////////////////////////////////////////////
// TYPE A
void CDijkstra::PackRoute()
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
void CDijkstra::Paint(LPBYTE pwBits,HDC hdc,HWND hWnd)
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
		int n;
		int yx;
		register _RGBA *tmp;
		
		//
		int length=0;
		if(Setup->okToPath && path_found)
		{
			int route;
			yx=endyx;
			while(yx!=startyx)
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
		}
		
		// statistics
		sprintf(szStatusLine,
			"-- Dijkstra --\nMAX_OPEN_NODES %d\nbest node %d ( %d ) %d)\n\npath found? %s\nlength= %d\n",
			MAX_OPEN_NODES,
			nodes[best_node].prev, best_node, nodes[best_node].next,
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
		
		p=pbegin; //starting point
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
		// ending point
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
			for(n=1;n<=MAX_OPEN_NODES;n++)
			{
				if(nodes[n].next!=EMPTY_NODE)
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
			if(Setup->okToPath)
			{
			p=pbegin;
			yx=endyx;
			while(yx!=startyx)
			{
				tmp=(p+(yx>>YSHIFT)*clientwidth+(yx&XMASK));
				tmp->red=255;
				tmp->green=0;
				tmp->blue=0;
				yx+=DXY[world[yx].route].yx;
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
		for(n=1;n<=MAX_OPEN_NODES;n++)
		{
			if(nodes[n].next!=EMPTY_NODE)
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
				case 0: tmp->red=0; tmp->green=0;   tmp->blue=255;   break; // n
				case 1: tmp->red=127; tmp->green=0; tmp->blue=255;   break; // e
				case 2: tmp->red=255;   tmp->green=0; tmp->blue=0;   break; // s
				case 3: tmp->red=255; tmp->green=0; tmp->blue=127; break; // w pink
					
				case 4: tmp->red=0;   tmp->green=127;   tmp->blue=255; break; // ne
				case 5: tmp->red=127;   tmp->green=127; tmp->blue=255; break; // se
				case 6: tmp->red=255;   tmp->green=127; tmp->blue=0;   break; // sw
				case 7: tmp->red=255; tmp->green=127; tmp->blue=127;   break; // nw
					
				default: tmp->red=0;  tmp->green=0;   tmp->blue=0;   break; //NO_ROUTE
				};
			}
		}

		
		// cost
		float maxcost=0.0f;
		yx=WIDTH*HEIGHT-1;
		do
		{
			if(world[yx].cost>maxcost)
				maxcost=world[yx].cost;
		} while(--yx>=0);
		GetClientRect(hWnd, &rt);
		rt.top=HEIGHT*2+16;
		rt.left=(WIDTH+4)*2;
		rt.right=(WIDTH+4)*2+WIDTH;
		sprintf(szStatusLine,"cost (max %f)",maxcost);
		DrawText(hdc, szStatusLine, strlen(szStatusLine), &rt, DT_CENTER);
		p=pbegin+(WIDTH+4)*2+clientwidth*(HEIGHT+16);
		for(y=0;y<HEIGHT;y++)
		{
			for(x=0;x<WIDTH;x++)
			{
				tmp=(p+y*clientwidth+x);
				tmp->red=0;
				tmp->green=(BYTE) ((world[(y<<YSHIFT)+x].cost/maxcost) *255.0f);
				tmp->blue=0;
			}
		}
		
		
		// state
		GetClientRect(hWnd, &rt);
		rt.top=HEIGHT;
		rt.left=(WIDTH+4)*2;
		rt.right=(WIDTH+4)*2+WIDTH;
		sprintf(szStatusLine,"state");
		DrawText(hdc, szStatusLine, strlen(szStatusLine), &rt, DT_CENTER);
		p=pbegin+(WIDTH+4)*2;
		for(y=0;y<HEIGHT;y++)
		{
			for(x=0;x<WIDTH;x++)
			{
				tmp=(p+y*clientwidth+x);
				switch(world[(y<<YSHIFT)+x].state)
				{
				case UNKNOWN: tmp->red=0; tmp->green=0; tmp->blue=255; break;
				case OPEN: tmp->red=255; tmp->green=255; tmp->blue=255; break;
				case CLOSED: tmp->red=255; tmp->green=0; tmp->blue=0; break;
				}
			}
		}
	}
}
