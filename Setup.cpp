// Setup.cpp: implementation of the Setup class.
//
//////////////////////////////////////////////////////////////////////

#include "stdafx.h"
#include "Setup.h"

#include "stdio.h"

#include "math.h" //sqrt

#include "stdlib.h" //for rand
#include <time.h> //for time for srand
#include "string.h"
#include "io.h"  //for _open _close ,etc
#include <fcntl.h> //file constants
#include <sys/stat.h>



//////////////////////////////////////////////////////////////////////
// Construction/Destruction
//////////////////////////////////////////////////////////////////////

CSetup::CSetup()
{
	use_terrain=false;
	distance_method=MANHATTAN_DISTANCE;
	cost=1.0f; //0.25f;
	diagonal_cost=1.0f;
	
	iterations_per_frame=1;
	
	directions=8; //4;
	
	median_terrain_cost=1.0f;
	median_terrain_cost_auto=1.0f;
	
	startx=10;
	starty=10;
	
	endx=(BYTE)(WIDTH-21);
	endy=(BYTE)(HEIGHT-21);
	
	for(int y=0;y<HEIGHT;y++)
	{
		for(int x=0;x<WIDTH;x++)
		{
			world[y][x].terrain_cost=0;
		}
	}
	
	//
	colorscheme=COLORSCHEME_BLACK;
	
	//
	options.uniform_cost=false;
	options.terrain_cost=false;
	options.distance=false;
	options.search_directions=false;

	//
	okToPath=true;

	//
	frame=0;
	bigtick.QuadPart=0;

	// presearch
	presearch_toggle=true;
	presearch_maxgroup=1;

		// make the routing lookup table
	DXY[0].y=-1;	DXY[0].x=0;
	DXY[1].y=0;		DXY[1].x=1;
	DXY[2].y=1;		DXY[2].x=0;
	DXY[3].y=0;		DXY[3].x=-1;

	DXY[4].y=-1;	DXY[4].x=1;
	DXY[5].y=1;		DXY[5].x=1;
	DXY[6].y=-1;	DXY[6].x=-1;
	DXY[7].y=1;		DXY[7].x=-1;

	//in case a NO_ROUTE accidentally is passed for lookup
	DXY[8].y=0;		DXY[8].x=0;
}

CSetup::~CSetup()
{
}

// ////////////////////////////////////////////////////////////////////////
// ////////////////////////////////////////////////////////////////////////
// ////////////////////////////////////////////////////////////////////////
/*
On Sat, 07 Jun 2003 11:36:47 GMT, Eternal Vigilance <wotan@oneeye.com> wrote:
>
>A frequent trick is also to have the edge of the map be BLOCKED nodes so that
>you  can eliminate the test:
>if (x < 0 || x >= width || y < 0 || y >= height)  return;       which rarely
>fails and thus is executed alot....

  Good point.
  
	//		(x<0 || x>=WIDTH || y<0 || y>=HEIGHT) || //if x,y is off edges of map ignore
*/
void CSetup::blocking_walls()
{
	register int n;
	
	for(n=1;n<HEIGHT-1;n++)
	{
		world[n][0].terrain_cost=IMPASSABLE_TERRAIN_COST;
		world[n][WIDTH-1].terrain_cost=IMPASSABLE_TERRAIN_COST;
	}
	for(n=0;n<WIDTH;n++)
	{
		world[0][n].terrain_cost=IMPASSABLE_TERRAIN_COST;
		world[HEIGHT-1][n].terrain_cost=IMPASSABLE_TERRAIN_COST;
	}
}

void CSetup::blank(BYTE elevation)
{
	for(register int y=0;y<HEIGHT;y++)
	{
		for(register int x=0;x<WIDTH;x++)
		{
			world[y][x].terrain_cost=elevation;
		}
	}
}

