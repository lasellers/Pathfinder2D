// AStarComplete.cpp: implementation of the AStarComplete class.
//
//////////////////////////////////////////////////////////////////////
/*
AStar Version 2c w/complete path revisiting

Sorted Linked Lists
*/

#include "stdafx.h"
#include "AStarComplete.h"

#include "math.h" //sqrt
#include "stdio.h" //for sprintf


//////////////////////////////////////////////////////////////////////
// Construction/Destruction
//////////////////////////////////////////////////////////////////////

CAStarComplete::CAStarComplete()
{
	Setup=NULL;
	airoute=NULL;
}

CAStarComplete::CAStarComplete(CSetup *Setup, AIROUTE *airoute)
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

CAStarComplete::~CAStarComplete()
{
	
}

void CAStarComplete::Reset(AIROUTE *airoute)
{
	//
	bigtick.QuadPart=0;
	LARGE_INTEGER tmp1;
	QueryPerformanceCounter(&tmp1);

	this->airoute=airoute;
	airoute->count=0;
	
	// opt me
	for(register int n=0;n<WIDTH*HEIGHT;n++)
	{
		world[n].state=(world[n].terrain_cost==IMPASSABLE_TERRAIN_COST); //?IMPASSABLE:UNKNOWN;
		world[n].node=EMPTY_NODE;
	}
	
	//
	frame=0;
	
	path_found=false;
	no_path=false;
	
	//
	open_nodes=1;
	closed_nodes=0;
	reopened_nodes=0;
	
	//
	best_node=1;
	nodesdata[best_node].yx=startyx;
	nodesdata[best_node].g=0.0f;
	if(use_terrain)
		nodes[best_node].f=nodesdata[best_node].g+distance(startyx)*median_terrain_cost;
	else
		nodes[best_node].f=nodesdata[best_node].g+distance(startyx);
	nodes[best_node].prev=best_node;
	nodes[best_node].next=best_node;
	nodesdata[best_node].ancestor=EMPTY_NODE;

	world[startyx].route=NO_ROUTE;
	world[startyx].node=best_node;
	
	//closed node seed
	closed_node=0;
	nodes[closed_node].prev=EMPTY_NODE;
	nodes[closed_node].next=EMPTY_NODE;
	nodesdata[closed_node].ancestor=EMPTY_NODE;
	
	free_node=1;
	
	//
	LARGE_INTEGER tmp2;
	QueryPerformanceCounter(&tmp2);
	bigtick.QuadPart += tmp2.QuadPart - tmp1.QuadPart;
}


//
void CAStarComplete::FindPath()
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
	if(use_terrain)
		with_terrain(iterations_per_frame);
	else
		without_terrain(iterations_per_frame);
	
	//
	if(path_found || no_path) PackRoute();
	
	//
	LARGE_INTEGER tmp2;
	QueryPerformanceCounter(&tmp2);
	bigtick.QuadPart += tmp2.QuadPart - tmp1.QuadPart;
	
	Setup->frame=frame;
	Setup->bigtick.QuadPart=bigtick.QuadPart;
}

void CAStarComplete::without_terrain(int iterations_per_frame)
{
	int count=iterations_per_frame;
	do
	{
		WORD parent_node=best_node;
		expand_nodes_to_open(parent_node); // open successor nodes
		move_node_to_closed(parent_node);// close spawning parent node
	} while(--count>0 && !path_found && !no_path);
	frame+=(iterations_per_frame-count);
}

void CAStarComplete::with_terrain(int iterations_per_frame)
{
	int count=iterations_per_frame;
	do
	{
		WORD parent_node=best_node;
		expand_nodes_to_open_terrain(parent_node); // open successor nodes
		move_node_to_closed(parent_node);// close spawning parent node
	} while(--count>0 && !path_found && !no_path);
	frame+=(iterations_per_frame-count);
}

