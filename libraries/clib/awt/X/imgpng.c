/**
 * imgpng.c - interface for libpng input routines
 *
 * Copyright (c) 1998
 *      Transvirtual Technologies, Inc.  All rights reserved.
 *
 * See the file "license.terms" for information on usage and redistribution 
 * of this file. 
 */

#include "config.h"
#include "toolkit.h"

#if defined(HAVE_PNG_H)
#include "png.h"


/* references to img.c */
Image* createImage ( int width, int height );
void Java_java_awt_Toolkit_imgFreeImage( JNIEnv* env, jclass clazz, Image * img);
XImage* createXImage ( Toolkit* X, int width, int height );
XImage* createXMaskImage ( Toolkit* X, int width, int height );
AlphaImage* createAlphaImage ( Toolkit* X, int width, int height );
void reduceAlpha ( Toolkit* X, Image* img, int threshold );


/**************************************************************************************
 * auxiliary funtions
 */

__inline__ int
hasAlpha ( png_structp png_ptr )
{
  return  (png_ptr->color_type == PNG_COLOR_TYPE_GRAY_ALPHA ||
		   png_ptr->color_type == PNG_COLOR_TYPE_RGB_ALPHA);

}

__inline__ jint
readARGB ( unsigned char** p, int hasAlpha )
{
  jint argb;

  argb = *(*p)++;
  argb <<= 8;
  argb += *(*p)++;
  argb <<= 8;
  argb += *(*p)++;

  if ( hasAlpha ) {
	argb <<= 8; 
	argb += *(*p)++;
  }

  return argb;
}


__inline__ void
setPixel ( Image* img, unsigned long argb, int row, int col )
{
  if ( img->alpha ){
	PutAlpha( img->alpha, col, row, argb >> 24);
  }

  XPutPixel( img->xImg, col, row, pixelValue( X, argb));
}


int
readRowData ( png_structp png_ptr, png_infop info_ptr, png_bytep row, Image *img )
{
  int            i, j;
  jint           argb;
  unsigned char  *p;

  for ( i = 0; i < info_ptr->height; i++ ) {
	png_read_row( png_ptr, row, 0);

	for ( j=0, p=(unsigned char*)row; j<info_ptr->width; j++ ) {
	  argb = readARGB( &p, (img->alpha != 0));
	  setPixel( img, argb, i, j);
	}
  }
}

int
readImageData ( png_structp png_ptr, png_infop info_ptr, png_bytepp rows, Image *img )
{
  int            i, j;
  jint           argb;
  unsigned char  *p;

  png_read_image( png_ptr, rows);

  for ( i=0; i<info_ptr->height; i++ ) {
	for ( j=0, p=(unsigned char*)rows[i]; j<info_ptr->width; j++ ) {
	  argb = readARGB( &p, (img->alpha != 0));
	  setPixel( img, argb, i, j);
	}
  }
}

void
readbackRow ( Image *img, unsigned char* rowBuf, int row )
{
  int            i, j;
  unsigned char  *p;
  int            r, g, b, a;
  unsigned long  pix;

  for ( i=0, p=rowBuf; i<img->width; i++ ) {
	pix = XGetPixel( img->xImg, i, row);
	rgbValues( X, pix, &r, &g, &b);

	a = (img->alpha) ? GetAlpha( img->alpha, i, row) : 0xff;

	*p++ = a;
	*p++ = r;
	*p++ = g;
	*p++ = b;
  }
}

/*
 * THIS DOESN'T WORK YET. The idea is to avoid allocating temporary
 * memory for the WHOLE image in ARGB pels (but ADAM7 seems to require
 * neighbor rows, too)
 */
int
readInterlacedData ( png_structp png_ptr, png_infop info_ptr, png_bytep row, Image *img )
{
  int   i, j, pass;
  jint           argb;
  unsigned char  *p;


  for ( pass=0; pass<7; pass++ ) {
	for ( i = 0; i < info_ptr->height; i++ ) {
	  if ( pass ) {
		readbackRow( img, row, i);
	  }
	  png_read_row( png_ptr, row, 0);

	  for ( j=0, p=(unsigned char*)row; j<info_ptr->width; j++ ) {
		argb = readARGB( &p, (img->alpha != 0));
		setPixel( img, argb, i, j);
	  }
	}
  }
}


