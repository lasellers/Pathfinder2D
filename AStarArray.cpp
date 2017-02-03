// AStarArray.cpp: implementation of the AStarArray class.
//
//////////////////////////////////////////////////////////////////////
/*
AStar Version 1

  Static Arrays
*/

#include "stdafx.h"
#include "AStarArray.h"

#include "math.h" //sqrt
#include "stdio.h" //for sprintf

//////////////////////////////////////////////////////////////////////
// Construction/Destruction
//////////////////////////////////////////////////////////////////////

CAStarArray::CAStarArray()
{
	Setup=NULL;
	airoute=NULL;
}

CAStarArray::CAStarArray(CSetup *Setup, AIROUTE *airoute)
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

CAStarArray::~CAStarArray()
{
	
}

void CAStarArray::Reset(AIROUTE *airoute)
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
	
	open_nodes=1;
	closed_nodes=0;
	
	//
	for(int yx=0;yx<WIDTH*HEIGHT;yx++)
	{
		world[yx].state=UNKNOWN;
	}
	
	int n,i;
	for(n=0;n<=MAX_NODES;n++)
	{
		nodes[n].yx=0;
		nodes[n].f=0.0f;
		nodes[n].g=0.0f;
		nodes[n].h=0.0f;
		nodes[n].ancestor=EMPTY_NODE;
		nodes[n].open=true;
		for(i=0;i<8;i++) nodes[n].successor[i]=EMPTY_NODE;
		nodes[n].open_node_index=EMPTY_NODE;
	}
	
	for(n=0;n<MAX_OPEN_NODE_INDEXES;n++) //optimization c
		open_node_index[n]=EMPTY_NODE;

	//
	nodes[EMPTY_NODE].f=MAX_F; //fake high number to avoid special casing EMPTY_NODE
	nodes[EMPTY_NODE].open=false;

	//
	last_node=current_node=1;
	last_index=1;
	
	nodes[current_node].yx=startyx;
	nodes[current_node].g=0.0f;
	nodes[current_node].h=distance(startyx)*DXY[4].cost_multiplier;//*median_terrain_cost;
	nodes[current_node].f=nodes[1].g+nodes[current_node].h;
	nodes[current_node].open=true;
	nodes[current_node].open_node_index=last_index;
	open_node_index[last_index]=current_node;
	
	//
	open_node_index[0]=EMPTY_NODE;

	//
	LARGE_INTEGER tmp2;
	QueryPerformanceCounter(&tmp2);
	bigtick.QuadPart += tmp2.QuadPart - tmp1.QuadPart;
}