// most often called function -- must be as fast as possible
inline void CAStarComplete::expand_nodes_to_open(WORD parent_node)
{
	for(int d=0;d<directions;d++)
	{
		//
		WORD yx=nodesdata[parent_node].yx+DXY[d].yx;
		
		if(world[yx].state==UNKNOWN)
		{
			//
			world[yx].state=OPEN;
			world[yx].route=d;
			
			free_node++;
			world[yx].node=free_node;
			
			nodesdata[free_node].yx=yx;
			
			nodesdata[free_node].g=nodesdata[parent_node].g + cost*DXY[d].cost_multiplier;
			nodes[free_node].f=nodesdata[free_node].g+distance(yx);
			
			register WORD sorted_node=find_sorted_node_position(nodes[free_node].f);
			
			if(nodes[free_node].f<nodes[best_node].f)
				best_node=free_node;
			
			WORD next=nodes[sorted_node].next;
			nodes[sorted_node].next=free_node;
			nodes[free_node].prev=sorted_node;
			nodes[free_node].next=next;
			nodes[next].prev=free_node;
			
			nodesdata[parent_node].successor[d]=free_node;
			nodesdata[free_node].ancestor=parent_node;
			for(register int i=0;i<directions;i++) nodesdata[free_node].successor[i]=EMPTY_NODE;
			
			//
			open_nodes++;
		}
		
		else if(world[yx].state==CLOSED)
		{
			float g=nodesdata[parent_node].g + cost*DXY[d].cost_multiplier;
			float f=g+distance(yx);
			if(f<nodes[world[yx].node].f)
			{
				reopen_closed_node(f,g,world[yx].node,yx,d,parent_node);
			}
		}
		
	}
}


// most often called function -- must be as fast as possible
inline void CAStarComplete::expand_nodes_to_open_terrain(WORD parent_node)
{
	for(int d=0;d<directions;d++)
	{
		//
		WORD yx=nodesdata[parent_node].yx+DXY[d].yx;
		
		if(world[yx].state==UNKNOWN)
		{
			//
			world[yx].state=OPEN;
			world[yx].route=d;
			
			free_node++;
			world[yx].node=free_node;
			
			nodesdata[free_node].yx=yx;
			
			nodesdata[free_node].g=nodesdata[parent_node].g + (float)world[yx].terrain_cost*DXY[d].terrain_cost_multiplier;
			nodes[free_node].f=nodesdata[free_node].g+distance(yx);
			
			register WORD sorted_node=find_sorted_node_position(nodes[free_node].f);
			
			if(nodes[free_node].f<nodes[best_node].f)
				best_node=free_node;
			
			WORD next=nodes[sorted_node].next;
			nodes[sorted_node].next=free_node;
			nodes[free_node].prev=sorted_node;
			nodes[free_node].next=next;
			nodes[next].prev=free_node;
			
			nodesdata[parent_node].successor[d]=free_node;
			nodesdata[free_node].ancestor=parent_node;
			for(register int i=0;i<directions;i++) nodesdata[free_node].successor[i]=EMPTY_NODE;
			
			//
			open_nodes++;
		}
		
		else if(world[yx].state==CLOSED)
		{
			float g=nodesdata[parent_node].g + (float)world[yx].terrain_cost*DXY[d].terrain_cost_multiplier;
			float f=g+distance(yx);
			if(f<nodes[world[yx].node].f)
			{
				reopen_closed_node(f,g,world[yx].node,yx,d,parent_node);
			}
		}
		
	}
}

//
inline void CAStarComplete::reopen_closed_node(
											  float f,
											  float g,
											  WORD reopened_node,
											  WORD yx,
											  BYTE d,
											  WORD parent_node
											  )
{
	//
	world[yx].state=OPEN;
	world[yx].route=d;
	nodesdata[reopened_node].g=g;
	nodes[reopened_node].f=f;
	nodesdata[reopened_node].ancestor=parent_node;
	nodesdata[parent_node].successor[d]=reopened_node;
				
	//
	if(f<nodes[best_node].f)
		best_node=reopened_node;
				
	//unlink from closed
	WORD prev=nodes[reopened_node].prev;
	WORD next=nodes[reopened_node].next;
	if(prev==EMPTY_NODE)
		nodes[next].prev=EMPTY_NODE;
	else if(next==EMPTY_NODE)
		nodes[prev].next=EMPTY_NODE;
	else
	{
		nodes[prev].next=next;
		nodes[next].prev=prev;
	}
				
	// relink to open
	register WORD sorted_node=find_sorted_node_position(f);
	next=nodes[sorted_node].next;
	nodes[sorted_node].next=reopened_node;
	nodes[reopened_node].prev=sorted_node;
	nodes[reopened_node].next=next;
	nodes[next].prev=reopened_node;
				
	reopened_nodes++;

	//
	open_nodes++;
	closed_nodes--;
}


// we look for a new f we're going to place to the right/next of the node we select.
inline WORD CAStarComplete::find_sorted_node_position(float f)
{
	WORD sorted_node=best_node;
	do
	{
		if(nodes[sorted_node].f>f) break;
		sorted_node=nodes[sorted_node].prev;
	} while(sorted_node!=best_node);
	return sorted_node;
}

