
#include "Pfm.h"
#include <string.h>

#include <cstring>

#ifdef _MSC_VER
#pragma warning(disable: 4996) //fopen
#endif

//basic constructor
PFM::PFM(){
    Reset();
}

//destructor
PFM::~PFM(){
	Release();
}



//load a bitmap from a file and represent it correctly
//in memory
bool PFM::LoadRGB(const char *file) {

	Release();

	FILE *f = fopen( file, "rb" );

	if ( !f )  {
		fprintf( stderr, "PFM::Load: Error opening file '%s'\n", file );
		return false;
	}

	char tmp[ 1024 ];
	fscanf( f, "%s\n", tmp );
	if ( strcmp( tmp, "PF" ) != 0 )
		return false;

	fscanf( f, "%d%d", &width, &height );
	float sc;
	fscanf( f, "%f", &sc );

	pImg = new float[ width * height * 3];

	fread( pImg, 1, 1, f);
	fread( pImg, sizeof(float) * 3, width * height, f);

	fclose( f );
	return true;
}

bool PFM::SaveRGB(const char* file) {

	FILE *f = fopen( file, "wb" );

	if ( !f )  {
		fprintf( stderr, "PFM::Save: Error opening file '%s'\n", file );
		return false;
	}

	fprintf(f, "PF\n");
	fprintf(f, "%d %d\n", width, height );
	fprintf(f, "-1.0000000\n");

	fwrite( pImg, sizeof( float ) * 3,  width * height, f );

	fclose( f );
	return true;

}



//load a bitmap from a file and represent it correctly
//in memory
bool PFM::LoadGrayscale(const char *file) {

	Release();

	FILE *f = fopen( file, "rb" );

	if ( !f )  {
		fprintf( stderr, "PFM::Load: Error opening file '%s'\n", file );
		return false;
	}

	char tmp[ 1024 ];
	fscanf( f, "%s\n", tmp );
	if ( strcmp( tmp, "Pf" ) != 0 )
		return false;

	fscanf( f, "%d%d", &width, &height );
	float sc;
	fscanf( f, "%f", &sc );

	pImg = new float[ width * height];

	fread( pImg, 1, 1, f);
	fread( pImg, sizeof(float), width * height, f );

	fclose( f );
	return true;
}

bool PFM::SaveGrayscale(const char* file) {

	FILE *f = fopen( file, "wb" );

	if ( !f )  {
		fprintf( stderr, "PFM::Save: Error opening file '%s'\n", file );
		return false;
	}

	fprintf(f, "Pf\n");
	fprintf(f, "%d %d\n", width, height );
	fprintf(f, "-1.00000\n");

	fwrite( pImg, sizeof(float),  width * height, f );

	fclose( f );
	return true;

}


//function to set the inital values
void PFM::Reset(void) {
	height = 0;
	width  = 0;
    pImg = NULL;
}

void PFM::Release(void){
	if (pImg)
		delete [] pImg;
}
