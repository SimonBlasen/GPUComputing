#ifndef PFM_H
#define PFM_H

#include <iostream>
#include <cstdio>
#include <string>
using namespace std;


class PFM {
public:

	//variables
	int width;
	int height;
	float *pImg;

 
	//methods
    PFM(void);
    ~PFM();
    bool LoadRGB(const char *);
	bool SaveRGB(const char*); 
    bool LoadGrayscale(const char *);
	bool SaveGrayscale(const char*); 

private:

    //methods
    void Reset(void);
	void Release(void);
};

#endif //_BITMAP_H