//
inline float CAStarComplete::distance(WORD yx) //BYTE y,BYTE x)
{
	int x=yx&XMASK;
	int y=(yx>>YSHIFT);
	int endx=endyx&XMASK;
	int endy=(endyx>>YSHIFT);
	switch(distance_method)
	{
	case PYTHAGORAS_DISTANCE: //aka STRAIGHT LINE, pythagoras
		{
			int xpart=x-endx;
			int ypart=y-endy;
			return (float)sqrt((float)(xpart*xpart + ypart*ypart));
		}
		break;
		
	case MANHATTAN_DISTANCE:
		return (float)(abs(x-endx) + abs(y-endy));
		break;
		
	case DIAGONAL_DISTANCE:
		{
			WORD a=(WORD)abs(x-endx);
			WORD b=(WORD)abs(y-endy);
			return (a>b)?(float)a:(float)b;
		}
		break;
		
	case SIMPLE_PYTHAGORAS_DISTANCE:
		{
			int xpart=x-endx;
			int ypart=y-endy;
			return (float)(xpart*xpart + ypart*ypart);
		}
		break;
	default: return 0.0f;
	}
}


//
void CAStarComplete::move_node_to_closed(WORD parent_node)
{
	//
	if(best_node==parent_node)
		best_node=nodes[best_node].prev;
	
	//
	if(nodesdata[parent_node].yx==endyx)
		path_found=true;
	
	world[ nodesdata[parent_node].yx ].state=CLOSED;
	
	// cache prev/next link and f
	WORD prev=nodes[parent_node].prev;
	WORD next=nodes[parent_node].next;
	
	//remove node from open list
	nodes[prev].next=next;
	nodes[next].prev=prev;
	
	//insert node into closed linked-list
	nodes[closed_node].next=parent_node;
	nodes[parent_node].prev=closed_node;
	nodes[parent_node].next=EMPTY_NODE;
	closed_node=parent_node;
	
	//
	closed_nodes++;
	if(--open_nodes==0) no_path=true;
}




///////////////////////////////////////////////////////////////////////////
///////////////////////////////////////////////////////////////////////////
///////////////////////////////////////////////////////////////////////////
// to make it easier to add many new Setups to patherfinder.cpp we use Setup.cpp 
// class as a holder for all general settings and world info.

// tells Setup.cpp what menu options should be enabled/disabled for this alg.
void CAStarComplete::GetOptions()
{
	Setup->options.uniform_cost=true;
	Setup->options.terrain_cost=true;
	Setup->options.distance=true;
	Setup->options.search_directions=true;
}

