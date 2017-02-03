// Setup.h: interface for the CSetup class.
//
//////////////////////////////////////////////////////////////////////


#if !defined(H_CSETUP)
#define H_CSETUP

#if _MSC_VER > 1000
#pragma once
#endif // _MSC_VER > 1000

//
#ifndef _WINDOWS_
typedef char CHAR;

typedef unsigned char BYTE;
typedef unsigned short WORD;
typedef unsigned int DWORD;
typedef char boolean;
#endif

enum
{
	WIDTH=256, //64 256
		HEIGHT=256, //64 256
		YSHIFT=8, //6 8
		XMASK=255, //63 255
		
		MAX_PIXELS=WIDTH*HEIGHT,
		EMPTY_NODE=0,
		IMPASSABLE_TERRAIN_COST=15
};
enum
{
	IMPASSABLE_GROUP=0,
		MIN_GROUP=1,
		MAX_GROUP=254, //63,
		NO_GROUP=255
};

enum
{
	MANHATTAN_DISTANCE=0,
		DIAGONAL_DISTANCE=1,
		PYTHAGORAS_DISTANCE=2,
		SIMPLE_PYTHAGORAS_DISTANCE=3
};


///////////////////////////////////////////////////////////////////////////

// for Paint functions of all path-finding classes
struct _RGBA
{
	BYTE blue;
	BYTE green;
	BYTE red;
	BYTE alpha;
};
enum
{
	COLORSCHEME_WHITE,
		COLORSCHEME_BLACK,
		COLORSCHEME_GRAY,
		COLORSCHEME_PAPER
};


///////////////////////////////////////////////////////////////////////////
//#define DISPLAY_AIS
enum
{
	MAX_AIS=500,
		//MAX_ROUTES_BYTES=384,
		MAX_ROUTES=(WIDTH+HEIGHT)*4 //1024,
		//MAX_RUNLENGTH=32
};
/*typedef struct _ROUTE
{
BYTE route		:3; //0-7
BYTE runlength	:5; //0-31 +1
} ROUTE;*/
typedef struct _AIROUTE
{
	// debugging
	DWORD color;
	// processing
	BYTE priority; //0=done
	BYTE active			:1; //0=no ai, 1=ai
	BYTE compression	:1;
	// walking
	WORD walk_point; //at what point in the path-ways are we?
	WORD walk_runlength_step; //and if on a runlength at what step?
	// encoding/decoding
	WORD startyx;
	WORD endyx;
	WORD start;  //0 to MAX_PATH_LENGTH-1
	WORD count; //if count==0 then not used
	//ROUTE route[MAX_ROUTES];
	BYTE route[MAX_ROUTES]; //MAX_ROUTES_BYTES];
} AIROUTE;


///////////////////////////////////////////////////////////////////////////

//
class CSetup  
{
public:
	CSetup();
	virtual ~CSetup();

	void Load(const char* filename);
	void Save(const char* filename);
	void median();
	
	int get_startx();
	int get_starty();
	int get_endx();
	int get_endy();
	void set_start(const int y, const int x);
	void set_end(const int y, const int x);
	
	void save_settings();
	void load_settings();
	
	void get_colorscheme_colors(COLORREF &background, COLORREF &foreground);
	
	void blank(BYTE terrain_cost);
	void blocking_walls();
	void rectangle(int x0, int y0, int x1, int y1, BYTE terrain_cost);
	void openrectangle(int x0, int y0, int x1, int y1, BYTE terrain_cost);
	void finish_load();
	
	void Map_BoxOnBoxNoG();
	void Map_BoxOnBox();
	void Map_CheckerBoard();
	void Map_Grid();
	void Map_PipeMaze();
	void Map_Random();
	void Map_No_Path();
	void Map_Clear_Path();
	void Map_StraightLine();
	void Map_CrashMe();
	void Map_BigBox();
	void Map_RandomBoxes();
	void Map_RandomOpenBoxes();
	void Map_RandomTerrain();
	void Map_Gates();
	void Map_ZigZag();

public:
	LARGE_INTEGER bigtick; //
	
