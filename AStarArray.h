// AStarArray.h: interface for the AStarArray class.
//
//////////////////////////////////////////////////////////////////////

#if !defined(H_ASTARARRAY)
#define H_ASTARARRAY

#if _MSC_VER > 1000
#pragma once
#endif // _MSC_VER > 1000

//
#include "Setup.h"

//
class CAStarArray  
{
public:
	CAStarArray();
	virtual ~CAStarArray();
	CAStarArray(CSetup *Setup, AIROUTE *airoute);
	void PackRoute();
	
	void Reset(AIROUTE *airoute);
	void FindPath();
	void UpdateWorld();
	void UpdateSettings();
	void GetOptions();
	
	void Paint(LPBYTE pwBits, HDC hdc,HWND hWnd);
	
private:
	inline float distance(WORD yx);
	inline bool move_node_to_closed(WORD node);
	inline void open_successor_nodes();
	inline void backtrack_to_find_best_node_to_explore_off_of();
	inline void keep_moving_forward_through_best_successor_node();
	
	enum
	{
		MAX_NODES=WIDTH*HEIGHT, // typically only 1 to 4k is needed.. except on the mars map
			MAX_OPEN_NODE_INDEXES=WIDTH*HEIGHT, //4096, //optimization c
			MAX_F=16*256*256,
			NO_ROUTE=8
	};
	enum
	{
		UNKNOWN,
			OPEN,
			CLOSED
	};
	
public:
	CSetup *Setup;
	AIROUTE *airoute;
	
	LARGE_INTEGER bigtick;
	
	int iterations_per_frame;
	int frame;
	
	bool path_found;
	bool no_path;
	bool use_terrain;

	WORD distance_method;
	
	WORD current_node;
	WORD last_node; 
	
	WORD open_nodes;
	WORD closed_nodes;
	
	WORD directions;
	
	WORD startyx;
	WORD endyx;
	
	private:
	struct _D_TO_XY_LOOKUP
	{
		short yx;
		BYTE route;
		float cost_multiplier;
	} DXY[8+1];
	
	public:
	struct _WORLD
	{
		BYTE terrain_cost	:4;
		BYTE state			:2; //optimization: this is used so we can avoid some expensive searching through
		// the nodes when trying to determine if a point is in the open/closed list already.
		// UNKNOWN, CLOSED, OPEN
		BYTE route			; // 3;used when packing the route into a runlength stream at end of pathfinding
	} world[WIDTH*HEIGHT];
	
	struct _NODES
	{
		WORD yx;
		float f; //g+h
		float g; //sum
		float h; //dist to goal
		WORD ancestor;
		WORD successor[8]; //0-3 or 0-7 directions
		bool open; //false==closed true==open
		WORD open_node_index;
	}  nodes[MAX_NODES+1];
	
	WORD last_index;
	WORD open_node_index[MAX_OPEN_NODE_INDEXES]; //optimization c

};

#endif