Image*
readPng ( png_structp png_ptr, png_infop info_ptr )
{
  Image          *img = 0;
  png_uint_32    num_palette;
  png_colorp     palette;
  double         screen_gamma = 1.2, file_gamma;
  png_color_16p  histogram;
  int            i, intent, number_passes;
  int            row_bytes;
  png_bytepp     rows = 0;
  png_bytep      data = 0;

  if ( setjmp(png_ptr->jmpbuf) ) {
	if ( img )
	  Java_java_awt_Toolkit_imgFreeImage( 0, 0, img);
	if ( rows )
	  free( rows);
	if ( data )
	  free( data);
	return 0;
  }

  png_read_info( png_ptr, info_ptr);
  
  /* handle gamma correction  */
#if defined(PNG_READ_sRGB_SUPPORTED)
  if ( png_get_sRGB( png_ptr, info_ptr, &intent) ) {
	png_set_sRGB( png_ptr, info_ptr, intent);
  }
  else
#endif
  {
	if ( png_get_gAMA(png_ptr, info_ptr, &file_gamma) )
	  png_set_gamma( png_ptr, screen_gamma, file_gamma);
	else
	  png_set_gamma( png_ptr, screen_gamma, 0.50);
  }

  png_set_strip_16( png_ptr);
  png_set_expand( png_ptr);
  png_set_gray_to_rgb( png_ptr);
  png_set_swap_alpha( png_ptr); /* we need ARGB instead of the standard RGBA */

  row_bytes     = png_get_rowbytes( png_ptr, info_ptr);
  number_passes = png_set_interlace_handling( png_ptr);

  /* Optional call to gamma correct and add the background to the palette */
  png_read_update_info( png_ptr, info_ptr);

  /* time to create the image */
  img = createImage( info_ptr->width, info_ptr->height);
  img->xImg = createXImage( X, img->width, img->height);
  if ( hasAlpha( png_ptr) )
	img->alpha = createAlphaImage( X, img->width, img->height);

  if ( info_ptr->interlace_type != 0 ) {
#ifndef OPTIMIZE_SPACE
	/*
	 * This is bad: to read an interlaced image, we need enough space to (temporarily)
	 * store the whole transformed data (passes need prev. results). Unfortunately,
	 * interlacing is used for large images, and this might require a LOT of memory.
	 */
	rows = malloc( sizeof(png_bytep) * info_ptr->height);
	data = malloc( row_bytes * info_ptr->height);
	for ( i=0; i<info_ptr->height; i++ )
	  rows[i] = (data + i*row_bytes);

	readImageData( png_ptr, info_ptr, rows, img);

	free( rows);
	free( data);
#else
	data = malloc( row_bytes);
	readInterlacedData( png_ptr, info_ptr, data, img);
	free( data);
#endif
  }
  else {
	data = malloc( row_bytes);
	readRowData( png_ptr, info_ptr, data, img);
	free( data);
  }

  /* read rest of file, and get additional chunks in info_ptr */
  png_read_end( png_ptr, info_ptr);

  /* try to reduce alphas in the name of efficiency */
  if ( img->alpha &&  !needsFullAlpha( X, img, 0.01) )
	reduceAlpha( X, img, 128);

  return img;
}


/**************************************************************************************
 * memory buffer IO
 */

typedef struct {
  unsigned char *buf;
  unsigned char *p;
  long          remain;
} BufferSource;

void 
bufferRead ( png_structp png_ptr, png_bytep data, png_size_t length )
{
  BufferSource *psource = png_get_io_ptr( png_ptr);

  if ( psource->remain >= length ) {
	memcpy( data, psource->p, length);
	psource->p += length;
	psource->remain -= length;
  }
  else
	png_error( png_ptr, "illegal data read");
}


#endif /* HAVE_PNG_H */

/**************************************************************************************
 * these are the "exported" production interface functions
 */

Image*
readPngFile ( FILE* infile )
{
  Image          *img = 0;
#if defined(HAVE_PNG_H)
  png_structp    png_ptr;
  png_infop      info_ptr;

  png_ptr = png_create_read_struct( PNG_LIBPNG_VER_STRING,
									(png_voidp)0, (png_error_ptr)0, (png_error_ptr)0);
  if ( !png_ptr )
	return 0;

  info_ptr = png_create_info_struct( png_ptr);
  png_init_io( png_ptr, infile);

  img = readPng( png_ptr, info_ptr);

  png_destroy_read_struct( &png_ptr, &info_ptr, (png_infopp)NULL);
#endif /* HAVE_PNG_H */

  return img;
}


Image*
readPngData ( unsigned char* buf, long len )
{
  Image          *img = 0;
#if defined(HAVE_PNG_H)
  png_structp    png_ptr;
  png_infop      info_ptr;
  BufferSource   source;

  source.buf = source.p = buf;
  source.remain = len;

  png_ptr = png_create_read_struct( PNG_LIBPNG_VER_STRING,
									(png_voidp)0, (png_error_ptr)0, (png_error_ptr)0);
  if ( !png_ptr )
	return 0;

  info_ptr = png_create_info_struct( png_ptr);
  png_set_read_fn( png_ptr, &source, bufferRead);

  img = readPng( png_ptr, info_ptr);

  png_destroy_read_struct( &png_ptr, &info_ptr, (png_infopp)NULL);
#endif /* HAVE_PNG_H */

  return img;
}
