#include "dxgl.h"
char buffer[80];                             // general printing buffer

int gwidth  = -1;
int gheight = -1;
int min_clip_y=0;
int min_clip_x=0;
int max_clip_y=SCREEN_HEIGHT;
int max_clip_x=SCREEN_WIDTH;


extern LPDIRECTDRAW7         lpdd         ;   // dd4 object
/*
extern LPDIRECTDRAWSURFACE7  lpddsprimary ;   // dd primary surface
extern LPDIRECTDRAWSURFACE7  lpddsback    ;   // dd back surface
extern LPDIRECTDRAWPALETTE   lpddpal      ;   // a pointer to the created dd palette
extern LPDIRECTDRAWCLIPPER   lpddclipper  ;   // dd clipper
*/
int Unload_Bitmap_File(BITMAP_FILE_PTR bitmap)
{
// this function releases all memory associated with "bitmap"
	if (bitmap->buffer)
	{
		// release memoryread
		free(bitmap->buffer);

		// reset pointer
		bitmap->buffer = NULL;
	}
	return(1);
}

int Load_Bitmap_File(BITMAP_FILE_PTR bitmap, char *filename)
{
// this function opens a bitmap file and loads the data into bitmap

int file_handle,  // the file handle
    index;        // looping index

UCHAR   *temp_buffer = NULL; // used to convert 24 bit images to 16 bit
OFSTRUCT file_data;          // the file data information

// open the file if it exists
if ((file_handle = OpenFile(filename,&file_data,OF_READ))==-1)
   return(0);

// now load the bitmap file header
_lread(file_handle, &bitmap->bitmapfileheader,sizeof(BITMAPFILEHEADER));

// test if this is a bitmap file
if (bitmap->bitmapfileheader.bfType!=BITMAP_ID)
   {
   // close the file
   _lclose(file_handle);

   // return error
   return(0);
   } // end if

// now we know this is a bitmap, so read in all the sections

// first the bitmap infoheader

// now load the bitmap file header
_lread(file_handle, &bitmap->bitmapinfoheader,sizeof(BITMAPINFOHEADER));

// now load the color palette if there is one
if (bitmap->bitmapinfoheader.biBitCount == 8)
   {
   _lread(file_handle, &bitmap->palette,MAX_COLORS_PALETTE*sizeof(PALETTEENTRY));

   // now set all the flags in the palette correctly and fix the reversed 
   // BGR RGBQUAD data format
   for (index=0; index < MAX_COLORS_PALETTE; index++)
       {
       // reverse the red and green fields
       int temp_color                = bitmap->palette[index].peRed;
       bitmap->palette[index].peRed  = bitmap->palette[index].peBlue;
       bitmap->palette[index].peBlue = temp_color;
       
       // always set the flags word to this
       bitmap->palette[index].peFlags = PC_NOCOLLAPSE;
       } // end for index

    } // end if

// finally the image data itself
_lseek(file_handle,-(int)(bitmap->bitmapinfoheader.biSizeImage),SEEK_END);

// now read in the image

if (bitmap->bitmapinfoheader.biBitCount==8 || bitmap->bitmapinfoheader.biBitCount==16 || 
    bitmap->bitmapinfoheader.biBitCount==24)
   {
   // delete the last image if there was one
   if (bitmap->buffer)
       free(bitmap->buffer);

   // allocate the memory for the image
   if (!(bitmap->buffer = (UCHAR *)malloc(bitmap->bitmapinfoheader.biSizeImage)))
      {
      // close the file
      _lclose(file_handle);

      // return error
      return(0);
      } // end if

   // now read it in
   _lread(file_handle,bitmap->buffer,bitmap->bitmapinfoheader.biSizeImage);

   } // end if
else
   {
   // serious problem
   return(0);

   } // end else

#if 0
// write the file info out 
printf("\nfilename:%s \nsize=%d \nwidth=%d \nheight=%d \nbitsperpixel=%d \ncolors=%d \nimpcolors=%d",
        filename,
        bitmap->bitmapinfoheader.biSizeImage,
        bitmap->bitmapinfoheader.biWidth,
        bitmap->bitmapinfoheader.biHeight,
		bitmap->bitmapinfoheader.biBitCount,
        bitmap->bitmapinfoheader.biClrUsed,
        bitmap->bitmapinfoheader.biClrImportant);
#endif

// close the file
_lclose(file_handle);

// flip the bitmap
Flip_Bitmap(bitmap->buffer, 
            bitmap->bitmapinfoheader.biWidth*(bitmap->bitmapinfoheader.biBitCount/8), 
            bitmap->bitmapinfoheader.biHeight);

// return success
return(1);

} // end Load_Bitmap_File



