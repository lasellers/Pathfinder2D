// BestFirst.h: interface for the BestFirst class.
//
//////////////////////////////////////////////////////////////////////

#if !defined(H_BESTFIRST)
#define H_BESTFIRST

#if _MSC_VER > 1000
#pragma once
#endif // _MSC_VER > 1000

//
#include "Setup.h"

//
class CBestFirst  
{
public:
	CBestFirst();
	virtual ~CBestFirst();
	CBestFirst(CSetup *Setup, AIROUTE *airoute);
	void PackRoute();
	
	void Reset(AIROUTE *airoute);
	void FindPath();
	void UpdateWorld();
	void UpdateSettings();
	void GetOptions();
	
	void Paint(LPBYTE pwBits, HDC hdc,HWND hWnd);

	//
	enum
	{
		MIN_NODE=1,
			MAX_NODE=WIDTH*HEIGHT, // typically only 1 to 4k is needed.. except on the mars map
			MAX_HEAP_LEAFS=4096, //8192,
			NO_ROUTE=8
	};
	enum
	{
		UNKNOWN=0,
			IMPASSABLE=1,
			OPEN=2,
			CLOSED=3
	};
	enum
	{
		FINDING=0,
			NO_PATH=1,
			PATH_FOUND=2
	};
	
public:
	LARGE_INTEGER bigtick;

	CSetup *Setup;
	AIROUTE *airoute;
	
	int iterations_per_frame;
	int frame;

	WORD distance_method;
	
	WORD best_node;
	WORD free_node;
	
	WORD startyx;
	WORD endyx;

	BYTE pathing_state;
	BYTE directions;

	int endy, endx; //ints so we don't have to clear register first. used with distance()

private:
	struct _D_TO_XY_LOOKUP
	{
		short yx;
		BYTE route;
	} DXY[8+1];
	
	//
	enum
	{
		ROOT_HEAP=1
	};
	WORD last_heap_leaf;
	WORD heap[MAX_HEAP_LEAFS]; //node

public:
	//
	struct _WORLD
	{
		BYTE terrain_cost;	// remove me?
		BYTE state			:4; //:2; this is used so we can avoid some expensive searching through
		// the nodes when trying to determine if a point is in the open/closed list already.
		// UNKNOWN, CLOSED, OPEN, IMPASSABLE
		BYTE route			:4; //:3; // :3; //used when packing the route into a runlength stream at end of pathfinding
		//with a single BYTE DEBUG1 saves only .3ms over two BYTEs for WORLD.
	} world[WIDTH*HEIGHT];

	//
	struct _NODES
	{
		float h;
		WORD yx;
	}  nodes[MAX_NODE+1];

	//
	bool path_found;
	bool no_path;
	WORD closed_node;
	WORD open_nodes;
	WORD closed_nodes;

private:
	inline float distance(const WORD yx);
			
	//
	inline int LEFT(int k);
	inline int RIGHT(int k);
	inline int PARENT(int k);
	inline bool NOTEMPTY_UP(int k);
	inline bool NOTEMPTY_DOWN(int k);
	inline void swap_heap(const int k1, const int k2);
	void insert_node_to_heap(WORD node);
	void remove_root_from_heap();
};

#endif
