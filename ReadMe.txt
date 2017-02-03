
//
#define UP 0
#define RIGHT 1
#define DOWN 2
#define LEFT 3

#define cell_empty(a) (!(a)->up && !(a)->right && !(a)->down && !(a)->left)

void Setup::Map_Maze()
{
	blank(1);

	//
	typedef struct {
    unsigned int up      : 1;
    unsigned int right   : 1;
    unsigned int down    : 1;
    unsigned int left    : 1;
    unsigned int path    : 1;
    unsigned int visited : 1;
} cell_t;
cell_t maze_t[WIDTH][HEIGHT];

    maze_t mp, maze_top;
    char paths [4];
    int visits, directions;

    visits = width * height - 1;
    mp = maze;
    maze_top = mp + (width * height) - 1;

    while (visits) {
        directions = 0;

        if ((mp - width) >= maze && cell_empty (mp - width))
            paths [directions++] = UP;
        if (mp < maze_top && ((mp - maze + 1) % width) && cell_empty (mp + 1))
            paths [directions++] = RIGHT;
        if ((mp + width) <= maze_top && cell_empty (mp + width))
            paths [directions++] = DOWN;
        if (mp > maze && ((mp - maze) % width) && cell_empty (mp - 1))
            paths [directions++] = LEFT;

        if (directions) {
            visits--;
            directions = ((unsigned) rand () % directions);

            switch (paths [directions]) {
                case UP:
                    mp->up = TRUE;
                    (mp -= width)->down = TRUE;
                    break;
                case RIGHT:
                    mp->right = TRUE;
                    (++mp)->left = TRUE;
                    break;
                case DOWN:
                    mp->down = TRUE;
                    (mp += width)->up = TRUE;
                    break;
                case LEFT:
                    mp->left = TRUE;
                    (--mp)->right = TRUE;
                    break;
                default:
                    break;
            }
        } else {
            do {
                if (++mp > maze_top)
                    mp = maze;
            } while (cell_empty (mp));
        }
    }

	finish_load();
}




todo v1.20
! if leaf>max_leafs remove last leaf, then add new leaf
? presearch for all no-paths and mark as permantly_no_path if start/end there
!! consider hot-queuing if over f (or n-nodes) to prevent heaps with levels greater than x (say 6 to 7).


todo v1.14/1.15

MUST
fix a* complete.
fix A* p.q.
//fix A* array bugs
redo AIs handling
general cleanup

MAYBE
fix hlp?
DFS
add bilinear scaling to Load.
add registry load/save settings


//#define DISPLAY_AIS
enum
{
	MAX_AIS=1000,
	MAX_ROUTES=512,
	MAX_RUNLENGTH=32
};
typedef struct _ROUTE
{
	BYTE route		:3; //0-7
	BYTE runlength	:5; //0-31 +1
} ROUTE;
typedef struct _AIROUTE
{
	// debugging
	DWORD color;
	// processing
	BYTE priority; //0=done
	BYTE active			:1; //0=no ai, 1=ai
	BYTE uncompressed	:1;
	// walking
	WORD walk_point; //at what point in the path-ways are we?
	WORD walk_runlength_step; //and if on a runlength at what step?
	// encoding/decoding
	WORD startyx;
	WORD endyx;
	WORD start;  //0 to MAX_PATH_LENGTH-1
	WORD count; //if count==0 then not used
	ROUTE route[MAX_ROUTES];
} AIROUTE;

///////////////////////////////////////////////////////////////////////////

//
void Generic::Paint_All(LPBYTE pwBits, HDC hdc,HWND hWnd)
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
		vSetup->get_colorscheme_colors(background,foreground);
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
				BYTE b=(BYTE)(255-(vSetup->world[y][x].terrain_cost<<4));
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
#ifdef DISPLAY_AIS
		TCHAR szRoutes[4096];
		TCHAR sztmp[128];
