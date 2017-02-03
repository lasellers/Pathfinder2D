// Generic.h: interface for the Generic class.
//
//////////////////////////////////////////////////////////////////////

#if !defined(H_GENERIC)
#define H_GENERIC

#if _MSC_VER > 1000
#pragma once
#endif // _MSC_VER > 1000

#include "Setup.h"

//
enum
{
	NONE_ALGORITHM,
		DEVELOPMENT_ALGORITHM,
		ASTAR_ALGORITHM,
		ASTAR_HEAPINTEGER_ALGORITHM,
		ASTAR_HEAP_ALGORITHM,
		ASTAR_COMPLETE_ALGORITHM,
		ASTAR_ARRAY_ALGORITHM,
		ASTAR_LINKEDLIST_ALGORITHM,
		DIJKSTRA_ALGORITHM,
		
		BREADTHFIRST_ALGORITHM,
		BESTFIRST_ALGORITHM,
		DEPTHFIRST_ALGORITHM,

		BIBFS_ALGORITHM,
		DSTAR_ALGORITHM,
		RIGHTHANDRULE_ALGORITHM,
};


#include "Development.h"
#include "AStar.h"
#include "AStarHeapInteger.h"
#include "AStarHeap.h"
#include "AStarLinkedList.h"
#include "AStarComplete.h"
#include "AStarArray.h"
#include "Dijkstra.h"

#include "BreadthFirst.h"
#include "BestFirst.h"
#include "DepthFirst.h"

#include "RightHandRule.h"
#include "DStar.h"

//
class CGeneric  
{
public:
	CGeneric();
	virtual ~CGeneric();
	CGeneric(CSetup *Setup);
	
	//
	void Reset();
	void Paint(LPBYTE pwBits,HDC hdctmp,HWND hWnd);
	void UpdateWorld();
	void UpdateSettings();
	void FindPath();
	void GetOptions();
	void Switch_Algorithms(int Alg);
	void set_GameMode(bool state);
	
	bool PathFound();
	bool NoPath();
	void ResetAI();
	
	bool isDone();

	void Paint_All(LPBYTE pwBits, HDC hdc,HWND hWnd);
	
	//
public:
	int Alg;
	int ai;

	LARGE_INTEGER bigtick;

	bool gamemode;
	bool routing_view;

	CSetup *Setup;

private:
	//
	CDevelopment *Development;
	CAStar *AStar;
	CAStarHeapInteger *AStarHeapInteger;
	CAStarHeap *AStarHeap;
	CAStarComplete *AStarComplete;
	CAStarLinkedList *AStarLinkedList;
	CAStarArray *AStarArray;
	CDijkstra *Dijkstra;
	CBreadthFirst *BreadthFirst;
	CBestFirst *BestFirst;
	CDepthFirst *DepthFirst;

	//
	AIROUTE AIRoute[MAX_AIS+1];

	//
	struct _D_TO_XY_LOOKUP
	{
		short yx;
		BYTE route;
	} DXY[8+1];

};

#endif