void CSetup::rectangle(int x0, int y0, int x1, int y1, BYTE elevation)
{
	if(x0<0) x0=0;
	if(y0<0) y0=0;
	if(x1>WIDTH-1) x1=WIDTH-1;
	if(y1>HEIGHT-1) y1=HEIGHT-1;
	for(register int y=y0;y<y1;y++)
	{
		for(register int x=x0;x<x1;x++)
		{
			world[y][x].terrain_cost=elevation;
		}
	}
}
void CSetup::openrectangle(int x0, int y0, int x1, int y1, BYTE elevation)
{
	if(x0<0) x0=0;
	if(y0<0) y0=0;
	if(x1>WIDTH-1) x1=WIDTH-1;
	if(y1>HEIGHT-1) y1=HEIGHT-1;
	for(int y=y0;y<y1;y++)
	{
		world[y][x0].terrain_cost=elevation;
		world[y][x1].terrain_cost=elevation;
		world[y][x0+1].terrain_cost=elevation;
		world[y][x1-1].terrain_cost=elevation;
	}
	for(int x=x0;x<x1;x++)
	{
		world[y0][x].terrain_cost=elevation;
		world[y1][x].terrain_cost=elevation;
		world[y0+1][x].terrain_cost=elevation;
		world[y1-1][x].terrain_cost=elevation;
	}
}

void CSetup::finish_load()
{
	blocking_walls();
	median();
	Presearch();
}


///////////////////////////////////////////////////////////////////////////
// 45,60 - 155,140
// 85,30 - 110, 60
void CSetup::Map_BoxOnBoxNoG()
{
	blank(0);
	rectangle(WIDTH/5,HEIGHT/3,WIDTH-WIDTH/5,HEIGHT-HEIGHT/4,IMPASSABLE_TERRAIN_COST);
	rectangle(WIDTH/2-WIDTH/8,HEIGHT/8,WIDTH/2+WIDTH/8,HEIGHT/3,4);
	
	finish_load();
}

void CSetup::Map_BoxOnBox()
{
	blank(1);
	rectangle(WIDTH/5,HEIGHT/3,WIDTH-WIDTH/5,HEIGHT-HEIGHT/4,IMPASSABLE_TERRAIN_COST);
	rectangle(WIDTH/2-WIDTH/8,HEIGHT/8,WIDTH/2+WIDTH/8,HEIGHT/3,4);
	
	finish_load();
}

void CSetup::Map_CheckerBoard()
{
	blank(1);
	
	for(int y=0;y<HEIGHT;y+=2)
	{
		for(int x=0;x<WIDTH;x+=2)
		{
			world[y][x+((y>>1)&1)].terrain_cost=IMPASSABLE_TERRAIN_COST;
		}
	}
	
	world[starty][startx].terrain_cost=0;
	world[endy][endx].terrain_cost=0;

	finish_load();
}

//
void CSetup::Map_Grid()
{
	blank(1);
	
	for(int y=0;y<HEIGHT;y+=2)
	{
		for(int x=1;x<WIDTH;x+=2)
		{
			world[y][x].terrain_cost=IMPASSABLE_TERRAIN_COST;
		}
	}
	
	world[starty][startx].terrain_cost=0;
	world[endy][endx].terrain_cost=0;

	finish_load();
}

void CSetup::Map_PipeMaze()
{
	blank(1);
	for(int x=1;x<WIDTH;x+=2)
	{
		if(x&2)
		{
			for(int y=2;y<HEIGHT;y++) world[y][x].terrain_cost=IMPASSABLE_TERRAIN_COST;
		}
		else
		{
			for(int y=0;y<HEIGHT-2;y++) world[y][x].terrain_cost=IMPASSABLE_TERRAIN_COST;
		}
	}
	
	world[starty][startx].terrain_cost=0;
	world[endy][endx].terrain_cost=0;

	finish_load();
}

void CSetup::Map_Random()
{
	blank(1);
	
	int y,x,c;
	for(int n=0;n<2000;n++)
	{
		y=rand()%HEIGHT;
		x=rand()%WIDTH;
		c=rand()%16;
		world[y][x].terrain_cost=c;
	}
	
	finish_load();
}


void CSetup::Map_Clear_Path()
{
	blank(0);

	finish_load();
}