int Flip_Bitmap(UCHAR *image, int bytes_per_line, int height)
{
// this function is used to flip bottom-up .BMP images

	UCHAR *buffer; // used to perform the image processing
	int index;     // looping index

	// allocate the temporary buffer
	if (!(buffer = (UCHAR *)malloc(bytes_per_line*height)))
	   return(0);

	// copy image to work area
	memcpy(buffer,image,bytes_per_line*height);

	// flip vertically
	for (index=0; index < height; index++)
		memcpy(&image[((height-1) - index)*bytes_per_line],
			   &buffer[index*bytes_per_line], bytes_per_line);

	// release the memory
	free(buffer);

	// return success
	return(1);
}

LPDIRECTDRAWCLIPPER DDraw_Attach_Clipper(LPDIRECTDRAWSURFACE7 lpdds,
                                         int num_rects,
                                         LPRECT clip_list)

{
	// this function creates a clipper from the sent clip list and attaches
	// it to the sent surface

	int index;                         // looping var
	LPDIRECTDRAWCLIPPER lpddclipper;   // pointer to the newly created dd clipper
	LPRGNDATA region_data;             // pointer to the region data that contains
									   // the header and clip list

	// first create the direct draw clipper
	if (FAILED(lpdd->CreateClipper(0,&lpddclipper,NULL)))
	   return(NULL);

	// now create the clip list from the sent data

	// first allocate memory for region data
	region_data = (LPRGNDATA)malloc(sizeof(RGNDATAHEADER)+num_rects*sizeof(RECT));

	// now copy the rects into region data
	memcpy(region_data->Buffer, clip_list, sizeof(RECT)*num_rects);

	// set up fields of header
	region_data->rdh.dwSize          = sizeof(RGNDATAHEADER);
	region_data->rdh.iType           = RDH_RECTANGLES;
	region_data->rdh.nCount          = num_rects;
	region_data->rdh.nRgnSize        = num_rects*sizeof(RECT);

	region_data->rdh.rcBound.left    =  64000;
	region_data->rdh.rcBound.top     =  64000;
	region_data->rdh.rcBound.right   = -64000;
	region_data->rdh.rcBound.bottom  = -64000;

	// find bounds of all clipping regions
	for (index=0; index<num_rects; index++)
	{
		// test if the next rectangle unioned with the current bound is larger
		if (clip_list[index].left < region_data->rdh.rcBound.left)
		   region_data->rdh.rcBound.left = clip_list[index].left;

		if (clip_list[index].right > region_data->rdh.rcBound.right)
		   region_data->rdh.rcBound.right = clip_list[index].right;

		if (clip_list[index].top < region_data->rdh.rcBound.top)
		   region_data->rdh.rcBound.top = clip_list[index].top;

		if (clip_list[index].bottom > region_data->rdh.rcBound.bottom)
		   region_data->rdh.rcBound.bottom = clip_list[index].bottom;

	} 

	// now we have computed the bounding rectangle region and set up the data
	// now let's set the clipping list

	if (FAILED(lpddclipper->SetClipList(region_data, 0)))
	{
	   // release memory and return error
	   free(region_data);
	   return(NULL);
	}

	// now attach the clipper to the surface
	if (FAILED(lpdds->SetClipper(lpddclipper)))
	{
	   // release memory and return error
	   free(region_data);
	   return(NULL);
	}

	// all is well, so release memory and send back the pointer to the new clipper
	free(region_data);
	return(lpddclipper);
}


