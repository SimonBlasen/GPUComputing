////////////////////////////////////////////////////////////
//                                                        //
//  Simple OpenGL Texture Loader                          //
//  (w)(c)2007 Carsten Dachsbacher                        //
//                                                        //
////////////////////////////////////////////////////////////

#include "CGLTexture.h"
#include <stdio.h>

#ifdef _MSC_VER
#pragma warning ( disable : 4996 ) 
#endif

CGLTexture::CGLTexture( bool _rectangular )
{
	ID = 0;
	if (_rectangular )
		target = GL_TEXTURE_RECTANGLE_ARB; else
		target = GL_TEXTURE_2D;
}

CGLTexture::~CGLTexture()
{
	deleteTexture();
}

bool CGLTexture::createTexture()
{
	deleteTexture();

	glGenTextures( 1, &ID );

	ID ++;

	glBindTexture  ( target, ID - 1 );
	glTexParameterf( target, GL_TEXTURE_WRAP_S, GL_REPEAT );
	glTexParameterf( target, GL_TEXTURE_WRAP_T, GL_REPEAT );
	glTexParameterf( target, GL_TEXTURE_MAG_FILTER, GL_LINEAR_MIPMAP_LINEAR );
	glTexParameterf( target, GL_TEXTURE_MIN_FILTER, GL_LINEAR_MIPMAP_LINEAR );

	return true;
}

void CGLTexture::deleteTexture()
{
	if( ID )
	{
		ID --;
		glDeleteTextures( 1, &ID );
		ID = 0;
	}
}

void CGLTexture::bind()
{
	glBindTexture( target, ID - 1 );
}

#define TGA_RGB		 2		// RGB file
#define TGA_A		 3		// ALPHA file
#define TGA_RLE		10		// run-length encoded

bool CGLTexture::loadTGA( const char *filename )
{
	unsigned short wwidth, 
				   wheight;
	unsigned char  length, 
				   imageType, 
				   bits;
	unsigned long  stride, 
				   channels;
	unsigned char *data;

	width = height = 0;
	createTexture();
	
	FILE *f;

	if( ( f = fopen( filename, "rb" ) ) == NULL ) 
	{
		fprintf( stderr, "[texture]: error opening file '%s'\n", filename );
		return false;
	}
		
	fread( &length, sizeof( unsigned char ), 1, f );
	fseek( f, 1, SEEK_CUR );
	fread( &imageType, sizeof( unsigned char ), 1, f );
	fseek( f, 9, SEEK_CUR ); 

	fread( &wwidth,  sizeof( unsigned short ), 1, f );
	fread( &wheight, sizeof( unsigned short ), 1, f );
	fread( &bits,    sizeof( unsigned char ), 1, f );
	
	width  = wwidth;
	height = wheight;

	fseek( f, length + 1, SEEK_CUR ); 

	// umcompressed image file
	if( imageType != TGA_RLE )
	{
		// true color
		if( bits == 24 || bits == 32 )
		{
			channels = bits / 8;
			stride   = channels * width;
			data     = new unsigned char[ stride * height ];

			for( int y = 0; y < height; y++ )
			{
				unsigned char *pLine = &( data[ stride * y ] );

				fread( pLine, stride, 1, f );
			
				for( int i = 0; i < (int)stride; i += channels )
				{
					unsigned char temp	   = pLine[ i ];
					pLine[ i ]     = pLine[ i + 2 ];
					pLine[ i + 2 ] = temp;
				}
			}
		} else 
		// hi color
		if( bits == 16 )
		{
			unsigned short pixels = 0;
			int r, g, b;

			channels = 3;
			stride   = channels * width;
			data     = new unsigned char[ stride * height ];

			for( int i = 0; i < (int)(width*height); i++ )
			{
				fread( &pixels, sizeof(unsigned short), 1, f );
				
				b =   ( pixels & 0x1f ) << 3;
				g = ( ( pixels >> 5 ) & 0x1f ) << 3;
				r = ( ( pixels >> 10 ) & 0x1f ) << 3;
				
				data[ i * 3 + 0 ] = (unsigned char) r;
				data[ i * 3 + 1 ] = (unsigned char) g;
				data[ i * 3 + 2 ] = (unsigned char) b;
			}
		} else
			return false;
	} else
	// RLE compressed image
	{
		unsigned char rleID = 0;
		int colorsRead = 0;
		
		channels = bits / 8;
		stride   = channels * width;

		data = new unsigned char[ stride * height ];
		unsigned char *pColors = new unsigned char [ channels ];

		int i = 0;
		while( i < width * height )
		{
			fread( &rleID, sizeof( unsigned char ), 1, f );
			
			if( rleID < 128 )
			{
				rleID++;

				while( rleID )
				{
					fread( pColors, sizeof( unsigned char ) * channels, 1, f );

					data[ colorsRead + 0 ] = pColors[ 2 ];
					data[ colorsRead + 1 ] = pColors[ 1 ];
					data[ colorsRead + 2 ] = pColors[ 0 ];

					if ( bits == 32 )
						data[ colorsRead + 3 ] = pColors[ 3 ];

					i ++;
					rleID --;
					colorsRead += channels;
				}
			} else
			{
				rleID -= 127;

				fread( pColors, sizeof( unsigned char ) * channels, 1, f );

				while( rleID )
				{
					data[ colorsRead + 0 ] = pColors[ 2 ];
					data[ colorsRead + 1 ] = pColors[ 1 ];
					data[ colorsRead + 2 ] = pColors[ 0 ];

					if ( bits == 32 )
						data[ colorsRead + 3 ] = pColors[ 3 ];

					i ++;
					rleID --;
					colorsRead += channels;
				}
			}
		}

		delete[] pColors;
	}

	fclose( f );

	if ( channels == 4 )
		gluBuild2DMipmaps( target, 4, width, height, GL_RGBA, GL_UNSIGNED_BYTE, data ); else
		gluBuild2DMipmaps( target, 3, width, height, GL_RGB,  GL_UNSIGNED_BYTE, data );

	delete[] data;

	fprintf( stderr, "[texture]: loaded '%s'\n", filename );

	return true;
}