//
void CAStarArray::FindPath()
{
	if(current_node==EMPTY_NODE || last_node<=0 || path_found || no_path)
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
	int count=iterations_per_frame;
	do
	{
		//
		open_successor_nodes();
		WORD parent_node=current_node;
		keep_moving_forward_through_best_successor_node();
		move_node_to_closed(parent_node);
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

//
inline void CAStarArray::keep_moving_forward_through_best_successor_node()
{
	WORD parent_node=current_node;
	for(register WORD n=0;n<directions;n++)
	{
		register WORD node=nodes[parent_node].successor[n];
		if(
			node!=EMPTY_NODE &&
			nodes[node].open &&
			nodes[node].f <= nodes[current_node].f
			)
			current_node=node;
	}
}


// optimize me!
inline void CAStarArray::backtrack_to_find_best_node_to_explore_off_of()
{ //backtrack
	int node=last_node;
	int index=last_index;
	current_node=EMPTY_NODE;
	do
	{
		node=open_node_index[index];
		if(node==EMPTY_NODE) break;
		if(nodes[node].f<=nodes[current_node].f)
			current_node=node;
	} while(--index>0);
	if(current_node==EMPTY_NODE)
		no_path=true;
}

// most often called function -- must be as fast as possible
inline void CAStarArray::open_successor_nodes()
{
	WORD parentyx=nodes[current_node].yx;
	
	for(int d=0;d<directions;d++)
	{
		WORD yx=parentyx+DXY[d].yx;
		
		if(
			(last_node>=MAX_NODES) || //if we've ran out of allocatd nodes ignore
			(world[yx].terrain_cost==IMPASSABLE_TERRAIN_COST) ||//if elevation is high mountains we can't travel there
			(world[yx].state!=UNKNOWN) //optimization
			)
			;
		else
		{
			//
			world[yx].state=OPEN; //optimization
			world[yx].route=d;
			
			//
			last_node++;
			
			//
			nodes[last_node].open=true;
			nodes[last_node].yx=yx;
			
			nodes[last_node].h=distance(yx);
			
			if(use_terrain)
				nodes[last_node].g=nodes[current_node].g + world[yx].terrain_cost*DXY[d].cost_multiplier;
			else
				nodes[last_node].g=nodes[current_node].g + DXY[d].cost_multiplier;
			
			nodes[last_node].ancestor=current_node;
			nodes[current_node].successor[d]=last_node;
			nodes[last_node].f=nodes[last_node].g+nodes[last_node].h;
			
			// for optimization c
			if(++last_index>MAX_OPEN_NODE_INDEXES)
				no_path=true;
			open_node_index[last_index]=last_node;
			nodes[last_node].open_node_index=last_index;
			
			//
			open_nodes++;
		}
	}
}


//
inline bool CAStarArray::move_node_to_closed(WORD node)
{
		world[nodes[node].yx].state=CLOSED;
		nodes[node].open=false;
		
		// indexs
		WORD index=nodes[node].open_node_index;
		for(register int i=index;i<last_index;i++)
		{
			nodes[open_node_index[i+1]].open_node_index--;
			open_node_index[i]=open_node_index[i+1];
		}
		if(--last_index<=0) no_path=true;

		//
		if(nodes[node].yx==endyx)
			path_found=true;
		else if(current_node==node) //closing the node we're on? dead end. pruning.
			backtrack_to_find_best_node_to_explore_off_of();
		
		//
		closed_nodes++;
		if(--open_nodes==0) no_path=true;
	
	return path_found;
}


//
inline float CAStarArray::distance(WORD yx)
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









///////////////////////////////////////////////////////////////////////////
///////////////////////////////////////////////////////////////////////////
///////////////////////////////////////////////////////////////////////////
// to make it easier to add many new Setups to patherfinder.cpp we use Setup.cpp 
// class as a holder for all general settings and world info.

// tells Setup.cpp what menu options should be enabled/disabled for this alg.
void CAStarArray::GetOptions()
{
	Setup->options.uniform_cost=true;
	Setup->options.terrain_cost=true;
	Setup->options.distance=true;
	Setup->options.search_directions=true;
}

// this transfers the world map from Setup.cpp to the internal state here.
void CAStarArray::UpdateWorld()
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
void CAStarArray::UpdateSettings()
{
	startyx=Setup->starty*WIDTH+Setup->startx;
	endyx=Setup->endy*WIDTH+Setup->endx;
	
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
void CAStarArray::PackRoute()
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
void CAStarArray::Paint(LPBYTE pwBits, HDC hdc,HWND hWnd)
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
		_RGBA  *pbegin=(_RGBA *)pwBits;
		register _RGBA *tmp;
		int yx;
		int y,x;
		int n;
		int node;
		
		//
		float smallest_f, smallest_g, smallest_h;
		float largest_f, largest_g, largest_h;
		smallest_f = smallest_g = smallest_h = 1048576.0f;
		largest_f = largest_g = largest_h = 0.0001f;
		for(node=1;node<=last_node;node++)
		{
			if(nodes[node].f>largest_f) largest_f=nodes[node].f;
			if(nodes[node].f<smallest_f) smallest_f=nodes[node].f;
			if(nodes[node].g>largest_g) largest_g=nodes[node].g;
			if(nodes[node].g<smallest_g) smallest_g=nodes[node].g;
			if(nodes[node].h>largest_h) largest_h=nodes[node].h;
			if(nodes[node].h<smallest_h) smallest_h=nodes[node].h;
		}
		
		//
		int count=0;
		int tmpnode=current_node;
		while(tmpnode!=EMPTY_NODE)
		{
			tmpnode=nodes[tmpnode].ancestor;
			count++;
		}
		
		
		// statistics
		GetClientRect(hWnd, &rt);
		sprintf(szStatusLine,
			"-- A* Array (v1) --\nf %f ... %f\ng %f ... %f\nh %f ... %f\n\nNodes %d (MAX %d)\nCurrent Node %d\nLast Open Node %d\nLast Index %d\n\npath found? %s\nlength= %d\n\nstart -> current\nf %f -> %f\ng %f -> %f\nh %f -> %f\n%s\n",
			smallest_f, largest_f,
			smallest_g, largest_g,
			smallest_h, largest_h,
			last_node,
			MAX_NODES,
			current_node,
			open_nodes,
			last_index,
			no_path?"NO PATH!":(path_found?"YES!":"Not yet..."),
			count,
			nodes[1].f,nodes[current_node].f,
			nodes[1].g,nodes[current_node].g,
			nodes[1].h,nodes[current_node].h,
			(nodes[current_node].f<=nodes[1].f)?"ADMISSIBLE":"not admissible"
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
				yx=(y<<YSHIFT)+x;
				BYTE b=(BYTE)(255-(world[yx].terrain_cost<<4));
				p->red=b; p->green=b; p->blue=b;
				p++;
			}
			ppush+=clientwidth;
			p=ppush;
		}
		
		p=pbegin;
		// start point
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
		// end point
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

		p=pbegin;
		node=current_node;
		while(node!=EMPTY_NODE)
		{
			tmp=(p+(nodes[node].yx>>YSHIFT)*clientwidth+(nodes[node].yx&XMASK));
			tmp->red=255;
			tmp->green=0;
			tmp->blue=0;
			node=nodes[node].ancestor;
		}
		
		
		// f
		GetClientRect(hWnd, &rt);
		rt.top=HEIGHT;
		rt.left=WIDTH+4;
		rt.right=WIDTH+4+WIDTH;
		sprintf(szStatusLine,"f (red closed/blue open)");
		DrawText(hdc, szStatusLine, strlen(szStatusLine), &rt, DT_CENTER);
		p=pbegin+WIDTH+4;
		for(n=1;n<=last_node;n++)
		{
			if(nodes[n].open)
			{
				tmp=(p+(nodes[n].yx>>YSHIFT)*clientwidth+(nodes[n].yx&XMASK));
				BYTE f=(BYTE)(((float)nodes[n].f/(float)largest_f)*255.0f);
				tmp->red=0;
				tmp->green=0;
				tmp->blue=f;
			}
		}
		for(n=1;n<=last_node;n++)
		{
			if(!nodes[n].open)
			{
				tmp=(p+(nodes[n].yx>>YSHIFT)*clientwidth+(nodes[n].yx&XMASK));
				BYTE f=(BYTE)(((float)nodes[n].f/(float)largest_f)*255.0f);
				tmp->red=f;
				tmp->green=0;
				tmp->blue=0;
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
		for(n=1;n<=last_node;n++)
		{
			if(nodes[n].open)
			{
				BYTE g=(BYTE)(((float)nodes[n].g/(float)largest_g)*255.0f);
				BYTE h=(BYTE)(((float)nodes[n].h/(float)largest_h)*255.0f);
				BYTE f=(BYTE)(((float)nodes[n].f/(float)largest_f)*255.0f);
				tmp=(p+(nodes[n].yx>>YSHIFT)*clientwidth+(nodes[n].yx&XMASK));
				tmp->red=h;
				tmp->green=g;
				tmp->blue=f;
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
		for(n=1;n<=last_node;n++)
		{
			if(!nodes[n].open)
			{
				BYTE g=(BYTE)(((float)nodes[n].g/(float)largest_g)*255.0f);
				BYTE h=(BYTE)(((float)nodes[n].h/(float)largest_h)*255.0f);
				BYTE f=(BYTE)(((float)nodes[n].f/(float)largest_f)*255.0f);
				tmp=(p+(nodes[n].yx>>YSHIFT)*clientwidth+(nodes[n].yx&XMASK));
				tmp->red=h;
				tmp->green=g;
				tmp->blue=f;
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
		for(node=1;node<=last_node;node++)
		{
			int successors=0;
			for(int i=0;i<directions;i++)
			{
				if(nodes[node].successor[i]!=EMPTY_NODE)
					successors+=diff;
			}
			tmp=(p+(nodes[node].yx>>YSHIFT)*clientwidth+(nodes[node].yx&XMASK));
			tmp->red=(BYTE)successors;
			tmp->green=0;
			tmp->blue=0;
		}
	}
	
}