int DDraw_Fill_Surface(LPDIRECTDRAWSURFACE7 lpdds,int color)
{
	DDBLTFX ddbltfx; // this contains the DDBLTFX structure

	// clear out the structure and set the size field 
	DDRAW_INIT_STRUCT(ddbltfx);

	// set the dwfillcolor field to the desired color
	ddbltfx.dwFillColor = color; 

	// ready to blt to surface
	lpdds->Blt(NULL,       // ptr to dest rectangle
			   NULL,       // ptr to source surface, NA            
			   NULL,       // ptr to source rectangle, NA
			   DDBLT_COLORFILL | DDBLT_WAIT,   // fill and wait                   
			   &ddbltfx);  // ptr to DDBLTFX structure

	// return success
	return(1);
} // end DDraw_Fill_Surface

///////////////////////////////////////////////////////////////

int DDraw_Draw_Surface(LPDIRECTDRAWSURFACE7 source, // source surface to draw
                      int x, int y,                 // position to draw at
                      int width, int height,        // size of source surface
                      LPDIRECTDRAWSURFACE7 dest,    // surface to draw the surface on
                      int transparent = 1)          // transparency flag
{
	// draw a bob at the x,y defined in the BOB
	// on the destination surface defined in dest

	RECT dest_rect,   // the destination rectangle
		 source_rect; // the source rectangle                             

	// fill in the destination rect
	dest_rect.left   = x;
	dest_rect.top    = y;
	dest_rect.right  = x+width;
	dest_rect.bottom = y+height;

	// fill in the source rect
	source_rect.left    = 0;
	source_rect.top     = 0;
	source_rect.right   = width;
	source_rect.bottom  = height;

	// test transparency flag

	if (transparent)
	{
	   // enable color key blit
	   // blt to destination surface
	   if (FAILED(dest->Blt(&dest_rect, source,
						 &source_rect,(DDBLT_WAIT | DDBLT_KEYSRC),
						 NULL)))
			   return(0);

	}
	else
	{
	// perform blit without color key
	// blt to destination surface
	if (FAILED(dest->Blt(&dest_rect, source,
                     &source_rect,(DDBLT_WAIT),
                     NULL)))
           return(0);

	} 

	return(1);
}

int DDraw_DrawSized_Surface(LPDIRECTDRAWSURFACE7 source, // source surface to draw
                      int x, int y,                 // position to draw at
                      int width, int height,        // size of source surface
					  int destWidth,int destHeight,
                      LPDIRECTDRAWSURFACE7 dest,    // surface to draw the surface on
                      int transparent = 1)          // transparency flag
{
	// draw a bob at the x,y defined in the BOB
	// on the destination surface defined in dest

	RECT dest_rect,   // the destination rectangle
		 source_rect; // the source rectangle                             

	// fill in the destination rect
	dest_rect.left   = x;
	dest_rect.top    = y;
	dest_rect.right  = x+destWidth;
	dest_rect.bottom = y+destHeight;

	// fill in the source rect
	source_rect.left    = 0;
	source_rect.top     = 0;
	source_rect.right   = width;
	source_rect.bottom  = height;

	// test transparency flag

	if (transparent)
	{
	   // enable color key blit
	   // blt to destination surface
	   if (FAILED(dest->Blt(&dest_rect, source,
						 &source_rect,(DDBLT_WAIT | DDBLT_KEYSRC),
						 NULL)))
			   return(0);

	}
	else
	{
	// perform blit without color key
	// blt to destination surface
	if (FAILED(dest->Blt(&dest_rect, source,
                     &source_rect,(DDBLT_WAIT),
                     NULL)))
           return(0);

	} 

	return(1);
}