void CSetup::Map_No_Path()
{
	for(register int y=0;y<HEIGHT;y++)
	{
		for(register int x=0;x<WIDTH;x++)
		{
			world[y][x].terrain_cost=IMPASSABLE_TERRAIN_COST;
		}
	}

	finish_load();
}

void CSetup::Map_StraightLine()
{
	blank(1);
	
	for(register int y=HEIGHT/2-HEIGHT/20;y<HEIGHT/2+HEIGHT/20;y++)
	{
		for(register int x=WIDTH/4;x<WIDTH-WIDTH/4;x++)
		{
			world[y][x].terrain_cost=IMPASSABLE_TERRAIN_COST;
		}
	}
	
	finish_load();
}

void CSetup::Map_CrashMe()
{
	blank(1);
	
	for(int x=0;x<WIDTH;x+=2)
	{
		if(x&2)
		{
			for(int y=1;y<HEIGHT;y++) world[y][x].terrain_cost=IMPASSABLE_TERRAIN_COST;
		}
		else
		{
			for(int y=0;y<HEIGHT-1;y++) world[y][x].terrain_cost=IMPASSABLE_TERRAIN_COST;
		}
	}
	
	world[starty][startx].terrain_cost=0;
	world[endy][endx].terrain_cost=0;

	finish_load();
}

void CSetup::Map_BigBox()
{
	blank(1);
	rectangle(WIDTH/8,HEIGHT/8,WIDTH-WIDTH/8,HEIGHT-HEIGHT/8,IMPASSABLE_TERRAIN_COST);
	
	finish_load();
}

void CSetup::Map_RandomBoxes()
{
	blank(1);
	
	int y,x;
	BYTE c;
	for(int n=0;n<2000;n++)
	{
		y=rand()%HEIGHT;
		x=rand()%WIDTH;
		c=rand()%16;
		rectangle(x-5,y-5,x+5,y+5,c);
	}
	
	finish_load();
}

void CSetup::Map_RandomOpenBoxes()
{
	blank(1);
	
	int y,x;
	BYTE c;
	for(int n=0;n<2000;n++)
	{
		y=rand()%HEIGHT;
		x=rand()%WIDTH;
		c=rand()%16;
		openrectangle(x-5,y-5,x+5,y+5,c);
	}
	
	finish_load();
}

void CSetup::Map_RandomTerrain()
{
	blank(1);
	
	for(int y=0;y<HEIGHT;y++)
	{
	for(int x=0;x<WIDTH;x++)
	{
		world[y][x].terrain_cost=rand()%16;
	}
	}
	
	finish_load();
}

void CSetup::Map_Gates()
{
	blank(1);

	for(int y=10;y<HEIGHT-10;y+=10)
	{
	for(int x=10;x<WIDTH-10;x++)
	{
		world[y][x].terrain_cost=IMPASSABLE_TERRAIN_COST;
	}
	}

	finish_load();
}

void CSetup::Map_ZigZag()
{
	blank(1);

	int wx,wy;
	for(wx=2,wy=2;wx<WIDTH-2 && wy<HEIGHT-2;wx+=4,wy+=4)
	{

		int x=2;
		int y=wy;
		while(x<wx && y>2)
		{
			world[y][x].terrain_cost=IMPASSABLE_TERRAIN_COST;
			x++;
			y--;
		}
	}

	finish_load();
}










// ////////////////////////////////////////////////////////////////////////
// ////////////////////////////////////////////////////////////////////////
// ////////////////////////////////////////////////////////////////////////