#endif
		GetClientRect(hWnd, &rt);

		int maxai=gamemode?MAX_AIS:1;
		for(int a=0;a<maxai;a++)
		{
			AIROUTE *airoute=&AIRoute[a];
			
#ifdef DISPLAY_AIS
			sztmp[0]='\0';
			sprintf(szRoutes,"AI# %d count %d -- ",a,airoute->count);
#endif
			int countcheck=0;
			if(airoute->count>0)
			{
				//
				int yx=airoute->startyx;
				int count=airoute->count;
				int position=airoute->start;
				while(count--)
				{
					int runlength=airoute->route[position].runlength;
					int route=airoute->route[position].route;
					
#ifdef DISPLAY_AIS
					sprintf(sztmp,"%d(%d) ",route,runlength);
					if(strlen(sztmp)+strlen(szRoutes)<4096-1)
					strcat(szRoutes,sztmp);
#endif
					
					while(runlength-->=0)
					{
						countcheck++;
						tmp=(DWORD *)(pbegin+(yx>>YSHIFT)*(clientwidth<<1)+((yx&XMASK)<<1));
						if(tmp>(DWORD *)pbegin && tmp<(DWORD *)pend)
						{
						*tmp=airoute->color;
						*(tmp+1)=airoute->color;
						*(tmp+clientwidth)=airoute->color;
						*(tmp+clientwidth+1)=airoute->color;
						}
						
						yx+=DXY[route].yx;
					};
					position++;
				}
			}
								
#ifdef DISPLAY_AIS
			sprintf(sztmp," = countcheck %d ",countcheck);
			if(strlen(sztmp)+strlen(szRoutes)<4096-1)
			strcat(szRoutes,sztmp);
			
			rt.top=16*a;
			DrawText(hdc, szRoutes, strlen(szRoutes), &rt, DT_RIGHT);
#endif
		}
	}
}