int NDDraw_Draw_Surface(LPDIRECTDRAWSURFACE7 source, // source surface to draw
                      int x, int y,                 // position to take data from
                      int width, int height,        // size of source surface
                      LPDIRECTDRAWSURFACE7 dest,    // surface to draw the surface on
                      int transparent = 1)          // transparency flag
{
	// draw a bob at the x,y defined in the BOB
	// on the destination surface defined in dest

	RECT dest_rect,   // the destination rectangle
		 source_rect; // the source rectangle                             

	// fill in the destination rect
	dest_rect.left   = 0;
	dest_rect.top    = 0;
	dest_rect.right  = width;
	dest_rect.bottom = height;

	// fill in the source rect
	source_rect.left    = x;
	source_rect.top     = y;
	source_rect.right   = x+width;
	source_rect.bottom  = y+height;

	// test transparency flag

	if (transparent)
	{
	   // enable color key blit
	   // blt to destination surface
	   if (FAILED(dest->Blt(&dest_rect, source,
						 &source_rect,(DDBLT_WAIT | DDBLT_KEYSRC),
						 NULL)))
			   return(0);

	}
	else
	{
	// perform blit without color key
	// blt to destination surface
	if (FAILED(dest->Blt(&dest_rect, source,
                     &source_rect,(DDBLT_WAIT),
                     NULL)))
           return(0);

	} 

	return(1);
}

int Scan_Image_Bitmap(BITMAP_FILE_PTR bitmap,     // bitmap file to scan image data from
                      LPDIRECTDRAWSURFACE7 lpdds, // surface to hold data
                      int cx,int cy)              // cell to scan image from
{
// this function extracts a bitmap out of a bitmap file

UCHAR *source_ptr,   // working pointers
      *dest_ptr;

DDSURFACEDESC2 ddsd;  //  direct draw surface description 

// get the addr to destination surface memory

// set size of the structure
memset(&ddsd,0,sizeof(ddsd));
ddsd.dwSize = sizeof(ddsd);

// lock the display surface
lpdds->Lock(NULL,
            &ddsd,
            DDLOCK_WAIT | DDLOCK_SURFACEMEMORYPTR,
            NULL);

// compute position to start scanning bits from
cx = cx*(ddsd.dwWidth+1) + 1;
cy = cy*(ddsd.dwHeight+1) + 1;
  
// extract bitmap data
source_ptr = bitmap->buffer + cy*bitmap->bitmapinfoheader.biWidth+cx;

// assign a pointer to the memory surface for manipulation
dest_ptr = (UCHAR *)ddsd.lpSurface;

// iterate thru each scanline and copy bitmap
for (unsigned int index_y=0; index_y < ddsd.dwHeight; index_y++)
    {
    // copy next line of data to destination
    memcpy(dest_ptr, source_ptr, ddsd.dwWidth);

    // advance pointers
    dest_ptr   += (ddsd.dwWidth);
    source_ptr += bitmap->bitmapinfoheader.biWidth;
    } // end for index_y

// unlock the surface 
lpdds->Unlock(NULL);

// return success
return(1);

} // end Scan_Image_Bitmap

