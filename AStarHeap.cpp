// AStarHeap.cpp: implementation of the AStarHeap class.
//
//////////////////////////////////////////////////////////////////////

/*
AStarHeap version 3
*/
#include "stdafx.h"
#include "AStarHeap.h"

#include "math.h" //sqrt
#include "stdio.h" //for sprintf

//#pragma optimize( "yawgt", on )


//////////////////////////////////////////////////////////////////////
// Construction/Destruction
//////////////////////////////////////////////////////////////////////

CAStarHeap::CAStarHeap()
{
	Setup=NULL;
	airoute=NULL;
}

CAStarHeap::CAStarHeap(CSetup *Setup, AIROUTE *airoute)
{
	this->Setup=Setup;
	this->airoute=airoute;
	
	GetOptions();
	UpdateSettings();
	UpdateWorld();
	Reset(airoute);
	
	// empty node
	nodes[0].yx=0;
	nodes[0].f=0.0f;
	nodes[0].g=0.0f;
	
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

CAStarHeap::~CAStarHeap()
{
	
}

void CAStarHeap::Reset(AIROUTE *airoute)
{
	//
	bigtick.QuadPart=0;
	LARGE_INTEGER tmp1;
	QueryPerformanceCounter(&tmp1);
	
	this->airoute=airoute;
	airoute->count=0;
	
	// opt me
	_WORLD * pworld=world;
	int n=WIDTH*HEIGHT;
	do {
		pworld->state=(pworld->terrain_cost==IMPASSABLE_TERRAIN_COST);
		pworld++;
	} while(--n>0);
	
	//
	frame=0;
	
	pathing_state=FINDING;
	path_found=false;
	no_path=false;
	
	//
	open_nodes=1;
	closed_nodes=0;
	
	// open node see
	best_f_node=1;
	nodes[best_f_node].yx=startyx;
	nodes[best_f_node].g=0.0f;
	nodes[best_f_node].f=nodes[best_f_node].g+(WORD)((float)distance(startyx)*DXY[4].cost_multiplier);
	world[startyx].route=NO_ROUTE;
	
	//closed node seed
	closed_node=0;
	
	free_node=1;
	
	//
	heap[0]=EMPTY_NODE;
	last_heap_leaf=1;
	heap[last_heap_leaf]=best_f_node;

	//
	LARGE_INTEGER tmp2;
	QueryPerformanceCounter(&tmp2);
	bigtick.QuadPart += tmp2.QuadPart - tmp1.QuadPart;
}






///////////////////////////////////////////////////////////////////////////
// heap priority queue code
// note: make non-recursive
inline int CAStarHeap::LEFT(int k)
{
	return k<<1; //k*2;
}
inline int CAStarHeap::RIGHT(int k)
{
	return (k<<1)+1; //k*2+1;
}
inline int CAStarHeap::PARENT(int k)
{
	return (k>>1); //k/2;
}
inline bool CAStarHeap::NOTEMPTY_UP(int k)
{
	return k!=0;
}
inline bool CAStarHeap::NOTEMPTY_DOWN(int k)
{
	return k<=last_heap_leaf;
}
inline void CAStarHeap::swap_heap(const int k1, const int k2)
{
	WORD tmp=heap[k1];
	heap[k1]=heap[k2];
	heap[k2]=tmp;
}
//
void CAStarHeap::remove_root_from_heap()
{
	// move last leaf to the root and delte the last leaf
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
			if(nodes[heap[leftk]].f < nodes[heap[rightk]].f)
				bestk=leftk;
			else
				bestk=rightk;
		}
		else if(NOTEMPTY_DOWN(leftk))
			bestk=leftk;
		else
			break;
		
		if(nodes[heap[bestk]].f < nodes[heap[k]].f)
		{
			swap_heap(k,bestk);
			k=bestk;
		}
		else
			break;
	}
}
//
void CAStarHeap::insert_node_to_heap(WORD node)
{
	// add new leaf unless the tree is full.
	// in that case prune worst leaf
	//(ie, remove the very last leaf before adding another).
	if(last_heap_leaf<MAX_HEAP_LEAFS)
		last_heap_leaf++;
	
	//
	heap[last_heap_leaf]=node;
	
	// move new k up through heap to closest level
	int k=last_heap_leaf;
	while(NOTEMPTY_UP(k))
	{
		int parentk=PARENT(k);
		if(NOTEMPTY_UP(parentk))
		{
			if(nodes[heap[k]].f < nodes[heap[parentk]].f)
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

//
void CAStarHeap::FindPath()
{
	if(pathing_state)
		return;
	if(
		(Setup->presearch_toggle && Setup->world[startyx>>YSHIFT][startyx&XMASK].group!=Setup->world[endyx>>YSHIFT][endyx&XMASK].group) ||
		world[startyx].terrain_cost==IMPASSABLE_TERRAIN_COST ||
		world[endyx].terrain_cost==IMPASSABLE_TERRAIN_COST
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
	open_nodes=last_heap_leaf;
	closed_nodes=(WORD)(free_node-last_heap_leaf);
}

//
void CAStarHeap::without_terrain()
{
	int count=iterations_per_frame;
	do
	{
		closed_node=best_f_node=heap[ROOT_HEAP]; //parent_node
		
		// PART A close node
		_NODES *pparent_node=nodes+best_f_node;
		if(pparent_node->yx==endyx) pathing_state|=PATH_FOUND; // did we find the goal?
		world[ pparent_node->yx ].state=CLOSED; // map the world map with closed state
		remove_root_from_heap(); //
		
		// PART B open nodes
		for(BYTE d=0;d<directions;d++)
		{
			// figure the [y][x] offset into the world map
			WORD yx=pparent_node->yx+DXY[d].yx;
			_WORLD *pworld=world+yx;
			
			if(pworld->state==UNKNOWN) //if we've never visited this [y][x] before...
			{
				pworld->state=OPEN; //mark [y][x] of world map as part of an open node
				pworld->route=d; //store routing info for use later in packroute...
				
				free_node++; //cycle up to the next unused slot for a node
				// since we don't actually delete nodes when we close them there's no need to
				// search unused nodes out.
				
				_NODES *pfree_node=nodes+free_node;
				
				pfree_node->yx=yx; // mark the [y][x] position of this node for the world map
				
				// figure what the new f and g will be and store directly to [].f and [].g
				pfree_node->g=pparent_node->g + DXY[d].cost_multiplier;
				pfree_node->f=pfree_node->g+distance(yx);
				
				// insert this new node into the binary heap
				insert_node_to_heap(free_node);
			}
		}
		
		if(last_heap_leaf<=0) pathing_state|=NO_PATH;
	} while(--count>0 && !pathing_state);
	
	//
	frame+=(iterations_per_frame-count); //
}


//
void CAStarHeap::with_terrain()
{
	int count=iterations_per_frame;
	do
	{
		closed_node=best_f_node=heap[ROOT_HEAP]; //parent_node
		
		// PART A close node
		_NODES *pparent_node=nodes+best_f_node;
		if(pparent_node->yx==endyx) pathing_state|=PATH_FOUND; // did we find the goal?
		world[ pparent_node->yx ].state=CLOSED; // map the world map with closed state
		remove_root_from_heap(); //
		
		// PART B open nodes
		for(BYTE d=0;d<directions;d++)
		{
			// figure the [y][x] offset into the world map
			WORD yx=pparent_node->yx+DXY[d].yx;
			_WORLD *pworld=world+yx;
			
			if(pworld->state==UNKNOWN) //if we've never visited this [y][x] before...
			{
				pworld->state=OPEN; //mark [y][x] of world map as part of an open node
				pworld->route=d; //store routing info for use later in packroute...
				
				free_node++; //cycle up to the next unused slot for a node
				// since we don't actually delete nodes when we close them there's no need to
				// search unused nodes out.
				
				_NODES *pfree_node=nodes+free_node;
				
				pfree_node->yx=yx; // mark the [y][x] position of this node for the world map
				
				// figure what the new f and g will be and store directly to [].f and [].g
				pfree_node->g=pparent_node->g + (float)pworld->terrain_cost*DXY[d].cost_multiplier;
				pfree_node->f=pfree_node->g+distance(yx);
				
				// insert this new node into the binary heap
				insert_node_to_heap(free_node);
			}
		}
		
		if(last_heap_leaf<=0) pathing_state|=NO_PATH;
	} while(--count>0 && !pathing_state);
	
	//
	frame+=(iterations_per_frame-count); //
}


// note: removing switchs and just using manhattan saves a grand total of .5ms on
// the test machine. :)
inline float CAStarHeap::distance(const WORD yx)
{
	switch(distance_method)
	{
	case MANHATTAN_DISTANCE:
		{
			return (float)(abs((yx&XMASK)-endx) + abs((yx>>YSHIFT)-endy));
		}
		break;
		
	case PYTHAGORAS_DISTANCE: //aka STRAIGHT LINE, pythagoras
		{
			int xpart=(yx&XMASK)-endx;
			int ypart=(yx>>YSHIFT)-endy;
			return (float)sqrt((float)(xpart*xpart + ypart*ypart));
		}
		break;
		
	case DIAGONAL_DISTANCE:
		{
			float a=(float)abs((yx&XMASK)-endx);
			float b=(float)abs((yx>>YSHIFT)-endy);
			return (a>b)?a:b;
		}
		break;
		
	case SIMPLE_PYTHAGORAS_DISTANCE:
		{
			int xpart=(yx&XMASK)-endx;
			int ypart=(yx>>YSHIFT)-endy;
			return (float)(xpart*xpart + ypart*ypart);
		}
		break;
	default: return 0.0f;
	}
}







///////////////////////////////////////////////////////////////////////////
///////////////////////////////////////////////////////////////////////////
///////////////////////////////////////////////////////////////////////////
// to make it easier to add many new Setups to patherfinder.cpp we use Setup.cpp 
// class as a holder for all general settings and world info.

// tells Setup.cpp what menu options should be enabled/disabled for this alg.
void CAStarHeap::GetOptions()
{
	Setup->options.uniform_cost=true;
	Setup->options.terrain_cost=true;
	Setup->options.distance=true;
	Setup->options.search_directions=true;
}

// this transfers the world map from Setup.cpp to the internal state here.
void CAStarHeap::UpdateWorld()
{
	LARGE_INTEGER tmp1;
	QueryPerformanceCounter(&tmp1);
	
	//
	for(int y=0;y<HEIGHT;y++)
	{
		for(int x=0;x<WIDTH;x++)
		{
			WORD yx=(y<<YSHIFT)+x;
			world[yx].terrain_cost=Setup->world[y][x].terrain_cost;
			world[yx].state=(world[yx].terrain_cost==IMPASSABLE_TERRAIN_COST); //?IMPASSABLE:UNKNOWN;
		}
	}
	
	//
	LARGE_INTEGER tmp2;
	QueryPerformanceCounter(&tmp2);
	bigtick.QuadPart += tmp2.QuadPart - tmp1.QuadPart;
}

// this transfers the all the settings in Setup.cpp to here.
void CAStarHeap::UpdateSettings()
{
	startyx=Setup->starty*WIDTH+Setup->startx;
	endyx=Setup->endy*WIDTH+Setup->endx;
	
	endx=endyx&XMASK;
	endy=(endyx>>YSHIFT);
	
	use_terrain=Setup->use_terrain;
	distance_method=Setup->distance_method;
	
	iterations_per_frame=Setup->iterations_per_frame;
	
	directions=Setup->directions;
	
	float cost=Setup->cost;
	float diagonal_cost=Setup->diagonal_cost;
	float median_terrain_cost=Setup->median_terrain_cost;
	
	int n;
	
	if(use_terrain)
	{
		for(n=0;n<4;n++)
		{
			DXY[n].cost_multiplier=median_terrain_cost;
		}
			for(n=4;n<8;n++)
			{
				DXY[n].cost_multiplier=diagonal_cost*median_terrain_cost;
			}
	}
	else
	{
		for(n=0;n<4;n++)
		{
			DXY[n].cost_multiplier=1.0f*cost;
		}
			for(n=4;n<8;n++)
			{
				DXY[n].cost_multiplier=diagonal_cost*cost;
			}
	}
}



///////////////////////////////////////////////////////////////////////////
// TYPE B
void CAStarHeap::PackRoute()
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
		route=world[yx].route;
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
void CAStarHeap::Paint(LPBYTE pwBits, HDC hdc,HWND hWnd)
{
	if(pwBits)
	{
		//
		RECT rt;
		GetClientRect(hWnd, &rt);
		int clientwidth = (rt.right-rt.left);
		//		int clientheight = (rt.bottom-rt.top);
		
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
		int node;
		int count;
		int n;
		register _RGBA *tmp;
		
		//
		int used_nodes=0;
		float smallest_f, smallest_g, smallest_h;
		float largest_f, largest_g, largest_h;
		smallest_f = smallest_g = smallest_h = 1048576.0f;
		largest_f = largest_g = largest_h = 0.0001f;
		for(node=1;node<=free_node;node++)
		{
			if(nodes[node].f>largest_f) largest_f=nodes[node].f;
			if(nodes[node].f<smallest_f) smallest_f=nodes[node].f;
			if(nodes[node].g>largest_g) largest_g=nodes[node].g;
			if(nodes[node].g<smallest_g) smallest_g=nodes[node].g;
			float h=nodes[node].f-nodes[node].g;
			if(h>largest_h) largest_h=h;
			if(h<smallest_h) smallest_h=h;
			used_nodes++;
		}
		
		//
		int length=0;
		p=pbegin;
		yx=nodes[best_f_node].yx;
		while(yx!=startyx && yx>0 && yx<yxmax)
		{
			yx+=DXY[DXY[world[yx].route].route].yx;
			length++;
			if(length>MAX_NODE) break;
		}
		
		
		// stats
		GetClientRect(hWnd, &rt);
		sprintf(szStatusLine,
			"-- A* Heap (v3) --\nf %f ... %f\ng %f ... %f\nh %f ... %f\n\nstruct size %d bytes\n(nodes %d B)\n(heap %d B)\n(world %d B)\n[used node %d B]\n[used heap %d B]\n\nMAX NODES %d\nBest Node\n%4d\nf %f\ng %f\nh %f\nClosed Node\n%4d\nf %f\ng %f\nh %f\n\npath found? %s\nlength= %d\n\nstart -> current\nf %f -> %f\ng %f -> %f\nh %f -> %f\n%s\n",
			smallest_f, largest_f,
			smallest_g, largest_g,
			smallest_h, largest_h,
			sizeof(nodes) + sizeof(world) + sizeof(heap),
			sizeof(nodes),
			sizeof(heap),
			sizeof(world),
			used_nodes*(sizeof(_NODES)),
			last_heap_leaf*(sizeof(WORD)),
			MAX_NODE,
			best_f_node,
			nodes[best_f_node].f,
			nodes[best_f_node].g,
			nodes[best_f_node].f-nodes[best_f_node].g,
			closed_node,
			nodes[closed_node].f,
			nodes[closed_node].g,
			nodes[closed_node].f-nodes[closed_node].g,
			pathing_state&NO_PATH?"NO PATH!":(pathing_state&PATH_FOUND?"YES!":"Not yet..."),
			length,
			nodes[1].f,nodes[closed_node].f,
			nodes[1].g,nodes[closed_node].g,
			nodes[1].f-nodes[1].g,nodes[closed_node].f-nodes[closed_node].g,
			(nodes[closed_node].f<=nodes[1].f)?"ADMISSIBLE":"not admissible"
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
				BYTE b=(BYTE)(255-(world[y*WIDTH+x].terrain_cost<<4));
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
		yx=nodes[best_f_node].yx;
		while(yx!=startyx && yx>0 && yx<yxmax)
		{
			tmp=(p+(yx>>YSHIFT)*clientwidth+(yx&XMASK));
			tmp->red=255;
			tmp->green=0;
			tmp->blue=0;
			yx+=DXY[DXY[world[yx].route].route].yx;
			if(length>MAX_NODE) break;
		}
		
		
		// f
		GetClientRect(hWnd, &rt);
		rt.top=HEIGHT;
		rt.left=WIDTH+4;
		rt.right=WIDTH+4+WIDTH;
		sprintf(szStatusLine,"f (red closed/blue open)");
		DrawText(hdc, szStatusLine, strlen(szStatusLine), &rt, DT_CENTER);
		p=pbegin+(WIDTH+4);
		count=MAX_NODE;
		for(node=1;node<=free_node;node++)
		{
			tmp=(p+(nodes[node].yx>>YSHIFT)*clientwidth+(nodes[node].yx&XMASK));
			tmp->red=(BYTE)((nodes[node].f/largest_f)*255.0f);
			tmp->green=0;
			tmp->blue=0;
		};
		for(n=1;n<=last_heap_leaf;n++)
		{
			node=heap[n];
			tmp=(p+(nodes[node].yx>>YSHIFT)*clientwidth+(nodes[node].yx&XMASK));
			tmp->red=0;
			tmp->green=0;
			tmp->blue=(BYTE)((nodes[node].f/largest_f)*255.0f);
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
		sprintf(szStatusLine,"open nodes %d (%d heap levels)",last_heap_leaf,levels);
		DrawText(hdc, szStatusLine, strlen(szStatusLine), &rt, DT_CENTER);
		p=pbegin+(HEIGHT+16)*clientwidth;
		for(n=1;n<=last_heap_leaf;n++)
		{
			node=heap[n];
			
			BYTE f=(BYTE)((nodes[node].f/largest_f)*255.0f);
			BYTE g=(BYTE)((nodes[node].g/largest_g)*255.0f);
			BYTE h=(BYTE)(((nodes[node].f-nodes[node].g)/largest_h)*255.0f);
			tmp=(p+(nodes[node].yx>>YSHIFT)*clientwidth+(nodes[node].yx&XMASK));
			tmp->red=h;
			tmp->green=g;
			tmp->blue=f;
		}
		
		
		// closed
		GetClientRect(hWnd, &rt);
		rt.top=HEIGHT*2+16;
		rt.left=WIDTH+4;
		rt.right=WIDTH+4+WIDTH;
		sprintf(szStatusLine,"closed nodes %d",closed_nodes);
		DrawText(hdc, szStatusLine, strlen(szStatusLine), &rt, DT_CENTER);
		p=pbegin+WIDTH+4+(HEIGHT+16)*clientwidth;
		for(node=1;node<=free_node;node++)
		{
			BYTE f=(BYTE)((nodes[node].f/largest_f)*255.0f);
			BYTE g=(BYTE)((nodes[node].g/largest_g)*255.0f);
			BYTE h=(BYTE)(((nodes[node].f-nodes[node].g)/largest_h)*239.0f);
			tmp=(p+(nodes[node].yx>>YSHIFT)*clientwidth+(nodes[node].yx&XMASK));
			tmp->red=h;
			tmp->green=g;
			tmp->blue=f;
		}
		for(n=1;n<=last_heap_leaf;n++) //erase
		{
			node=heap[n];
			DWORD *tmp=((DWORD *)p+(nodes[node].yx>>YSHIFT)*clientwidth+(nodes[node].yx&XMASK));
			*tmp=(DWORD)background;
		}
		
		
		// state
		GetClientRect(hWnd, &rt);
		rt.top=HEIGHT;
		rt.left=(WIDTH+4)*2;
		rt.right=(WIDTH+4)*2+WIDTH;
		sprintf(szStatusLine,"state (g=open b=closed r=impass)");
		DrawText(hdc, szStatusLine, strlen(szStatusLine), &rt, DT_CENTER);
		p=pbegin+(WIDTH+4)*2;
		for(y=0;y<HEIGHT;y++)
		{
			for(x=0;x<WIDTH;x++)
			{
				yx=y*WIDTH+x;
				tmp=(p+y*clientwidth+x);
				tmp->green=(BYTE)((world[yx].state==OPEN)?255:0);
				tmp->blue=(BYTE)((world[yx].state==CLOSED)?255:0);
				tmp->red=(BYTE)((world[yx].state==IMPASSABLE)?255:0);
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
				switch(world[(y<<YSHIFT)+x].route)
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
