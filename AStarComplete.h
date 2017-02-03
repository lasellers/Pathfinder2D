// AStarComplete.h: interface for the AStarComplete class.
//
//////////////////////////////////////////////////////////////////////

#if !defined(H_ASTAR_RECURSIVE)
#define H_ASTAR_RECURSIVE

#if AStarComplete > 1000
#pragma once
#endif // AStarComplete > 1000

//
#include "Setup.h"

//
class CAStarComplete  
{
public:
	CAStarComplete();
	virtual ~CAStarComplete();
	CAStarComplete(CSetup *Setup, AIROUTE *airoute);
	void PackRoute();
	
	void Reset(AIROUTE *airoute);
	void FindPath();
	void UpdateWorld();
	void UpdateSettings();
	void GetOptions();
	
	void Paint(LPBYTE pwBits, HDC hdc,HWND hWnd);
	
private:
	inline float distance(WORD yx);
	inline void expand_nodes_to_open(WORD parent_node);
	inline void expand_nodes_to_open_terrain(WORD parent_node);
	void with_terrain(int iterations_per_frame);
	void without_terrain(int iterations_per_frame);
	inline WORD find_sorted_node_position(float f);
	void move_node_to_closed(WORD parent_node);
	inline void reopen_closed_node(
		float f,
		float g,
		WORD reopened_node,
		WORD yx,
		BYTE d,
		WORD parent_node
		);
	
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
	
	int iterations_per_frame;
	
	int frame;
	bool path_found;
	bool no_path;
	
	bool use_terrain;
	float cost;
	float diagonal_cost;
	
	WORD distance_method;
	
	WORD best_node;
	WORD closed_node;
	
	WORD open_nodes;
	WORD closed_nodes;
	WORD reopened_nodes;
	
	WORD directions;
	
	WORD startyx;
	WORD endyx;
	WORD free_node;
	
	float median_terrain_cost;
	
private:
	struct _D_TO_XY_LOOKUP
	{
		short yx;
		BYTE route;
		float cost_multiplier;
		float terrain_cost_multiplier;
	} DXY[8+1];
	
	public:
		struct _WORLD
		{
			BYTE terrain_cost	:4;
			BYTE state			:2; //optimization: this is used so we can avoid some expensive searching through
			// the nodes when trying to determine if a point is in the open/closed list already.
			// UNKNOWN, CLOSED, OPEN, IMPASSABLE
			BYTE route			; //used when packing the route into a runlength stream at end of pathfinding
			WORD node;
		} world[WIDTH*HEIGHT];
		
		// nodes and nodesdata split node data into two parts in an attempt to minmize
		//cache miss when searching through nodes for f values.
		struct _NODES
		{
			WORD prev;
			WORD next;
			float f; //g+h
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