int Scan_Image_Bitmap16(BITMAP_FILE_PTR bitmap,     // bitmap file to scan image data from
                      LPDIRECTDRAWSURFACE7 lpdds, // surface to hold data
                      int cx,int cy,int sizex,int sizey)              // cell to scan image from
{

USHORT *source_ptr,   // working pointers
       *dest_ptr;

DDSURFACEDESC2 ddsd;  //  direct draw surface description 

// get the addr to destination surface memory

// set size of the structure
memset(&ddsd,0,sizeof(ddsd));
ddsd.dwSize = sizeof(ddsd);

// lock the display surface
lpdds->Lock(NULL,
            &ddsd,
            DDLOCK_WAIT | DDLOCK_SURFACEMEMORYPTR,
            NULL);

// copy the bitmap image to the primary buffer line by line
// notice the 24 to 16 bit conversion pixel by pixel

// get video pointer to primary surfce
dest_ptr= (USHORT *)ddsd.lpSurface;       

// process each line and copy it into the given buffer
for (unsigned int index_y = 0; index_y < sizey; index_y++)
    {
    for (unsigned int index_x = 0; index_x < sizex; index_x++)
        {
        // get BGR values, note the scaling down of the channels, so that they
        // fit into the 5.6.5 format
        UCHAR blue  = (bitmap->buffer[index_y*sizex*3 + index_x*3 + 0]) >> 3,
              green = (bitmap->buffer[index_y*sizex*3 + index_x*3 + 1]) >> 3,
              red   = (bitmap->buffer[index_y*sizex*3 + index_x*3 + 2]) >> 3;

        // this builds a 16 bit color value in 5.6.5 format (green dominant mode)
        USHORT pixel = _RGB16BIT565(red,green,blue);

        // write the pixel
		if(sizex==640)
		{
			dest_ptr[index_x + (index_y*ddsd.lPitch>>1)] = pixel;
		}
		else
			dest_ptr[index_x + (index_y*ddsd.lPitch>>1)] = pixel;
        } // end for index_x

    } // end for index_y

// now unlock the primary surface
if (FAILED(lpdds->Unlock(NULL)))
   return(0);

return(1);

} // end Scan_Image_Bitmap

///////////////////////////////////////////////////////////////

LPDIRECTDRAWSURFACE7 DDraw_Create_Surface(int width, int height, int mem_flags, int color_key = 0)
{
// this function creates an offscreen plain surface

	DDSURFACEDESC2 ddsd;         // working description
	LPDIRECTDRAWSURFACE7 lpdds;  // temporary surface
    //LPDIRECTDRAW4 lpdd;

	// set to access caps, width, and height
	memset(&ddsd,0,sizeof(ddsd));
	ddsd.dwSize  = sizeof(ddsd);
	ddsd.dwFlags = DDSD_CAPS | DDSD_WIDTH | DDSD_HEIGHT;

	// set dimensions of the new bitmap surface
	ddsd.dwWidth  =  width;
	ddsd.dwHeight =  height;

	// set surface to offscreen plain
	ddsd.ddsCaps.dwCaps = DDSCAPS_OFFSCREENPLAIN | mem_flags;

	// create the surface
	if (FAILED(lpdd->CreateSurface(&ddsd,&lpdds,NULL)))
	   return(NULL);


	if (color_key >= 0)
    {

	   DDCOLORKEY color_key; 
	   color_key.dwColorSpaceLowValue  = 0;
	   color_key.dwColorSpaceHighValue = 0;

	   lpdds->SetColorKey(DDCKEY_SRCBLT, &color_key);
    } 

	return(lpdds);
}

int Draw_Clip_Line(int x0,int y0, int x1, int y1,UCHAR color, 
                    UCHAR *dest_buffer, int lpitch)
{
// this function draws a wireframe triangle

int cxs, cys,
	cxe, cye;

// clip and draw each line
cxs = x0;
cys = y0;
cxe = x1;
cye = y1;

// clip the line
if (Clip_Line(cxs,cys,cxe,cye))
	Draw_Line(cxs, cys, cxe,cye,color,dest_buffer,lpitch);

// return success
return(1);

} // end Draw_Clip_Line

///////////////////////////////////////////////////////////

