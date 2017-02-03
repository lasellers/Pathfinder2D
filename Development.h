// Development.h: interface for the Development class.
//
//////////////////////////////////////////////////////////////////////

#if !defined(H_DEVELOPMENT)
#define H_DEVELOPMENT

#if _MSC_VER > 1000
#pragma once
#endif // _MSC_VER > 1000

//
#include "Setup.h"

//
class CDevelopment  
{
public:
	CDevelopment();
	virtual ~CDevelopment();
	CDevelopment(CSetup *Setup, AIROUTE *airoute);
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
		MAX_NODE=WIDTH*HEIGHT, // typically only 1 to 4k is needed.. except on the mars map
		MAX_HEAP_LEAFS=((WIDTH*HEIGHT)/16), //4096 8192,
		NO_ROUTE=8
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

	int endy, endx; //ints so we don't have to clear register first. used with distance()

	WORD closed_yx;
	
	WORD startyx;
	WORD endyx;

	BYTE pathing_state;
	BYTE directions;

private:
	struct _D_TO_XY_LOOKUP
	{
		WORD cost_multiplier;
		short yx;
		BYTE route;
	} DXY[8+1];
	
	//
	enum
	{
		ROOT_HEAP=1
	};
	WORD last_heap_leaf;
	struct _HEAP
	{
	WORD f;
	WORD g;
	WORD node;
	} heap[MAX_HEAP_LEAFS];

//public:
	/*
	as of v1.18 _NODES and _WORLD have been combined into a single data structure.
	This reduces away several redundancies but does make the structure somewhat
	more difficult to follow because of some aspects.
	For example: "node" is now the same thing as "yx".
	*/
	struct _WORLD //4+4+2
	{
		union
		{
			struct
			{
		BYTE terrain_cost	:4;
		BYTE visited		:1; //:2; this is used so we can avoid some expensive searching through
		// the worlds when trying to determine if a point is in the open/closed list already.
		// UNKNOWN, CLOSED, OPEN, IMPASSABLE
		BYTE route			:3; //:3; // :3; //used when packing the route into a runlength stream at end of pathfinding
		//with a single BYTE DEBUG1 saves only .3ms over two BYTEs for WORLD.
			} state;
		BYTE raw;
		} u;
	} world[WIDTH*HEIGHT];

public:
	//
	bool path_found;
	bool no_path;
	bool use_terrain;

private:
	void without_terrain();
	void with_terrain();
	inline WORD distance(const WORD yx);
			
	//
	inline int LEFT(int k);
	inline int RIGHT(int k);
	inline int PARENT(int k);
	inline bool NOTEMPTY_UP(int k);
	inline bool NOTEMPTY_DOWN(int k);
	inline void swap_heap(const int k1, const int k2);
	inline void insert_node_to_heap(const WORD node, const WORD g);
	inline void remove_root_from_heap();
};

#endif