///////////////////////////////////////////////////////////////////////////
// TYPE A
void Dijkstra::PackRoute()
{
	if(no_path)
	{
		airoute->count=0;
		return;
	}
	
	//
	airoute->start=MAX_ROUTES;
	
	WORD yx=endyx;
	
	int runlength;
	WORD route=NO_ROUTE;
	do
	{
		if(route==world[yx].route && runlength<MAX_RUNLENGTH)
		{
			airoute->route[airoute->start].route=DXY[route].route;
			airoute->route[airoute->start].runlength=runlength++;
		}
		else
		{
			runlength=0;
			airoute->start--;
			airoute->route[airoute->start].route=DXY[world[yx].route].route;
			airoute->route[airoute->start].runlength=runlength;
		}
		if(yx==startyx) break;
		route=world[yx].route;
		yx+=DXY[route].yx;
	} while(airoute->start>0 && world[yx].route!=NO_ROUTE);
	
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
// TYPE B
void AStar::PackRoute()
{
	if(no_path)
	{
		airoute->count=0;
		return;
	}
	
	//
	airoute->start=MAX_ROUTES;
	
	WORD yx=endyx;
	
	int runlength;
	WORD route=NO_ROUTE;
	do
	{
		if(route==world[yx].route && runlength<MAX_RUNLENGTH)
		{
			airoute->route[airoute->start].route=route;
			airoute->route[airoute->start].runlength=runlength++;
		}
		else
		{
			runlength=0;
			airoute->start--;
			airoute->route[airoute->start].route=world[yx].route;
			airoute->route[airoute->start].runlength=runlength;
		}
		if(yx==startyx) break;
		route=world[yx].route;
		yx+=DXY[DXY[route].route].yx;
	} while(airoute->start>0 && world[yx].route!=NO_ROUTE);
	
	airoute->start++;
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


/*
inline void AStar::movedown_heap(int k)
{
	while(NOTEMPTY_DOWN(k))
	{
		int leftk=LEFT(k);
		int rightk=RIGHT(k);
		if(NOTEMPTY_DOWN(leftk) && NOTEMPTY_DOWN(rightk) )
		{
			if(nodes[heap[leftk]].f < nodes[heap[rightk]].f)
			{
				if(nodes[heap[leftk]].f < nodes[heap[k]].f)
				{
					swap_heap(k,leftk);
					k=leftk;
				}
				else
					break;
			}
			else
			{
				if(nodes[heap[rightk]].f < nodes[heap[k]].f)
				{
					swap_heap(k,rightk);
					k=rightk;
				}
				else
					break;
			}
		}
		else if(NOTEMPTY_DOWN(leftk))
		{
			if(nodes[heap[leftk]].f < nodes[heap[k]].f)
			{
			swap_heap(k,leftk);
			k=leftk;
			}
			else
				break;
		}
		else
			break;
	}
}
*/


inline void AStar::movedown_heap(int k)
{
	while(NOTEMPTY_DOWN(k))
	{
		int leftk=LEFT(k);
		int rightk=RIGHT(k);
		if(NOTEMPTY_DOWN(leftk) && NOTEMPTY_DOWN(rightk) )
		{
			if(nodes[heap[leftk]].f < nodes[heap[rightk]].f)
			{
				if(nodes[heap[leftk]].f < nodes[heap[k]].f)
				{
					swap_heap(k,leftk);
					k=leftk;
				}
				else
					break;
			}
			else
			{
				if(nodes[heap[rightk]].f < nodes[heap[k]].f)
				{
					swap_heap(k,rightk);
					k=rightk;
				}
				else
					break;
			}
		}
		else if(NOTEMPTY_DOWN(leftk))
		{
			if(nodes[heap[leftk]].f < nodes[heap[k]].f)
			{
			swap_heap(k,leftk);
			k=leftk;
			}
			else
				break;
		}
		else
			break;
	}
}




inline void AStar::moveup_heap(int k)
{
	if(!EMPTY(k))
	{
		int parentk=PARENT(k);
		if(!EMPTY(parentk))
		{
			if(nodes[heap[k]].f <= nodes[heap[parentk]].f)
			{
				swap_heap(k,parentk);
				moveup_heap(parentk);
			}
		}
	}
}
inline void AStar::movedown_heap(int k)
{
	if(!EMPTY(k))
	{
		register int leftk=LEFT(k);
		register int rightk=RIGHT(k);
		if(!EMPTY(leftk) && !EMPTY(rightk) )
		{
			if(nodes[heap[leftk]].f <= nodes[heap[rightk]].f)
			{
				if(nodes[heap[leftk]].f <= nodes[heap[k]].f)
				{
					swap_heap(k,leftk);
					movedown_heap(leftk);
				}
			}
			else
			{
				if(nodes[heap[rightk]].f <= nodes[heap[k]].f)
				{
					swap_heap(k,rightk);
					movedown_heap(rightk);
				}
			}
		}
		else if(!EMPTY(leftk))
		{
			swap_heap(k,leftk);
			movedown_heap(leftk);
		}
	}
}


--
/*
{ //backtrack
	register int node=last_node;
	register int index=last_index;

	current_node=EMPTY_NODE;
	do
	{
		while(open_node_index[index]==EMPTY_NODE && index>0) index--;
		node=open_node_index[index];
		if(node==EMPTY_NODE) break;
		if(
			nodes[node].open &&
			nodes[node].ancestor!=EMPTY_NODE &&
			nodes[node].f<=nodes[current_node].f
			)
		{
			current_node=node;
		}
	} while(--index>0);
	if(current_node==EMPTY_NODE) no_path=true;
}
*/
/*
{ //backtrack
	register float best_f=MAX_F;
	register int node=last_node;
	
	do
	{
		if(nodes[node].open && nodes[node].ancestor!=EMPTY_NODE && nodes[node].f<=best_f)
		{
			best_f=nodes[node].f;
			current_node=node;
		}
	} while(node--!=EMPTY_NODE);
	if(node==EMPTY_NODE)
		no_path=true;
}


  */