int Clip_Line(int &x1,int &y1,int &x2, int &y2)
{
// this function clips the sent line using the globally defined clipping
// region

// internal clipping codes
#define CLIP_CODE_C  0x0000
#define CLIP_CODE_N  0x0008
#define CLIP_CODE_S  0x0004
#define CLIP_CODE_E  0x0002
#define CLIP_CODE_W  0x0001

#define CLIP_CODE_NE 0x000a
#define CLIP_CODE_SE 0x0006
#define CLIP_CODE_NW 0x0009 
#define CLIP_CODE_SW 0x0005

int xc1=x1, 
    yc1=y1, 
	xc2=x2, 
	yc2=y2;

int p1_code=0, 
    p2_code=0;

// determine codes for p1 and p2
if (y1 < min_clip_y)
	p1_code|=CLIP_CODE_N;
else
if (y1 > max_clip_y)
	p1_code|=CLIP_CODE_S;

if (x1 < min_clip_x)
	p1_code|=CLIP_CODE_W;
else
if (x1 > max_clip_x)
	p1_code|=CLIP_CODE_E;

if (y2 < min_clip_y)
	p2_code|=CLIP_CODE_N;
else
if (y2 > max_clip_y)
	p2_code|=CLIP_CODE_S;

if (x2 < min_clip_x)
	p2_code|=CLIP_CODE_W;
else
if (x2 > max_clip_x)
	p2_code|=CLIP_CODE_E;

// try and trivially reject
if ((p1_code & p2_code)) 
	return(0);

// test for totally visible, if so leave points untouched
if (p1_code==0 && p2_code==0)
	return(1);

// determine end clip point for p1
switch(p1_code)
	  {
	  case CLIP_CODE_C: break;

	  case CLIP_CODE_N:
		   {
		   yc1 = min_clip_y;
		   xc1 = x1 + 0.5+(min_clip_y-y1)*(x2-x1)/(y2-y1);
		   } break;
	  case CLIP_CODE_S:
		   {
		   yc1 = max_clip_y;
		   xc1 = x1 + 0.5+(max_clip_y-y1)*(x2-x1)/(y2-y1);
		   } break;

	  case CLIP_CODE_W:
		   {
		   xc1 = min_clip_x;
		   yc1 = y1 + 0.5+(min_clip_x-x1)*(y2-y1)/(x2-x1);
		   } break;
		
	  case CLIP_CODE_E:
		   {
		   xc1 = max_clip_x;
		   yc1 = y1 + 0.5+(max_clip_x-x1)*(y2-y1)/(x2-x1);
		   } break;

	// these cases are more complex, must compute 2 intersections
	  case CLIP_CODE_NE:
		   {
		   // north hline intersection
		   yc1 = min_clip_y;
		   xc1 = x1 + 0.5+(min_clip_y-y1)*(x2-x1)/(y2-y1);

		   // test if intersection is valid, of so then done, else compute next
			if (xc1 < min_clip_x || xc1 > max_clip_x)
				{
				// east vline intersection
				xc1 = max_clip_x;
				yc1 = y1 + 0.5+(max_clip_x-x1)*(y2-y1)/(x2-x1);
				} // end if

		   } break;
	  
	  case CLIP_CODE_SE:
      	   {
		   // south hline intersection
		   yc1 = max_clip_y;
		   xc1 = x1 + 0.5+(max_clip_y-y1)*(x2-x1)/(y2-y1);	

		   // test if intersection is valid, of so then done, else compute next
		   if (xc1 < min_clip_x || xc1 > max_clip_x)
		      {
			  // east vline intersection
			  xc1 = max_clip_x;
			  yc1 = y1 + 0.5+(max_clip_x-x1)*(y2-y1)/(x2-x1);
			  } // end if

		   } break;
	    
	  case CLIP_CODE_NW: 
      	   {
		   // north hline intersection
		   yc1 = min_clip_y;
		   xc1 = x1 + 0.5+(min_clip_y-y1)*(x2-x1)/(y2-y1);
		   
		   // test if intersection is valid, of so then done, else compute next
		   if (xc1 < min_clip_x || xc1 > max_clip_x)
		      {
			  xc1 = min_clip_x;
		      yc1 = y1 + 0.5+(min_clip_x-x1)*(y2-y1)/(x2-x1);	
			  } // end if

		   } break;
	  	  
	  case CLIP_CODE_SW:
		   {
		   // south hline intersection
		   yc1 = max_clip_y;
		   xc1 = x1 + 0.5+(max_clip_y-y1)*(x2-x1)/(y2-y1);	
		   
		   // test if intersection is valid, of so then done, else compute next
		   if (xc1 < min_clip_x || xc1 > max_clip_x)
		      {
			  xc1 = min_clip_x;
		      yc1 = y1 + 0.5+(min_clip_x-x1)*(y2-y1)/(x2-x1);	
			  } // end if

		   } break;

	  default:break;

	  } // end switch

// determine clip point for p2
switch(p2_code)
	  {
	  case CLIP_CODE_C: break;

	  case CLIP_CODE_N:
		   {
		   yc2 = min_clip_y;
		   xc2 = x2 + (min_clip_y-y2)*(x1-x2)/(y1-y2);
		   } break;

	  case CLIP_CODE_S:
		   {
		   yc2 = max_clip_y;
		   xc2 = x2 + (max_clip_y-y2)*(x1-x2)/(y1-y2);
		   } break;

	  case CLIP_CODE_W:
		   {
		   xc2 = min_clip_x;
		   yc2 = y2 + (min_clip_x-x2)*(y1-y2)/(x1-x2);
		   } break;
		
	  case CLIP_CODE_E:
		   {
		   xc2 = max_clip_x;
		   yc2 = y2 + (max_clip_x-x2)*(y1-y2)/(x1-x2);
		   } break;

		// these cases are more complex, must compute 2 intersections
	  case CLIP_CODE_NE:
		   {
		   // north hline intersection
		   yc2 = min_clip_y;
		   xc2 = x2 + 0.5+(min_clip_y-y2)*(x1-x2)/(y1-y2);

		   // test if intersection is valid, of so then done, else compute next
			if (xc2 < min_clip_x || xc2 > max_clip_x)
				{
				// east vline intersection
				xc2 = max_clip_x;
				yc2 = y2 + 0.5+(max_clip_x-x2)*(y1-y2)/(x1-x2);
				} // end if

		   } break;
	  
	  case CLIP_CODE_SE:
      	   {
		   // south hline intersection
		   yc2 = max_clip_y;
		   xc2 = x2 + 0.5+(max_clip_y-y2)*(x1-x2)/(y1-y2);	

		   // test if intersection is valid, of so then done, else compute next
		   if (xc2 < min_clip_x || xc2 > max_clip_x)
		      {
			  // east vline intersection
			  xc2 = max_clip_x;
			  yc2 = y2 + 0.5+(max_clip_x-x2)*(y1-y2)/(x1-x2);
			  } // end if

		   } break;
	    
	  case CLIP_CODE_NW: 
      	   {
		   // north hline intersection
		   yc2 = min_clip_y;
		   xc2 = x2 + 0.5+(min_clip_y-y2)*(x1-x2)/(y1-y2);
		   
		   // test if intersection is valid, of so then done, else compute next
		   if (xc2 < min_clip_x || xc2 > max_clip_x)
		      {
			  xc2 = min_clip_x;
		      yc2 = y2 + 0.5+(min_clip_x-x2)*(y1-y2)/(x1-x2);	
			  } // end if

		   } break;
	  	  
	  case CLIP_CODE_SW:
		   {
		   // south hline intersection
		   yc2 = max_clip_y;
		   xc2 = x2 + 0.5+(max_clip_y-y2)*(x1-x2)/(y1-y2);	
		   
		   // test if intersection is valid, of so then done, else compute next
		   if (xc2 < min_clip_x || xc2 > max_clip_x)
		      {
			  xc2 = min_clip_x;
		      yc2 = y2 + 0.5+(min_clip_x-x2)*(y1-y2)/(x1-x2);	
			  } // end if

		   } break;
	
	  default:break;

	  } // end switch

// do bounds check
if ((xc1 < min_clip_x) || (xc1 > max_clip_x) ||
	(yc1 < min_clip_y) || (yc1 > max_clip_y) ||
	(xc2 < min_clip_x) || (xc2 > max_clip_x) ||
	(yc2 < min_clip_y) || (yc2 > max_clip_y) )
	{
	return(0);
	} // end if

// store vars back
x1 = xc1;
y1 = yc1;
x2 = xc2;
y2 = yc2;

return(1);

} // end Clip_Line