// this transfers the world map from Setup.cpp to the internal state here.
void CAStarComplete::UpdateWorld()
{
	LARGE_INTEGER tmp1;
	QueryPerformanceCounter(&tmp1);
	
	//
	for(register int y=0;y<HEIGHT;y++)
	{
		for(register int x=0;x<WIDTH;x++)
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
void CAStarComplete::UpdateSettings()
{
	startyx=Setup->starty*WIDTH+Setup->startx;
	endyx=Setup->endy*WIDTH+Setup->endx;
	
	use_terrain=Setup->use_terrain;
	distance_method=Setup->distance_method;
	cost=Setup->cost;
	diagonal_cost=Setup->diagonal_cost;
	
	iterations_per_frame=Setup->iterations_per_frame;
	
	median_terrain_cost=Setup->median_terrain_cost;
	
	directions=Setup->directions;

	int n;
	for(n=0;n<4;n++)
	{
		DXY[n].cost_multiplier=1.0f;
		DXY[n].terrain_cost_multiplier=median_terrain_cost;
	}
		for(n=4;n<8;n++)
		{
			DXY[n].cost_multiplier=diagonal_cost;
			DXY[n].terrain_cost_multiplier=diagonal_cost*median_terrain_cost;
		}
}




///////////////////////////////////////////////////////////////////////////
// TYPE B
void CAStarComplete::PackRoute()
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
void CAStarComplete::Paint(LPBYTE pwBits, HDC hdc,HWND hWnd)
{
	if(pwBits) // && Setup->map_loaded)
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
		_RGBA *p,*ppush,*pbegin;
		pbegin=(_RGBA *)pwBits;
		register _RGBA *tmp;
		int y,x;
		int yx;
		int node;
		int count;
		
		//
		int used_nodes=0;
		float smallest_f, smallest_g, smallest_h;
		float largest_f, largest_g, largest_h;
		smallest_f = smallest_g = smallest_h = 1048576.0f;
		largest_f = largest_g = largest_h = 0.0001f;
		if(open_nodes>0)
		{
			count=MAX_NODE;
			node=best_node;
			do
			{
				if(nodes[node].f>largest_f) largest_f=nodes[node].f;
				if(nodes[node].f<smallest_f) smallest_f=nodes[node].f;
				if(nodesdata[node].g>largest_g) largest_g=nodesdata[node].g;
				if(nodesdata[node].g<smallest_g) smallest_g=nodesdata[node].g;
				float h=nodes[node].f-nodesdata[node].g;
				if(h>largest_h) largest_h=h;
				if(h<smallest_h) smallest_h=h;
				node=nodes[node].next;
				used_nodes++;
			} while(node!=best_node && node!=EMPTY_NODE && --count>=0);
		}
		count=MAX_NODE;
		node=closed_node;
		do
		{
			if(nodes[node].f>largest_f) largest_f=nodes[node].f;
			if(nodes[node].f<smallest_f) smallest_f=nodes[node].f;
			if(nodesdata[node].g>largest_g) largest_g=nodesdata[node].g;
			if(nodesdata[node].g<smallest_g) smallest_g=nodesdata[node].g;
			float h=nodes[node].f-nodesdata[node].g;
			if(h>largest_h) largest_h=h;
			if(h<smallest_h) smallest_h=h;
			used_nodes++;
			node=nodes[node].prev;
		} while(node!=EMPTY_NODE && --count>=0);
		
		//
		int length=0;
		count=MAX_NODE;
		node=closed_node;
		do
		{
			length++;
			node=nodesdata[node].ancestor;
		} while(node!=EMPTY_NODE && --count>=0);
		
		
		GetClientRect(hWnd, &rt);
		sprintf(szStatusLine,
			"-- A* Complete (v2c) --\nf %f ... %f\ng %f ... %f\nh %f ... %f\n\nstruct size %d bytes\n(world %d B)\n(node %d B)\n(used node %d B)\n\nMAX NODES %d\nBest Node\n(%d < %d > %d)\nf %f\ng %f\nh %f\nClosed Node\n(%d < %d > %d)\nf %f\ng %f\nh %f\n\npath found? %s\nlength= %d\n\nstart -> current\nf %f -> %f\ng %f -> %f\nh %f -> %f\n%s\n",
			smallest_f, largest_f,
			smallest_g, largest_g,
			smallest_h, largest_h,
			sizeof(nodes)+sizeof(nodesdata) + sizeof(world),
			sizeof(world),
			sizeof(nodes)+sizeof(nodesdata),
			used_nodes*(sizeof(_NODES)+sizeof(_NODESDATA)),
			MAX_NODE,
			nodes[best_node].prev,
			best_node,
			nodes[best_node].next,
			nodes[best_node].f,
			nodesdata[best_node].g,
			nodes[best_node].f-nodes[best_node].f,
			nodes[closed_node].prev,
			closed_node,
			nodes[closed_node].next,
			nodes[closed_node].f,
			nodesdata[closed_node].g,
			nodes[closed_node].f-nodes[closed_node].f,
			no_path?"NO PATH!":(path_found?"YES!":"Not yet..."),
			length,
			nodes[1].f,nodes[closed_node].f,
			nodesdata[1].g,nodesdata[closed_node].g,
			nodes[1].f-nodesdata[1].g,nodes[closed_node].f-nodesdata[closed_node].g,
			(nodes[closed_node].f<=nodes[1].f)?"ADMISSIBLE":"not admissible"
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
		count=MAX_NODE;
		node=closed_node;
		do
		{
			tmp=(p+(nodesdata[node].yx>>YSHIFT)*clientwidth+(nodesdata[node].yx&XMASK));
			tmp->red=255;
			tmp->green=0;
			tmp->blue=0;
			node=nodesdata[node].ancestor;
		} while(node!=EMPTY_NODE && --count>=0);
		
		
		// f
		GetClientRect(hWnd, &rt);
		rt.top=HEIGHT;
		rt.left=WIDTH+4;
		rt.right=WIDTH+4+WIDTH;
		sprintf(szStatusLine,"f (red closed/blue open)");
		DrawText(hdc, szStatusLine, strlen(szStatusLine), &rt, DT_CENTER);
		p=pbegin+(WIDTH+4);
		if(open_nodes>0)
		{
			count=MAX_NODE;
			node=best_node;
			do
			{
				tmp=(p+(nodesdata[node].yx>>YSHIFT)*clientwidth+(nodesdata[node].yx&XMASK));
				tmp->red=0;
				tmp->green=0;
				tmp->blue=(BYTE)((nodes[node].f/largest_f)*255.0f);
				node=nodes[node].prev;
			} while(node!=best_node && node!=EMPTY_NODE && --count>=0);
		}
		count=MAX_NODE;
		node=closed_node;
		do
		{
			tmp=(p+(nodesdata[node].yx>>YSHIFT)*clientwidth+(nodesdata[node].yx&XMASK));
			tmp->red=(BYTE)((nodes[node].f/largest_f)*255.0f);
			tmp->green=0;
			tmp->blue=0;
			node=nodes[node].prev;
		} while(node!=EMPTY_NODE && --count>=0);
		
		
		// open
		GetClientRect(hWnd, &rt);
		rt.top=HEIGHT*2+16;
		rt.left=0;
		rt.right=WIDTH;
		sprintf(szStatusLine,"open nodes %d",open_nodes);
		DrawText(hdc, szStatusLine, strlen(szStatusLine), &rt, DT_CENTER);
		if(open_nodes>0)
		{
			p=pbegin+(HEIGHT+16)*clientwidth;
			count=MAX_NODE;
			node=best_node;
			do
			{
				BYTE f=(BYTE)((nodes[node].f/largest_f)*255.0f);
				BYTE g=(BYTE)((nodesdata[node].g/largest_g)*255.0f);
				BYTE h=(BYTE)(((nodes[node].f-nodesdata[node].g)/largest_h)*255.0f);
				tmp=(p+(nodesdata[node].yx>>YSHIFT)*clientwidth+(nodesdata[node].yx&XMASK));
				tmp->red=h;
				tmp->green=g;
				tmp->blue=f;
				node=nodes[node].prev;
			} while(node!=best_node && node!=EMPTY_NODE && --count>=0);
		}
		
		
		// closed
		GetClientRect(hWnd, &rt);
		rt.top=HEIGHT*2+16;
		rt.left=WIDTH+4;
		rt.right=WIDTH+4+WIDTH;
		sprintf(szStatusLine,"closed %d (reopened %d) nodes",closed_nodes,reopened_nodes);
		DrawText(hdc, szStatusLine, strlen(szStatusLine), &rt, DT_CENTER);
		p=pbegin+WIDTH+4+(HEIGHT+16)*clientwidth;
		count=MAX_NODE;
		node=closed_node;
		do
		{
			BYTE f=(BYTE)((nodes[node].f/largest_f)*255.0f);
			BYTE g=(BYTE)((nodesdata[node].g/largest_g)*255.0f);
			BYTE h=(BYTE)(((nodes[node].f-nodesdata[node].g)/largest_h)*239.0f);
			tmp=(p+(nodesdata[node].yx>>YSHIFT)*clientwidth+(nodesdata[node].yx&XMASK));
			tmp->red=h;
			tmp->green=g;
			tmp->blue=f;
			node=nodes[node].prev;
		} while(node!=EMPTY_NODE && --count>=0);
		
		
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
		
		
		// successors
		GetClientRect(hWnd, &rt);
		rt.top=HEIGHT*2+16;
		rt.left=(WIDTH+4)*2;
		rt.right=(WIDTH+4)*2+WIDTH;
		sprintf(szStatusLine,"successors");
		DrawText(hdc, szStatusLine, strlen(szStatusLine), &rt, DT_CENTER);
		p=pbegin+(WIDTH+4)*2+(HEIGHT+16)*clientwidth;
		int diff=(directions==8)?31:63;
		if(open_nodes>0)
		{
			count=MAX_NODE;
			node=best_node;
			do
			{
				int successors=0;
				for(register int i=0;i<directions;i++)
				{
					if(nodesdata[node].successor[i]!=EMPTY_NODE)
						successors+=diff;
				}
				tmp=(p+(nodesdata[node].yx>>YSHIFT)*clientwidth+(nodesdata[node].yx&XMASK));
				tmp->red=(BYTE)successors;
				tmp->green=0;
				tmp->blue=0;
				node=nodes[node].prev;
			} while(node!=best_node && node!=EMPTY_NODE && --count>=0);
		}
		count=MAX_NODE;
		node=closed_node;
		do
		{
			int successors=0;
			for(register int i=0;i<directions;i++)
			{
				if(nodesdata[node].successor[i]!=EMPTY_NODE)
					successors+=diff;
			}
			tmp=(p+(nodesdata[node].yx>>YSHIFT)*clientwidth+(nodesdata[node].yx&XMASK));
			tmp->red=(BYTE)successors;
			tmp->green=0;
			tmp->blue=0;
			node=nodes[node].prev;
		} while(node!=EMPTY_NODE && --count>=0);
	}
}