//
void CSetup::Load(const char *filename)
{
	//
	BYTE *infile=NULL;
	int inlength=0;
	
	//
	const int stream=_open( (const char *)filename, _O_RDONLY | _O_BINARY );
	if(stream!=-1)
	{
		//
		inlength=_filelength(stream);
		
		infile = new BYTE[inlength];
		if(infile)
		{
			memset(infile,0,inlength);
			_read( stream, infile, inlength);
		}

		//
		_close(stream);
	}
	
	
	// B
	if(infile)
	{
		//
		if (inlength<sizeof(Header))
			; //printf("Header length WORD.");
		else
		{
			//
			BYTE* s=(BYTE *)infile;
			
			///////////////////////////////////////////////////////////
			//
			Header.IDLength=READBYTE(s);
			Header.ColorMapType=READBYTE(s);
			Header.ImageType=READBYTE(s);
			
			Header.ColorMapOrigin=READWORD(s);
			Header.ColorMapLength=READWORD(s);
			Header.ColorMapEntrySize=READBYTE(s);
			
			Header.XOrigin=READWORD(s);
			Header.YOrigin=READWORD(s);
			
			Header.Width=READWORD(s);
			Header.Height=READWORD(s);
			
			int count=Header.Width*Header.Height;
			
			Header.ImagePixelSize=READBYTE(s);
			Header.ImageDescriptorByte=READBYTE(s);
			
			// 			int AlphaChannelBits=Header.ImageDescriptorByte & 15;
			//			int ImageOrigin=(Header.ImageDescriptorByte>>4)&3;
			// 0= bottom left
			// 1=bottom right
			// 2=top left (default)
			// 3=top right
			bool toptobottom=(Header.ImageDescriptorByte&32)?1:0;
			//			bool lefttoright=(Header.ImageDescriptorByte&64)?1:0;
			
			//
			if(Header.IDLength>0)
			{
				memcpy(CIF,s,Header.IDLength);
				CIF[Header.IDLength]='\0';
				s+=(int)Header.IDLength;
			}
			
			//
			if(Header.Width!=WIDTH && Header.Height!=HEIGHT)
				; //printf("TGA is not %d by %d\n",width,height);
			else
			{
				//
				if(Header.ColorMapType==1 && Header.ColorMapLength>0)
				{
					memset(ColorMap,0,sizeof(ColorMap));
					
					if(Header.ColorMapEntrySize==16)
					{
						for(int i=0;i<Header.ColorMapLength;i++)
						{
							WORD w=READWORD(s);
							
							BYTE r,g,b;
							b=(BYTE)((w&31)<<3);
							g=(BYTE)(((w>>5)&31)<<3);
							r=(BYTE)(((w>>10)&31)<<3);
							
							ColorMap[i].red=r;
							ColorMap[i].green=g;
							ColorMap[i].blue=b;
						}
					}
					else if(Header.ColorMapEntrySize==24)
					{
						for(int i=0;i<Header.ColorMapLength;i++)
						{
							ColorMap[i].blue=READBYTE(s);
							ColorMap[i].green=READBYTE(s);
							ColorMap[i].red=READBYTE(s);
						}
					}
					else if(Header.ColorMapEntrySize==32)
					{
						for(int i=0;i<Header.ColorMapLength;i++)
						{
							ColorMap[i].blue=READBYTE(s);
							ColorMap[i].green=READBYTE(s);
							ColorMap[i].red=READBYTE(s);
							*s++;
						}
					}
				}
				
				
				///////////////////////////////////////////////////////////
				if(toptobottom)
				{
					switch(Header.ImageType)
					{
					case TGA_UncompressedColorMapped:
						if(Header.ImagePixelSize==8)
						{
							for(int y=0;y<Header.Height;y++)
							{
								for(int x=0;x<Header.Width;x++)
								{
									BYTE b=READBYTE(s);
									world[y][x].terrain_cost=15-(b>>4);
								}
							}
						}
						break;
						
						//
					case TGA_RLEColorMapped:
						{
							int x=0,y=0;
							
							int dcount=WIDTH;
							
							BYTE Packet;
							bool PacketType;
							BYTE PixelCount;
							
							while(count>0) // (s<ends) && (d<endd) )
							{
								Packet=READBYTE(s);
								PacketType=(Packet&128)==128;
								PixelCount=(BYTE)((Packet&127)+1);
								
								count-=PixelCount;
								
								if(PacketType)
								{
									// run-length
									BYTE b=READBYTE(s);
									
									while(PixelCount--)
									{
										world[y][x].terrain_cost=15-(b>>4);
										x++; if(x>=WIDTH) { x=0; y++; }
										
										//skip overdraw
										if(--dcount<=0) dcount=WIDTH;
									}
								}
								else
								{
									// raw data
									while(PixelCount--)
									{
										BYTE b=READBYTE(s);
										world[y][x].terrain_cost=15-(b>>4);
										x++; if(x>=WIDTH) { x=0; y++; }
										
										//skip overdraw
										if(--dcount<=0)	dcount=WIDTH;
									}
								}
							}
							//
							break;
						}
					}
				}
				///////////////////////////////////////////////////////////
				
				///////////////////////////////////////////////////////////
				if(!toptobottom)
				{
					switch(Header.ImageType)
					{
					case TGA_UncompressedColorMapped:
						if(Header.ImagePixelSize==8)
						{
							for(int y=Header.Height-1;y>=0;y--)
							{
								for(int x=0;x<Header.Width;x++)
								{
									BYTE b=READBYTE(s);
									world[y][x].terrain_cost=15-(b>>4);
								}
							}
						}
						break;
						
						//
					case TGA_RLEColorMapped:
						{
							int x=0,y=Header.Height-1;
							
							int dcount=WIDTH;
							
							BYTE Packet;
							bool PacketType;
							BYTE PixelCount;
							
							while(count>0) // (s<ends) && (d<endd) )
							{
								Packet=READBYTE(s);
								PacketType=(Packet&128)==128;
								PixelCount=(BYTE)((Packet&127)+1);
								
								count-=PixelCount;
								
								if(PacketType)
								{
									// run-length
									BYTE b=READBYTE(s);
									
									while(PixelCount--)
									{
										world[y][x].terrain_cost=15-(b>>4);
										x++; if(x>=WIDTH) { x=0; y--; }
										
										//skip overdraw
										if(--dcount<=0) dcount=WIDTH;
									}
								}
								else
								{
									// raw data
									while(PixelCount--)
									{
										BYTE b=READBYTE(s);
										world[y][x].terrain_cost=15-(b>>4);
										x++; if(x>=WIDTH) { x=0; y--; }
										
										//skip overdraw
										if(--dcount<=0)	dcount=WIDTH;
									}
								}
							}
							//
							break;
						}
					}
					///////////////////////////////////////////////////////////
					
				}
			}
		}

		//
		delete [] infile;
	}

	//
	finish_load();
}