///////////////////////////////////////////////////////////

int Draw_Line(int x0, int y0, // starting position 
              int x1, int y1, // ending position
              UCHAR color,    // color index
              UCHAR *vb_start, int lpitch) // video buffer and memory pitch
{
// this function draws a line from xo,yo to x1,y1 using differential error
// terms (based on Bresenahams work)

int dx,             // difference in x's
    dy,             // difference in y's
    dx2,            // dx,dy * 2
    dy2, 
    x_inc,          // amount in pixel space to move during drawing
    y_inc,          // amount in pixel space to move during drawing
    error,          // the discriminant i.e. error i.e. decision variable
    index;          // used for looping

// pre-compute first pixel address in video buffer
vb_start = vb_start + x0 + y0*lpitch;

// compute horizontal and vertical deltas
dx = x1-x0;
dy = y1-y0;

// test which direction the line is going in i.e. slope angle
if (dx>=0)
   {
   x_inc = 1;

   } // end if line is moving right
else
   {
   x_inc = -1;
   dx    = -dx;  // need absolute value

   } // end else moving left

// test y component of slope

if (dy>=0)
   {
   y_inc = lpitch;
   } // end if line is moving down
else
   {
   y_inc = -lpitch;
   dy    = -dy;  // need absolute value

   } // end else moving up

// compute (dx,dy) * 2
dx2 = dx << 1;
dy2 = dy << 1;

// now based on which delta is greater we can draw the line
if (dx > dy)
   {
   // initialize error term
   error = dy2 - dx; 

   // draw the line
   for (index=0; index <= dx; index++)
       {
       // set the pixel
       *vb_start = color;

       // test if error has overflowed
       if (error >= 0) 
          {
          error-=dx2;

          // move to next line
          vb_start+=y_inc;

	   } // end if error overflowed

       // adjust the error term
       error+=dy2;

       // move to the next pixel
       vb_start+=x_inc;

       } // end for

   } // end if |slope| <= 1
else
   {
   // initialize error term
   error = dx2 - dy; 

   // draw the line
   for (index=0; index <= dy; index++)
       {
       // set the pixel
       *vb_start = color;

       // test if error overflowed
       if (error >= 0)
          {
          error-=dy2;

          // move to next line
          vb_start+=x_inc;

          } // end if error overflowed

       // adjust the error term
       error+=dx2;

       // move to the next pixel
       vb_start+=y_inc;

       } // end for

   } // end else |slope| > 1

// return success
return(1);

} // end Draw_Line

int Draw_Text_GDI(char *text, int x,int y,COLORREF color, LPDIRECTDRAWSURFACE7 lpdds)
{
// this function draws the sent text on the sent surface 
// using color index as the color in the palette

HDC xdc; // the working dc

// get the dc from surface
if (FAILED(lpdds->GetDC(&xdc)))
   return(0);

// set the colors for the text up
SetTextColor(xdc,color);

// set background mode to transparent so black isn't copied
SetBkMode(xdc, TRANSPARENT);

// draw the text a
TextOut(xdc,x,y,text,strlen(text));

// release the dc
lpdds->ReleaseDC(xdc);

// return success
return(1);
} // end Draw_Text_GDI

///////////////////////////////////////////////////////////


