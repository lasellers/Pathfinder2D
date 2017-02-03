// Dijkstra.h: interface for the Dijkstra class.
//
//////////////////////////////////////////////////////////////////////

#if !defined(H_DIJKSTRA)
#define H_DIJKSTRA

#if _MSC_VER > 1000
#pragma once
#endif // _MSC_VER > 1000

#include "Setup.h"

//
class CDijkstra
{
public:
	CDijkstra();
	virtual ~CDijkstra();
	CDijkstra(CSetup *Setup, AIROUTE *airoute);
	void PackRoute();

	void Reset(AIROUTE *airoute);
	void FindPath();
	void UpdateWorld();
	void UpdateSettings();
	void GetOptions();

	void Paint(LPBYTE pwBits, HDC hdc,HWND hWnd);

private:
	inline void add_nodes_to_open();
	inline void move_best_node_to_closed();
	inline WORD find_free_node_for_add();	
	inline void free_best_node();
	inline WORD find_sorted_node_position(float newcost);

enum
{
	UNKNOWN,
		OPEN,
		CLOSED
};
enum
{
	NO_ROUTE=8,
		//max nodes is INCLUSION. ie, this is a valid node.
	MAX_OPEN_NODES=WIDTH*HEIGHT //(WIDTH+HEIGHT+WIDTH+HEIGHT)*6
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

	WORD best_node;
	
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
	} DXY[8+1];
	
public:
	struct _WORLD
	{
		BYTE terrain_cost	:4;
		BYTE route			:4; // 0-3 or 0-7, NO_ANCESTOR
		BYTE state; //			:2;
		float cost;
	} world[WIDTH*HEIGHT];
	
	struct _NODES
	{
		WORD yx;
		float cost;
		WORD prev;
		WORD next;
	}  nodes[MAX_OPEN_NODES+1];
};

#endif