//
void CSetup::Save(const char *filename)
{
	//
	const int outsize=256*3+sizeof(Header)+WIDTH*HEIGHT;
	int length=0;
	BYTE *outfile=new BYTE[outsize];
	if(outfile)
	{
		//
		BYTE *d=outfile;
		
		//
		WRITEBYTE(d,0); //idlength
		WRITEBYTE(d,1); //m->ColorMaptype
		WRITEBYTE(d,TGA_UncompressedColorMapped); //imagetype
		WRITEWORD(d,1); //m->ColorMapstart
		WRITEWORD(d,256); //m->ColorMaplength
		WRITEBYTE(d,24); //m->ColorMapdepth
		WRITEWORD(d,0); //xoffset
		WRITEWORD(d,0); //yoffset
		WRITEWORD(d,WIDTH); //width
		WRITEWORD(d,HEIGHT); //height
		WRITEBYTE(d,8); //pixeldepth
		WRITEBYTE(d,32); //imagedescriptor
		
		//
		for(int i=0;i<256;i++)
		{
			WRITEBYTE(d,(BYTE)i);
			WRITEBYTE(d,(BYTE)i);
			WRITEBYTE(d,(BYTE)i);
		}
		
		//
		for(int y=0;y<HEIGHT;y++)
		{
			for(int x=0;x<WIDTH;x++)
			{
				WRITEBYTE(d,(BYTE)((15-world[y][x].terrain_cost)<<4));
			}
		}
		
		//
		length=(d-outfile);
		const int stream=_open(
			(const char *)filename,
			_O_WRONLY | _O_CREAT | _O_TRUNC | _O_BINARY, _S_IREAD | _S_IWRITE
			);
		if(stream!=-1)
		{
			_write( stream, outfile, length);
			_close(stream);
		}
	
		//
		delete [] outfile;
	}
}




