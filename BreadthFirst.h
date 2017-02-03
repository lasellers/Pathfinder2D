// BreadthFirst.h: interface for the BreadthFirst class.
//
//////////////////////////////////////////////////////////////////////

#if !defined(H_BREADTHFIRST)
#define H_BREADTHFIRST

#if _MSC_VER > 1000
#pragma once
#endif // _MSC_VER > 1000

//
#include "Setup.h"

//
class CBreadthFirst
{
public:
	CBreadthFirst();
	virtual ~CBreadthFirst();
	CBreadthFirst(CSetup *Setup, AIROUTE *airoute);
	void PackRoute();
	
	void Reset(AIROUTE *airoute);
	void FindPath();
	void UpdateWorld();
	void UpdateSettings();
	void GetOptions();
	
	void Paint(LPBYTE pwBits, HDC hdc,HWND hWnd);
	
private:
	inline void expand_node_to_open();
	inline void move_first_node_to_closed();
	
enum
{
	UNKNOWN,
		OPEN,
		CLOSED
};
enum
{
	NO_ROUTE=8,
		MAX_OPEN_NODES=4096, //(WIDTH+HEIGHT)*4
};

public:
	CSetup *Setup;
	AIROUTE *airoute;
	
	LARGE_INTEGER bigtick;
	
	int iterations_per_frame;
	int frame;

	bool path_found;
	bool no_path;
	
	WORD first_node;
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
	} DXY[8+1];
	
public:
	struct _WORLD
	{
		BYTE terrain_cost	:4;
		BYTE route			:4; // 0-3 or 0-7, NO_ANCESTOR
		BYTE state; //			:2;
	} world[WIDTH*HEIGHT];
	
	struct _NODES
	{
		WORD yx;
	}  nodes[MAX_OPEN_NODES+1];
};

#endif
