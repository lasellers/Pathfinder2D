// AStar.cpp: implementation of the AStar class.
//
//////////////////////////////////////////////////////////////////////

#include "stdafx.h"
#include "AStar.h"

#include "math.h" //sqrt
#include "stdio.h" //for sprintf

//#pragma optimize( "yawgt", on )

//////////////////////////////////////////////////////////////////////
// Construction/Destruction
//////////////////////////////////////////////////////////////////////

CAStar::CAStar()
{
	Setup=NULL;
	airoute=NULL;
}

CAStar::CAStar(CSetup *Setup, AIROUTE *airoute)
{
	//
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

CAStar::~CAStar()
{
	
}

/*
*/
void CAStar::Reset(AIROUTE *airoute)
{
	//
	bigtick.QuadPart=0;
	LARGE_INTEGER tmp1;
	QueryPerformanceCounter(&tmp1);
	
	this->airoute=airoute;
	airoute->count=0;
	
	// opt me
	_WORLD* pworld=world;
	int n=WIDTH*HEIGHT;
	do {
		pworld->u.raw&=0xf;
		pworld++;
	} while(--n>0);
	
	//
	frame=0;
	
	pathing_state=FINDING;
	path_found=false;
	no_path=false;
	
	//
	closed_yx=EMPTY_NODE;
	
	//
	heap[0].node=EMPTY_NODE;
	heap[0].f=0;
	heap[0].g=0;
	
	last_heap_leaf=1;
	heap[last_heap_leaf].node=startyx;
	
	heap[last_heap_leaf].g=0;
	heap[last_heap_leaf].f=heap[last_heap_leaf].g+distance(startyx)*DXY[4].cost_multiplier;
	world[startyx].u.state.route=NO_ROUTE;
	
	//
	LARGE_INTEGER tmp2;
	QueryPerformanceCounter(&tmp2);
	bigtick.QuadPart += tmp2.QuadPart - tmp1.QuadPart;
}





///////////////////////////////////////////////////////////////////////////
// heap priority queue code
// note: make non-recursive
inline int CAStar::LEFT(int k)
{
	return k<<1; //k*2;
}
inline int CAStar::RIGHT(int k)
{
	return (k<<1)+1; //k*2+1;
}
inline int CAStar::PARENT(int k)
{
	return (k>>1); //k/2;
}
inline bool CAStar::NOTEMPTY_UP(int k)
{
	return k!=0;
}
inline bool CAStar::NOTEMPTY_DOWN(int k)
{
	return k<=last_heap_leaf;
}
inline void CAStar::swap_heap(const int k1, const int k2)
{ // swaps node,f and g as quickly as possible
	register _HEAP *heap1=&heap[k1];
	register _HEAP *heap2=&heap[k2];
	register WORD tmpnode;
	register DWORD tmp;
	
	tmpnode=heap1->node;
	tmp=*(DWORD *)heap1; //swap f & g at same time
	
	heap1->node=heap2->node;
	*heap1=*heap2;
	
	heap2->node=tmpnode;
	*(DWORD *)heap2=tmp;
}
// note: slowest function, speed up
inline void CAStar::remove_root_from_heap()
{
	// move last leaf to the root and delete the last leaf
	heap[ROOT_HEAP]=heap[last_heap_leaf--];
	
	//move transposed k back down level to appropriate place
	int k=ROOT_HEAP;
	while(NOTEMPTY_DOWN(k))
	{
		int leftk=LEFT(k);
		int rightk=RIGHT(k);
		int bestk;
		if(NOTEMPTY_DOWN(leftk) && NOTEMPTY_DOWN(rightk) )
		{
			bestk=(heap[leftk].f < heap[rightk].f)?leftk:rightk;
		}
		else if(NOTEMPTY_DOWN(leftk))
			bestk=leftk;
		else
			break;
		
		if(heap[bestk].f < heap[k].f)
		{
			swap_heap(k,bestk);
			k=bestk;
		}
		else
			break;
	}
}
//
// note: 2nd slowest function, speed up
inline void CAStar::insert_node_to_heap(const WORD node, const WORD g)
{
	// add new leaf unless the tree is full.
	// in that case prune worst leaf
	//(ie, remove the very last leaf before adding another).
	if(last_heap_leaf<MAX_HEAP_LEAFS)
		last_heap_leaf++;
	
	//
	heap[last_heap_leaf].node=node;
	heap[last_heap_leaf].g=g;
	heap[last_heap_leaf].f=g+distance(node);
	
	// move new k up through heap to closest level
	int k=last_heap_leaf;
	while(NOTEMPTY_UP(k))
	{
		int parentk=PARENT(k);
		if(NOTEMPTY_UP(parentk))
		{
			if(heap[k].f < heap[parentk].f)
			{
				swap_heap(k,parentk);
				k=parentk;
			}
			else
				break;
		}
		else
			break;
	}
}





///////////////////////////////////////////////////////////////////////////

/*
*/
void CAStar::FindPath()
{
	//
	if(pathing_state)
		return;
	if(
		(Setup->presearch_toggle && Setup->world[startyx>>YSHIFT][startyx&XMASK].group!=Setup->world[endyx>>YSHIFT][endyx&XMASK].group) ||
		world[startyx].u.state.terrain_cost==IMPASSABLE_TERRAIN_COST ||
		world[endyx].u.state.terrain_cost==IMPASSABLE_TERRAIN_COST
		)
		pathing_state=NO_PATH;
	else
	{
		//
		LARGE_INTEGER tmp1;
		QueryPerformanceCounter(&tmp1);
		
		//
		if(use_terrain)
			with_terrain();
		else
			without_terrain();
		
		//
		if(pathing_state) PackRoute();
		
		//
		LARGE_INTEGER tmp2;
		QueryPerformanceCounter(&tmp2);
		bigtick.QuadPart += tmp2.QuadPart - tmp1.QuadPart;
		
		Setup->frame=frame;
		Setup->bigtick.QuadPart=bigtick.QuadPart;
	}
	
	//
	path_found=(pathing_state&PATH_FOUND)==PATH_FOUND;
	no_path=(pathing_state&NO_PATH)==NO_PATH;
}

/*
*/
void CAStar::without_terrain()
{
	int count=iterations_per_frame;
	do
	{
		closed_yx=heap[ROOT_HEAP].node;
		WORD best_g=heap[ROOT_HEAP].g;
		
		// PART A close node
		remove_root_from_heap(); //
		// did we find the goal?
		if(closed_yx==endyx) { pathing_state|=PATH_FOUND; break; }
		
		// PART B open world
		for(BYTE d=0;d<directions;d++)
		{
			// figure the [y][x] offset into the world map
			WORD yx=closed_yx+DXY[d].yx;
			_WORLD *pworld=world+yx;
			
			//if we've never visited this [y][x] before...
			if( (pworld->u.raw&0x0f)!=0x0f && (pworld->u.raw&0x10)!=0x10 )
			{
				//mark [y][x] of world map as part of an open node
				//store routing info for use later in packroute...
				pworld->u.raw|=16|(d<<5); 
				
				// figure what the new f and g will be and pass on to heap insert...
				// insert this new node into the binary heap
				insert_node_to_heap(yx,best_g + DXY[d].cost_multiplier);
			}
		}
		
		if(last_heap_leaf<=0) pathing_state|=NO_PATH;
	} while(--count>0 && !pathing_state);
	
	//
	frame+=(iterations_per_frame-count); //
}


/*
*/
void CAStar::with_terrain()
{
	int count=iterations_per_frame;
	do
	{
		closed_yx=heap[ROOT_HEAP].node;
		WORD best_g=heap[ROOT_HEAP].g;
		
		// PART A close node
		remove_root_from_heap(); //
		// did we find the goal?
		if(closed_yx==endyx) { pathing_state|=PATH_FOUND; break; }
		
		// PART B open world
		for(BYTE d=0;d<directions;d++)
		{
			// figure the [y][x] offset into the world map
			WORD yx=closed_yx+DXY[d].yx;
			_WORLD *pworld=world+yx;
			
			//if we've never visited this [y][x] before...
			if( (pworld->u.raw&0x0f)!=0x0f && (pworld->u.raw&0x10)!=0x10 )
			{
				//mark [y][x] of world map as part of an open node
				//store routing info for use later in packroute...
				pworld->u.raw|=16|(d<<5); 
				
				// figure what the new f and g will be and pass on to heap insert...
				// insert this new node into the binary heap
				insert_node_to_heap(yx,best_g + pworld->u.state.terrain_cost*DXY[d].cost_multiplier);
			}
		}
		
		if(last_heap_leaf<=0) pathing_state|=NO_PATH;
	} while(--count>0 && !pathing_state);
	
	//
	frame+=(iterations_per_frame-count); //
}


// manhattan
inline WORD CAStar::distance(const WORD yx)
{
	return (WORD)(abs((yx&XMASK)-endx) + abs((yx>>YSHIFT)-endy));
}




///////////////////////////////////////////////////////////////////////////
///////////////////////////////////////////////////////////////////////////
///////////////////////////////////////////////////////////////////////////
// to make it easier to add many new Setups to patherfinder.cpp we use Setup.cpp 
// class as a holder for all general settings and world info.

// tells Setup.cpp what menu options should be enabled/disabled for this alg.
void CAStar::GetOptions()
{
	Setup->options.uniform_cost=true;
	Setup->options.terrain_cost=true;
	Setup->options.distance=false;
	Setup->options.search_directions=true;
}

// this transfers the world map from Setup.cpp to the internal state here.
void CAStar::UpdateWorld()
{
	LARGE_INTEGER tmp1;
	QueryPerformanceCounter(&tmp1);
	
	//
	for(int y=0;y<HEIGHT;y++)
	{
		for(int x=0;x<WIDTH;x++)
		{
			WORD yx=(y<<YSHIFT)+x;
			world[yx].u.state.terrain_cost=Setup->world[y][x].terrain_cost;
		}
	}
	
	//
	LARGE_INTEGER tmp2;
	QueryPerformanceCounter(&tmp2);
	bigtick.QuadPart += tmp2.QuadPart - tmp1.QuadPart;
}

// this transfers the all the settings in Setup.cpp to here.
void CAStar::UpdateSettings()
{
	startyx=Setup->starty*WIDTH+Setup->startx;
	endyx=Setup->endy*WIDTH+Setup->endx;
	
	endx=endyx&XMASK;
	endy=(endyx>>YSHIFT);
	
	use_terrain=Setup->use_terrain;
	//	distance_method=Setup->distance_method;
	
	iterations_per_frame=Setup->iterations_per_frame;
	
	directions=Setup->directions;
	
	float cost=Setup->cost;
	float diagonal_cost=Setup->diagonal_cost;
	float median_terrain_cost=Setup->median_terrain_cost;
	
	int n;
	
	//note: fix me somehow to handle diags
	if(use_terrain)
	{
		for(n=0;n<4;n++)
		{
			DXY[n].cost_multiplier=(WORD)median_terrain_cost;
		}
		for(n=4;n<8;n++)
		{
			DXY[n].cost_multiplier=(WORD)(diagonal_cost*median_terrain_cost);
		}
	}
	else
	{
		for(n=0;n<4;n++)
		{
			DXY[n].cost_multiplier=(WORD)cost;
		}
		for(n=4;n<8;n++)
		{
			DXY[n].cost_multiplier=(WORD)(diagonal_cost*cost);
		}
	}
}



///////////////////////////////////////////////////////////////////////////
// TYPE B
void CAStar::PackRoute()
{
	if(pathing_state&NO_PATH)
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
		//		route=DXY[world[yx].route].route;
		route=world[yx].u.state.route;
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
void CAStar::Paint(LPBYTE pwBits, HDC hdc,HWND hWnd)
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
		int starty=startyx>>YSHIFT;
		int startx=startyx&XMASK;
		int endy=endyx>>YSHIFT;
		int endx=endyx&XMASK;
		
		TCHAR szStatusLine[1024];
		_RGBA *p,*ppush;
		_RGBA *pbegin=(_RGBA *)pwBits;
		const int yxmax=WIDTH*HEIGHT;
		int y,x;
		int yx;
		int n;
		register _RGBA *tmp;
		
		//
		int used_nodes=0;
		WORD smallest_f, smallest_g, smallest_h;
		WORD largest_f, largest_g, largest_h;
		smallest_f = smallest_g = smallest_h = 0xffff; //1048576.0f;
		largest_f = largest_g = largest_h = 1; //0.0001f;
		for(n=1;n<=last_heap_leaf;n++)
		{
			if(heap[n].f>largest_f) largest_f=heap[n].f;
			if(heap[n].f<smallest_f) smallest_f=heap[n].f;
			
			if(heap[n].g>largest_g) largest_g=heap[n].g;
			if(heap[n].g<smallest_g) smallest_g=heap[n].g;
			
			WORD h=heap[n].f-heap[n].g;
			if(h>largest_h) largest_h=h;
			if(h<smallest_h) smallest_h=h;
			
			used_nodes++;
		}
		
		//
		int length=0;
		p=pbegin;
		yx=heap[ROOT_HEAP].node;
		while(yx!=startyx && yx>0 && yx<yxmax)
		{
			yx+=DXY[DXY[world[yx].u.state.route].route].yx;
			length++;
			if(length>MAX_NODE) break;
		}

		
		
		// stats
		GetClientRect(hWnd, &rt);
		sprintf(szStatusLine,
			"-- A* Heap, Integer, No Closed (v4) --\nf %d ... %d\ng %d ... %d\nh %d ... %d\n\nstruct size %d bytes\n(heap %d B)\n(world %d B)\n[used heap %d B]\nMAX NODES %d\n\nBest Node %4d\nf %d\ng %d\nh %d\n\npath found? %s\nlength= %d\n",
			smallest_f, largest_f,
			smallest_g, largest_g,
			smallest_h, largest_h,
			sizeof(world) + sizeof(heap),
			sizeof(heap),
			sizeof(world),
			last_heap_leaf*(sizeof(WORD)),
			MAX_NODE,
			heap[ROOT_HEAP].node,
			heap[ROOT_HEAP].f,
			heap[ROOT_HEAP].g,
			heap[ROOT_HEAP].f-heap[ROOT_HEAP].g,
			pathing_state&NO_PATH?"NO PATH!":(pathing_state&PATH_FOUND?"YES!":"Not yet..."),
			length
			);
		DrawText(hdc, szStatusLine, strlen(szStatusLine), &rt, DT_RIGHT);
		
		
		//shortest-path w/ terrain
		GetClientRect(hWnd, &rt);
		rt.top=HEIGHT;
		rt.right=WIDTH;
		sprintf(szStatusLine,"shortest path w/terrain");
		DrawText(hdc, szStatusLine, strlen(szStatusLine), &rt, DT_CENTER);
		p=ppush=pbegin;
		for(y=0;y<HEIGHT;y++)
		{
			for(x=0;x<WIDTH;x++)
			{
				BYTE b=(BYTE)(255-(world[y*WIDTH+x].u.state.terrain_cost<<4));
				p->red=b; p->green=b; p->blue=b;
				p++;
			}
			ppush+=clientwidth;
			p=ppush;
		}
		for(y=-1;y<=1;y++) //starting point
		{
			for(x=-1;x<=1;x++)
			{
				tmp=(pbegin+(starty+y)*clientwidth+startx+x);
				tmp->red=0;
				tmp->green=255;
				tmp->blue=0;
			}
		}
		for(y=-1;y<=1;y++) //end/goal point
		{
			for(x=-1;x<=1;x++)
			{
				tmp=(pbegin+(endy+y)*clientwidth+endx+x);
				tmp->red=255;
				tmp->green=0;
				tmp->blue=0;
			}
		}
		p=pbegin;
		yx=closed_yx;
		while(yx!=startyx && yx>0 && yx<yxmax)
		{
			tmp=(p+(yx>>YSHIFT)*clientwidth+(yx&XMASK));
			tmp->red=255;
			tmp->green=0;
			tmp->blue=0;
			yx+=DXY[DXY[world[yx].u.state.route].route].yx;
			if(length>MAX_NODE) break;
		}
		
		
		// open
		int levels=0;
		{
			int b=1;
			int sum=b;
			while(b<=last_heap_leaf)
			{
				b=b<<1;
				sum+=b;
				levels++;
			}
		}
		GetClientRect(hWnd, &rt);
		rt.top=HEIGHT*2+16;
		rt.left=0;
		rt.right=WIDTH;
		sprintf(szStatusLine,"open %d (%d heap levels)",last_heap_leaf,levels);
		DrawText(hdc, szStatusLine, strlen(szStatusLine), &rt, DT_CENTER);
		rt.top+=16;
		sprintf(szStatusLine,"f=blue g=green h=red");
		DrawText(hdc, szStatusLine, strlen(szStatusLine), &rt, DT_CENTER);
		p=pbegin+(HEIGHT+16)*clientwidth;
		for(n=1;n<=last_heap_leaf;n++)
		{
			WORD node=heap[n].node;
			
			BYTE f=(BYTE)(((float)heap[n].f/(float)largest_f)*255.0f);
			BYTE g=(BYTE)(((float)heap[n].g/(float)largest_g)*255.0f);
			BYTE h=(BYTE)(((float)(heap[n].f-heap[n].g)/(float)largest_h)*255.0f);
			tmp=(p+(node>>YSHIFT)*clientwidth+(node&XMASK));
			tmp->red=h;
			tmp->green=g;
			tmp->blue=f;
		}
		
		
		// visited
		GetClientRect(hWnd, &rt);
		rt.top=HEIGHT;
		rt.left=(WIDTH+4)*2;
		rt.right=(WIDTH+4)*2+WIDTH;
		sprintf(szStatusLine,"visited (green) / impassable (red)");
		DrawText(hdc, szStatusLine, strlen(szStatusLine), &rt, DT_CENTER);
		p=pbegin+(WIDTH+4)*2;
		for(y=0;y<HEIGHT;y++)
		{
			for(x=0;x<WIDTH;x++)
			{
				yx=y*WIDTH+x;
				tmp=(p+y*clientwidth+x);
				tmp->green=(BYTE)((world[yx].u.state.visited)?255:0);
				tmp->blue=0;
				tmp->red=(BYTE)((world[yx].u.state.terrain_cost==IMPASSABLE_TERRAIN_COST)?255:0);
			}
		}
		
		
		//routing
		GetClientRect(hWnd, &rt);
		rt.top=HEIGHT*2+16;
		rt.left=(WIDTH+4)*2;
		rt.right=(WIDTH+4)*2+WIDTH;
		sprintf(szStatusLine,"routes");
		DrawText(hdc, szStatusLine, strlen(szStatusLine), &rt, DT_CENTER);
		p=pbegin+(WIDTH+4)*2+(HEIGHT+16)*clientwidth;
		for(y=0;y<HEIGHT;y++)
		{
			for(x=0;x<WIDTH;x++)
			{
				tmp=(p+y*clientwidth+x);
				switch(world[(y<<YSHIFT)+x].u.state.route)
				{
				case 0: tmp->red=0;		tmp->green=0;	tmp->blue=255;	break; // n
				case 1: tmp->red=127;	tmp->green=0;	tmp->blue=255;	break; // e
				case 2: tmp->red=255;	tmp->green=0;	tmp->blue=0;	break; // s
				case 3: tmp->red=255;	tmp->green=0;	tmp->blue=127;	break; // w pink
				case 4: tmp->red=0;		tmp->green=255;	tmp->blue=255;	break; // ne
				case 5: tmp->red=127;	tmp->green=255;	tmp->blue=255;	break; // se
				case 6: tmp->red=255;	tmp->green=255;	tmp->blue=0;	break; // sw
				case 7: tmp->red=255;	tmp->green=255;	tmp->blue=127;	break; // nw
				};
			}
		}
	}
}