///////////////////////////////////////////////////////////////////////////

//
void CSetup::median()
{
	//
	int accum=0;
	
	for(int y=0;y<HEIGHT;y++)
	{
		for(int x=0;x<WIDTH;x++)
		{
			accum+=world[y][x].terrain_cost;
		}
	}

	//
	median_terrain_cost_auto=(float)accum/(float)(WIDTH*HEIGHT);
}

/*
int CSetup::random_range(int min, int max)
{
	return (rand()%(max-min))+min;
}
*/
//
int CSetup::get_startx()
{
	return startx;
}
int CSetup::get_starty()
{
	return starty;
}
int CSetup::get_endx()
{
	return endx;
}
int CSetup::get_endy()
{
	return endy;
}

//
void CSetup::set_start(const int y, const int x)
{
	startx=x;
	starty=y;
}
void CSetup::set_end(const int y, const int x)
{
	endx=x;
	endy=y;
}


// saves settings to registry
void CSetup::save_settings()
{
	//TODO
}

// loads last used settings from registry
void CSetup::load_settings()
{
	//TODO
}


//
void CSetup::get_colorscheme_colors(COLORREF &background, COLORREF &foreground)
{
	switch(colorscheme)
	{
	case COLORSCHEME_WHITE:	background=0x00ffffff; foreground=0x00000000; break;
	case COLORSCHEME_BLACK:	background=0x00000000; foreground=0x00ffffff; break;
	case COLORSCHEME_GRAY:	background=0x00C8D0D4; foreground=0x00000000; break;
	case COLORSCHEME_PAPER:	background=0x00c0ffff; foreground=0x00303030; break;
	default: background=0x00808080; foreground=0x00000000; break;
	}
}






//////////////////////////////////////////////////////////////////////
// presearch
//////////////////////////////////////////////////////////////////////

//
void CSetup::Presearch()
{
	//
	Presearch_Impassables();

	//
	int group=MIN_GROUP;
	int y;
	int x;
	Presearch_FindUngrouped(y,x);
	while(y!=-1 && x!=-1)
	{
		nodes[1].y=y;
		nodes[1].x=x;
		world[y][x].group=group;
        Presearch_FloodFill(group);
		Presearch_FindUngrouped(y,x);
		if(++group>=MAX_GROUP) break; //hmm...
	}

	presearch_maxgroup=group;
}

//
void CSetup::Presearch_FindUngrouped(int& y, int& x)
{
	y=-1;
	x=-1;

	for(int ly=0;ly<HEIGHT;ly++)
	{
		for(int lx=0;lx<WIDTH;lx++)
		{
			if(world[ly][lx].group==NO_GROUP)
			{
				y=ly;
				x=lx;
				return;
			}
		}
	}
}

//
void CSetup::Presearch_Impassables()
{
	for(int y=0;y<HEIGHT;y++)
	{
		for(int x=0;x<WIDTH;x++)
		{
			world[y][x].group=(BYTE)(world[y][x].terrain_cost==IMPASSABLE_TERRAIN_COST)?IMPASSABLE_GROUP:NO_GROUP;
		}
	}
}

//
void CSetup::Presearch_FloodFill(const int group)
{
	int last_node=1;
	do
	{
		int parenty=nodes[last_node].y;
		int parentx=nodes[last_node].x;
		last_node--;
		
		for(int d=0;d<8;d++)
		{
			int y=parenty+DXY[d].y;
			int x=parentx+DXY[d].x;
			
			if(world[y][x].group==NO_GROUP)
			{
				world[y][x].group=group;
				last_node++;
				nodes[last_node].y=y;
				nodes[last_node].x=x;
			}
		}
	} while(last_node>0 && last_node<MAX_PIXELS);
}