	int colorscheme;// for Paint functions of all path-finding classes
	
	bool okToPath;

	bool use_terrain;
	int iterations_per_frame;
	int frame;
	float cost;
	float diagonal_cost;
	float median_terrain_cost;
	float median_terrain_cost_auto;
	WORD distance_method;
	BYTE directions;
	
	struct _OPTIONS
	{
		bool uniform_cost;
		bool terrain_cost;
		bool distance;
		bool search_directions;
	} options;
	
	struct _WORLD
	{
		BYTE terrain_cost; //:4;
		BYTE group;
	} world[WIDTH][HEIGHT];
	
	//
	//private:
	BYTE startx;
	BYTE starty;
	BYTE endx;
	BYTE endy;
	
	private:	
		inline BYTE READBYTE(BYTE*& ptr)
		{
			ptr++;
			return* (ptr-1);
		}
		inline WORD READWORD(BYTE*& ptr)
		{
			WORD w=*(WORD *)ptr;
			ptr+=sizeof(WORD);
			return w;
		}
		inline DWORD READDWORD(BYTE*& ptr)
		{
			DWORD dw=*(DWORD *)ptr;
			ptr+=sizeof(DWORD);
			return dw;
		}
		inline void WRITEBYTE(BYTE*& ptr, BYTE b)
		{
			*ptr++=b;
		}
		inline void WRITEWORD(BYTE*& ptr, WORD w)
		{
			*(WORD *)ptr=w;
			ptr+=sizeof(WORD);
		}
		inline void WRITEDWORD(BYTE*& ptr, DWORD dw)
		{
			*(DWORD *)ptr=dw;
			ptr+=sizeof(DWORD);
		}
		
	private:
		enum _tgatypes
		{
			//image types
			TGA_Null=0, //No image data included.
				TGA_UncompressedColorMapped=1, //Uncompressed, color-mapped images.
				TGA_UncompressedRGB=2, //Uncompressed, RGB images.
				TGA_UncompressedMonochrome=3, //Uncompressed, black and white images.
				TGA_RLEColorMapped=9, //Runlength encoded color-mapped images.
				TGA_RLERGB=10, //Runlength encoded RGB images.
				TGA_RLEMonochrome=11, //Compressed, black and white images.
				TGA_HuffmanColorMapped=32, //Compressed color-mapped data, using Huffman, Delta, and runlength encoding.
				TGA_HuffmanColorMapped4=33, //Compressed color-mapped data, using Huffman, Delta, and runlength encoding.  4-pass quadtree-type process.
				//interleave flag
				//TGA_IL_None=0,
				//TGA_IL_Two=1,
				//TGA_IL_Four=2
		};
		
		struct _header
		{
			BYTE IDLength;
			BYTE ColorMapType; //0=no palette, 1=palette  
			BYTE ImageType; //0=none, 1-3=uncompressed, 9-11= compressed
			WORD ColorMapOrigin;
			WORD ColorMapLength;
			BYTE ColorMapEntrySize; //8 15 16 24 32
			WORD XOrigin;
			WORD YOrigin;
			WORD Width;
			WORD Height;
			BYTE ImagePixelSize; //8 16 24 32
			BYTE ImageDescriptorByte; // bits 0-3= Attribute bpp, bits 4-5 show origin loc
		} Header;
		BYTE CIF[256+1];
		
		//
		struct _colormap
		{
			BYTE red;
			BYTE green;
			BYTE blue;
			BYTE alpha;
		} ColorMap[256];
		
		
		// presearch
		public:
			bool presearch_toggle;
			int presearch_maxgroup;
			void Presearch();
		private:
			void Presearch_FindUngrouped(int &y, int& x);
			void Presearch_Impassables();
			void Presearch_FloodFill(const int group);
			
		private:
			struct _D_TO_XY_LOOKUP
			{
				short y;
				short x;
			} DXY[8+1];
			
			struct _NODES
			{
				short y;
				short x;
			}  nodes[WIDTH*HEIGHT];
};

#endif
