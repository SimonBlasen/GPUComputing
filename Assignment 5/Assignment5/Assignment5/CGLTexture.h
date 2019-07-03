////////////////////////////////////////////////////////////
//                                                        //
//  Simple OpenGL Texture Loader                          //
//  (w)(c)2007 Carsten Dachsbacher                        //
//                                                        //
////////////////////////////////////////////////////////////
#ifndef __TEXTURE_H
#define __TEXTURE_H

#include "GLCommon.h"

class CGLTexture
{
private:
	void	deleteTexture();
	bool	createTexture();

	GLenum	target;
	int		width, height;
	GLuint	ID;


public:
	CGLTexture( bool _rectangular = false );
	~CGLTexture();

	GLuint	getID()      { return ID-1; };
	int		getWidth()   { return width; };
	int		getHeight()  { return height; };
	
	bool	loadTGA		 ( const char *fileName );

	void	bind();
};

#endif
