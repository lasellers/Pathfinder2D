// AStarLinkedList.h: interface for the AStarLinkedList class.
//
//////////////////////////////////////////////////////////////////////

#if !defined(H_ASTAR_LINKEDLIST)
#define H_ASTAR_LINKEDLIST

#if _MSC_VER > 1000
#pragma once
#endif // _MSC_VER > 1000

//
#include "Setup.h"

//
class CAStarLinkedList	
{
public:
	CAStarLinkedList();
	virtual ~CAStarLinkedList();
	CAStarLinkedList(CSetup *Setup, AIROUTE *airoute);
	void PackRoute();
	
	void Reset(AIROUTE *airoute);
	void FindPath();
	void UpdateWorld();
	void UpdateSettings();
	void GetOptions();
	
	void Paint(LPBYTE pwBits, HDC hdc,HWND hWnd);
	
private:
	inline float distance(WORD yx);
	inline void add_nodes_to_open(WORD parent_node);
	inline void add_nodes_to_open_terrain(WORD parent_node);
	inline WORD find_sorted_node_position(float f);
	void move_node_to_closed(WORD parent_node);
	void with_terrain(int iterations_per_frame);
	void without_terrain(int iterations_per_frame);
	
	enum
	{
		MIN_NODE=1,
			MAX_NODE=WIDTH*HEIGHT, // typically only 1 to 4k is needed.. except on the mars map
			NO_ROUTE=8
	};
	enum
	{
		UNKNOWN=0,
			IMPASSABLE=1,
			OPEN=2,
			CLOSED=3
	};
	
public:
	CSetup *Setup;
	AIROUTE *airoute;
	
	LARGE_INTEGER bigtick;
	
	bool path_found;
	bool no_path;
	bool use_terrain;
	
	//
	int frame;
	int iterations_per_frame;
	//float cost;
	//float diagonal_cost;
	//float median_terrain_cost;
	
	//
	WORD distance_method;
	
	WORD best_node;
	WORD closed_node;
	
	WORD open_nodes;
	WORD closed_nodes;
	
	WORD free_node;
	
	WORD directions;
	
	WORD startyx;
	WORD endyx;
	
	int endy, endx; //ints so we don't have to clear register first. used with distance()
	
private:
	struct _D_TO_XY_LOOKUP
	{
		short yx;
		float cost_multiplier;
		BYTE route;
	} DXY[8+1];
	
	public:
		struct _WORLD
		{
			BYTE terrain_cost	:4;
			BYTE state			:2; //optimization: this is used so we can avoid some expensive searching through
			// the nodes when trying to determine if a point is in the open/closed list already.
			// UNKNOWN, CLOSED, OPEN, IMPASSABLE
			BYTE route			; //used when packing the route into a runlength stream at end of pathfinding
		} world[WIDTH*HEIGHT];
		
		// nodes and nodesdata split node data into two parts in an attempt to minmize
		//cache miss when searching through nodes for f values.
		struct _NODES
		{
			WORD prev;
			WORD next;
			float f; //g+h
			WORD yx;
		}  nodes[MAX_NODE+1];
		struct _NODESDATA
		{
			WORD ancestor;
			WORD successor[8]; //0-3 or 0-7 directions
			WORD yx;
			float g; //sum
		}  nodesdata[MAX_NODE+1];
};

#endif
