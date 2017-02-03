// Generic.cpp: implementation of the Generic class.
//
//////////////////////////////////////////////////////////////////////

#include "stdafx.h"
#include "Generic.h"

#include "stdio.h" //for sprintf

//for ms visual 2008
 #if defined(_CRT_SECURE_NO_DEPRECATE) && !defined(_CRT_SECURE_NO_WARNINGS)
 #define _CRT_SECURE_NO_WARNINGS
 #endif

//////////////////////////////////////////////////////////////////////
// Construction/Destruction
//////////////////////////////////////////////////////////////////////

CGeneric::CGeneric()
{
	Alg=NONE_ALGORITHM;
}

CGeneric::~CGeneric()
{
	delete Development;
	delete AStar;
	delete AStarHeapInteger;
	delete AStarHeap;
	delete AStarComplete;
	delete Dijkstra;
	delete AStarArray;
	delete AStarLinkedList;
	delete BreadthFirst;
	delete BestFirst;
	delete DepthFirst;
}

CGeneric::CGeneric(CSetup *Setup)
{
	//
	this->Setup=Setup;
	this->Alg=ASTAR_ALGORITHM;
	
	//
	for(int a=0;a<MAX_AIS;a++)
	{
		_AIROUTE *airoute=&AIRoute[a];
		airoute->color=((rand()%0xff)<<16) + ((rand()%0xff)<<8) + (rand()%0xff);
		airoute->count=0;
		airoute->priority=0;
		airoute->active=1;
		airoute->startyx=(rand()%(WIDTH-4)+2) +  ((rand()%(HEIGHT-4)+2)<<YSHIFT); 
		airoute->endyx=(rand()%(WIDTH-4)+2) +  ((rand()%(HEIGHT-4)+2)<<YSHIFT); 
	}

	//
	ai=0;
	
	//
	Development=new CDevelopment(Setup,&AIRoute[0]);
	AStar=new CAStar(Setup,&AIRoute[0]);
	AStarHeapInteger=new CAStarHeapInteger(Setup,&AIRoute[0]);
	AStarHeap=new CAStarHeap(Setup,&AIRoute[0]);
	AStarComplete=new CAStarComplete(Setup,&AIRoute[0]);
	Dijkstra=new CDijkstra(Setup,&AIRoute[0]);
	AStarArray=new CAStarArray(Setup,&AIRoute[0]);
	AStarLinkedList=new CAStarLinkedList(Setup,&AIRoute[0]);
	BreadthFirst=new CBreadthFirst(Setup,&AIRoute[0]);
	BestFirst=new CBestFirst(Setup,&AIRoute[0]);
	DepthFirst=new CDepthFirst(Setup,&AIRoute[0]);
	
	//
	bigtick.QuadPart=0;

	//
	gamemode=false;
	routing_view=false;
	
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




// ////////////////////////////////////////////////////////////////////////
// ////////////////////////////////////////////////////////////////////////
// ////////////////////////////////////////////////////////////////////////
void CGeneric::Reset()
{
	//
	switch(Alg)
	{
	case DEVELOPMENT_ALGORITHM:			Development->Reset(&AIRoute[ai]); break;
	case ASTAR_ALGORITHM:				AStar->Reset(&AIRoute[ai]); break;
	case ASTAR_HEAPINTEGER_ALGORITHM:	AStarHeapInteger->Reset(&AIRoute[ai]); break;
	case ASTAR_HEAP_ALGORITHM:			AStarHeap->Reset(&AIRoute[ai]); break;
	case ASTAR_COMPLETE_ALGORITHM:		AStarComplete->Reset(&AIRoute[ai]); break;
	case DIJKSTRA_ALGORITHM:			Dijkstra->Reset(&AIRoute[ai]); break;
	case ASTAR_ARRAY_ALGORITHM:			AStarArray->Reset(&AIRoute[ai]); break;
	case ASTAR_LINKEDLIST_ALGORITHM:	AStarLinkedList->Reset(&AIRoute[ai]); break;
	case BREADTHFIRST_ALGORITHM:		BreadthFirst->Reset(&AIRoute[ai]); break;
	case BESTFIRST_ALGORITHM:			BestFirst->Reset(&AIRoute[ai]); break;
	case DEPTHFIRST_ALGORITHM:			DepthFirst->Reset(&AIRoute[ai]); break;
	}
}

/*
*/
void CGeneric::UpdateWorld()
{
	switch(Alg)
	{
	case DEVELOPMENT_ALGORITHM:			Development->UpdateWorld(); break;
	case ASTAR_ALGORITHM:				AStar->UpdateWorld(); break;
	case ASTAR_HEAPINTEGER_ALGORITHM:	AStarHeapInteger->UpdateWorld(); break;
	case ASTAR_HEAP_ALGORITHM:			AStarHeap->UpdateWorld(); break;
	case ASTAR_COMPLETE_ALGORITHM:		AStarComplete->UpdateWorld(); break;
	case DIJKSTRA_ALGORITHM:			Dijkstra->UpdateWorld(); break;
	case ASTAR_ARRAY_ALGORITHM:			AStarArray->UpdateWorld(); break;
	case ASTAR_LINKEDLIST_ALGORITHM:	AStarLinkedList->UpdateWorld(); break;
	case BREADTHFIRST_ALGORITHM:		BreadthFirst->UpdateWorld(); break;
	case BESTFIRST_ALGORITHM:			BestFirst->UpdateWorld(); break;
	case DEPTHFIRST_ALGORITHM:			DepthFirst->UpdateWorld(); break;
	}
	//
	bigtick.QuadPart=0;
}

/*
*/
void CGeneric::UpdateSettings()
{
	switch(Alg)
	{
	case DEVELOPMENT_ALGORITHM:			Development->UpdateSettings(); break;
	case ASTAR_ALGORITHM:				AStar->UpdateSettings(); break;
	case ASTAR_HEAPINTEGER_ALGORITHM:	AStarHeapInteger->UpdateSettings(); break;
	case ASTAR_HEAP_ALGORITHM:			AStarHeap->UpdateSettings(); break;
	case ASTAR_COMPLETE_ALGORITHM:		AStarComplete->UpdateSettings(); break;
	case DIJKSTRA_ALGORITHM:			Dijkstra->UpdateSettings(); break;
	case ASTAR_ARRAY_ALGORITHM:			AStarArray->UpdateSettings(); break;
	case ASTAR_LINKEDLIST_ALGORITHM:	AStarLinkedList->UpdateSettings(); break;
	case BREADTHFIRST_ALGORITHM:		BreadthFirst->UpdateSettings(); break;
	case BESTFIRST_ALGORITHM:			BestFirst->UpdateSettings(); break;
	case DEPTHFIRST_ALGORITHM:			DepthFirst->UpdateSettings(); break;
	}
}

/*
*/
void CGeneric::GetOptions()
{
	switch(Alg)
	{
	case DEVELOPMENT_ALGORITHM:			Development->GetOptions(); break;
	case ASTAR_ALGORITHM:				AStar->GetOptions(); break;
	case ASTAR_HEAPINTEGER_ALGORITHM:	AStarHeapInteger->GetOptions(); break;
	case ASTAR_HEAP_ALGORITHM:			AStarHeap->GetOptions(); break;
	case ASTAR_COMPLETE_ALGORITHM:		AStarComplete->GetOptions(); break;
	case DIJKSTRA_ALGORITHM:			Dijkstra->GetOptions(); break;
	case ASTAR_ARRAY_ALGORITHM:			AStarArray->GetOptions(); break;
	case ASTAR_LINKEDLIST_ALGORITHM:	AStarLinkedList->GetOptions(); break;
	case BREADTHFIRST_ALGORITHM:		BreadthFirst->GetOptions(); break;
	case BESTFIRST_ALGORITHM:			BestFirst->GetOptions(); break;
	case DEPTHFIRST_ALGORITHM:			DepthFirst->GetOptions(); break;
	}
}



///////////////////////////////////////////////////////////////////////////
/*
*/
void CGeneric::Paint(LPBYTE pwBits,HDC hdctmp,HWND hWnd)
{
	if(routing_view)
		Paint_All(pwBits,hdctmp,hWnd);
	else
	{
		switch(Alg)
		{
		case DEVELOPMENT_ALGORITHM:			Development->Paint(pwBits,hdctmp,hWnd); break;
		case ASTAR_ALGORITHM:				AStar->Paint(pwBits,hdctmp,hWnd); break;
		case ASTAR_HEAPINTEGER_ALGORITHM:	AStarHeapInteger->Paint(pwBits,hdctmp,hWnd); break;
		case ASTAR_HEAP_ALGORITHM:			AStarHeap->Paint(pwBits,hdctmp,hWnd); break;
		case ASTAR_COMPLETE_ALGORITHM:		AStarComplete->Paint(pwBits,hdctmp,hWnd); break;
		case DIJKSTRA_ALGORITHM:			Dijkstra->Paint(pwBits,hdctmp,hWnd); break;
		case ASTAR_ARRAY_ALGORITHM:			AStarArray->Paint(pwBits,hdctmp,hWnd); break;
		case ASTAR_LINKEDLIST_ALGORITHM:	AStarLinkedList->Paint(pwBits,hdctmp,hWnd); break;
		case BREADTHFIRST_ALGORITHM:		BreadthFirst->Paint(pwBits,hdctmp,hWnd); break;
		case BESTFIRST_ALGORITHM:			BestFirst->Paint(pwBits,hdctmp,hWnd); break;
		case DEPTHFIRST_ALGORITHM:			DepthFirst->Paint(pwBits,hdctmp,hWnd); break;
		}
	}
}

/*
*/
bool CGeneric::PathFound()
{
	switch(Alg)
	{
	case DEVELOPMENT_ALGORITHM:			return Development->path_found; break;
	case ASTAR_ALGORITHM:				return AStar->path_found; break;
	case ASTAR_HEAPINTEGER_ALGORITHM:	return AStarHeapInteger->path_found; break;
	case ASTAR_HEAP_ALGORITHM:			return AStarHeap->path_found; break;
	case ASTAR_COMPLETE_ALGORITHM:		return AStarComplete->path_found; break;
	case DIJKSTRA_ALGORITHM:			return Dijkstra->path_found; break;
	case ASTAR_ARRAY_ALGORITHM:			return AStarArray->path_found; break;
	case ASTAR_LINKEDLIST_ALGORITHM:	return AStarLinkedList->path_found; break;
	case BREADTHFIRST_ALGORITHM:		return BreadthFirst->path_found; break;
	case BESTFIRST_ALGORITHM:			return BestFirst->path_found; break;
	case DEPTHFIRST_ALGORITHM:			return DepthFirst->path_found; break;
	}
	return false;
}

/*
*/
bool CGeneric::NoPath()
{
	switch(Alg)
	{
	case DEVELOPMENT_ALGORITHM:			return Development->no_path; break;
	case ASTAR_ALGORITHM:				return AStar->no_path; break;
	case ASTAR_HEAPINTEGER_ALGORITHM:	return AStarHeapInteger->no_path; break;
	case ASTAR_HEAP_ALGORITHM:			return AStarHeap->no_path; break;
	case ASTAR_COMPLETE_ALGORITHM:		return AStarComplete->no_path; break;
	case DIJKSTRA_ALGORITHM:			return Dijkstra->no_path; break;
	case ASTAR_ARRAY_ALGORITHM:			return AStarArray->no_path; break;
	case ASTAR_LINKEDLIST_ALGORITHM:	return AStarLinkedList->no_path; break;
	case BREADTHFIRST_ALGORITHM:		return BreadthFirst->no_path; break;
	case BESTFIRST_ALGORITHM:			return BestFirst->no_path; break;
	case DEPTHFIRST_ALGORITHM:			return DepthFirst->no_path; break;
	}
	return false;
}


/*
*/
void CGeneric::ResetAI()
{
	if(gamemode) bigtick.QuadPart=0;
	ai=0;
}

//
bool CGeneric::isDone()
{
	if(gamemode)
		return (ai>=MAX_AIS);
	else
		return (PathFound() || NoPath());
}


//
void CGeneric::FindPath()
{
	if(!isDone())
	{
		//
		switch(Alg)
		{
		case DEVELOPMENT_ALGORITHM:			Development->FindPath(); break;
		case ASTAR_ALGORITHM:				AStar->FindPath(); break;
		case ASTAR_HEAPINTEGER_ALGORITHM:	AStarHeapInteger->FindPath(); break;
		case ASTAR_HEAP_ALGORITHM:			AStarHeap->FindPath(); break;
		case ASTAR_COMPLETE_ALGORITHM:		AStarComplete->FindPath(); break;
		case DIJKSTRA_ALGORITHM:			Dijkstra->FindPath(); break;
		case ASTAR_ARRAY_ALGORITHM:			AStarArray->FindPath(); break;
		case ASTAR_LINKEDLIST_ALGORITHM:	AStarLinkedList->FindPath(); break;
		case BREADTHFIRST_ALGORITHM:		BreadthFirst->FindPath(); break;
		case BESTFIRST_ALGORITHM:			BestFirst->FindPath(); break;
		case DEPTHFIRST_ALGORITHM:			DepthFirst->FindPath(); break;
		}
		
		//
		if(!gamemode)
		{
			if(PathFound() || NoPath())
			{
				LARGE_INTEGER tick;
				switch(Alg)
				{
				case DEVELOPMENT_ALGORITHM:			tick.QuadPart=Development->bigtick.QuadPart; break;
				case ASTAR_ALGORITHM:				tick.QuadPart=AStar->bigtick.QuadPart; break;
				case ASTAR_HEAPINTEGER_ALGORITHM:	tick.QuadPart=AStarHeapInteger->bigtick.QuadPart; break;
				case ASTAR_HEAP_ALGORITHM:			tick.QuadPart=AStarHeap->bigtick.QuadPart; break;
				case ASTAR_COMPLETE_ALGORITHM:		tick.QuadPart=AStarComplete->bigtick.QuadPart; break;
				case DIJKSTRA_ALGORITHM:			tick.QuadPart=Dijkstra->bigtick.QuadPart; break;
				case ASTAR_ARRAY_ALGORITHM:			tick.QuadPart=AStarArray->bigtick.QuadPart; break;
				case ASTAR_LINKEDLIST_ALGORITHM:	tick.QuadPart=AStarLinkedList->bigtick.QuadPart; break;
				case BREADTHFIRST_ALGORITHM:		tick.QuadPart=BreadthFirst->bigtick.QuadPart; break;
				case BESTFIRST_ALGORITHM:			tick.QuadPart=BestFirst->bigtick.QuadPart; break;
				case DEPTHFIRST_ALGORITHM:			tick.QuadPart=DepthFirst->bigtick.QuadPart; break;
				default: tick.QuadPart=0;
				}
				if(bigtick.QuadPart==0.0)
					bigtick.QuadPart=tick.QuadPart;
				else if(tick.QuadPart<bigtick.QuadPart)
					bigtick.QuadPart=tick.QuadPart;
			}
		}
		else
		{
			if(ai<MAX_AIS && (PathFound() || NoPath()) )
			{
				//
				switch(Alg)
				{
				case DEVELOPMENT_ALGORITHM:			bigtick.QuadPart+=Development->bigtick.QuadPart; break;
				case ASTAR_ALGORITHM:				bigtick.QuadPart+=AStar->bigtick.QuadPart; break;
				case ASTAR_HEAPINTEGER_ALGORITHM:	bigtick.QuadPart+=AStarHeapInteger->bigtick.QuadPart; break;
				case ASTAR_HEAP_ALGORITHM:			bigtick.QuadPart+=AStarHeap->bigtick.QuadPart; break;
				case ASTAR_COMPLETE_ALGORITHM:		bigtick.QuadPart+=AStarComplete->bigtick.QuadPart; break;
				case DIJKSTRA_ALGORITHM:			bigtick.QuadPart+=Dijkstra->bigtick.QuadPart; break;
				case ASTAR_ARRAY_ALGORITHM:			bigtick.QuadPart+=AStarArray->bigtick.QuadPart; break;
				case ASTAR_LINKEDLIST_ALGORITHM:	bigtick.QuadPart+=AStarLinkedList->bigtick.QuadPart; break;
				case BREADTHFIRST_ALGORITHM:		bigtick.QuadPart+=BreadthFirst->bigtick.QuadPart; break;
				case BESTFIRST_ALGORITHM:			bigtick.QuadPart+=BestFirst->bigtick.QuadPart; break;
				case DEPTHFIRST_ALGORITHM:			bigtick.QuadPart+=DepthFirst->bigtick.QuadPart; break;
				}
				
				//
				ai++;
				if(ai<MAX_AIS)
				{
					Setup->set_start(AIRoute[ai].startyx>>YSHIFT,AIRoute[ai].startyx&XMASK);
					Setup->set_end(AIRoute[ai].endyx>>YSHIFT,AIRoute[ai].endyx&XMASK);
					UpdateSettings();
					Reset();
				}
			}
		}
	}
}


///////////////////////////////////////////////////////////////////////////
void CGeneric::Switch_Algorithms(int Alg)
{
	this->Alg=Alg;
	
	GetOptions();
	UpdateSettings();
	UpdateWorld();
	Reset();
}


///////////////////////////////////////////////////////////////////////////
void CGeneric::set_GameMode(bool state)
{
	gamemode=state;
	
	GetOptions();
	UpdateSettings();
	UpdateWorld();
	Reset();
}



///////////////////////////////////////////////////////////////////////////

//
void CGeneric::Paint_All(LPBYTE pwBits, HDC hdc,HWND hWnd)
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
		_RGBA *pbegin=(_RGBA *)pwBits+(clientwidth>>1)-(WIDTH)+ ((clientheight>>1)-(HEIGHT))*clientwidth;
		_RGBA *pend=pbegin+clientwidth*(HEIGHT<<1);
		DWORD *tmp;
		
		// draw world map
		_RGBA *ppush;
		int y,x;
		ppush=pbegin;
		tmp=(DWORD *)pbegin;
		for(y=0;y<HEIGHT;y++)
		{
			for(x=0;x<WIDTH;x++)
			{
				BYTE b=(BYTE)(255-(Setup->world[y][x].terrain_cost<<4));
				DWORD d=(b<<16) | (b<<8) | b;
				*tmp=d;
				*(tmp+1)=d;
				*(tmp+clientwidth)=d;
				*(tmp+clientwidth+1)=d;
				tmp+=2;
			}
			ppush+=(clientwidth<<1);
			tmp=(DWORD *)ppush;
		}
		
		// draw all know ai routes
		int maxai=gamemode?MAX_AIS:1;
		for(int a=0;a<maxai;a++)
		{
			AIROUTE *airoute=&AIRoute[a];
			
			if(airoute->count>0)
			{
				//
				int yx=airoute->startyx;
				int count=airoute->count;
				int start=airoute->start;
				while(count--)
				{
					tmp=(DWORD *)(pbegin+(yx>>YSHIFT)*(clientwidth<<1)+((yx&XMASK)<<1));
					if(tmp>(DWORD *)pbegin && tmp<(DWORD *)pend)
					{
						*tmp=airoute->color;
						*(tmp+1)=airoute->color;
						*(tmp+clientwidth)=airoute->color;
						*(tmp+clientwidth+1)=airoute->color;
					}
					
					yx+=DXY[airoute->route[start]].yx;
					start++;
				}
			}
		}
	}
}
