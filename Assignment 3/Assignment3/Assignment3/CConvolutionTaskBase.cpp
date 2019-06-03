/******************************************************************************
GPU Computing / GPGPU Praktikum source code.

******************************************************************************/

#include "CConvolutionTaskBase.h"

#include "../Common/CLUtil.h"

#include "Pfm.h"

#include <sstream>
#include <string.h>
#include <stdio.h>
#include <assert.h>
#include <cstdint>
#include <vector>

using namespace std;

///////////////////////////////////////////////////////////////////////////////
// CConvolutionTaskBase

CConvolutionTaskBase::CConvolutionTaskBase(const std::string& FileName, bool Monochrome)
	: m_FileName(FileName), m_Monochrome(Monochrome)
{
}

CConvolutionTaskBase::~CConvolutionTaskBase()
{
	ReleaseResources();
}

bool CConvolutionTaskBase::InitResources(cl_device_id , cl_context Context)
{
	PFM inputPfm;
	if (!inputPfm.LoadRGB(m_FileName.c_str())) {
		cerr<<"Error loading file: " << m_FileName.c_str() << "." << endl;
		return false;
	}

	//internally, we convert the bitmap to floats, and execute the same convolution
	//operation on its three channels separately
	m_Height = inputPfm.height;
	m_Width = inputPfm.width;
	m_Pitch = m_Width;
	if(m_Width % 32 != 0)
		m_Pitch = m_Width + 32 - (m_Width % 32); //This will make sure that the data accesses are ALWAYS coalesced

	cout<<"Size of image: "<<m_Width<<" x "<<m_Height<<endl;

	//allocate data for the float channels
	for(int i = 0; i < 3; i++)
	{
		m_hSourceChannels[i] = new float[m_Height * m_Pitch];
		m_hCPUResultChannels[i] = new float[m_Height * m_Pitch];
		m_hGPUResultChannels[i] = new float[m_Height * m_Pitch];
	}

	//extract R, G, B channels
	unsigned int pixelOffset = 0;
	unsigned int trippleOffset = 0;
	for(unsigned int y = 0; y < m_Height; y++)
	{
		for(unsigned int x = 0; x < m_Width; x++)
		{
			m_hSourceChannels[0][pixelOffset] = inputPfm.pImg[trippleOffset    ];
			m_hSourceChannels[1][pixelOffset] = inputPfm.pImg[trippleOffset + 1];
			m_hSourceChannels[2][pixelOffset] = inputPfm.pImg[trippleOffset + 2];

			//monochrome: the data is converted to grayscale
			if(m_Monochrome)
				RGBToGrayScale(	m_hSourceChannels[0][pixelOffset],
								m_hSourceChannels[1][pixelOffset],
								m_hSourceChannels[2][pixelOffset]);

			pixelOffset++;
			trippleOffset += 3;
		}
		//pad the image with zeros
		for(unsigned int i = 0; i < m_Pitch - m_Width; i++)
		{
			m_hSourceChannels[0][pixelOffset + i] = 0.0f;
			m_hSourceChannels[1][pixelOffset + i] = 0.0f;
			m_hSourceChannels[2][pixelOffset + i] = 0.0f;
		}
		pixelOffset += m_Pitch - m_Width;
	}

	unsigned int dataSize = m_Pitch * m_Height * sizeof(cl_float);
	
	cl_int clError;
	for(int i = 0; i < 3; i++)
	{
		m_dSourceChannels[i] = clCreateBuffer(Context, CL_MEM_READ_ONLY | CL_MEM_COPY_HOST_PTR, dataSize, m_hSourceChannels[i], &clError);
		V_RETURN_FALSE_CL(clError, "Error allocating device input array");

		m_dResultChannels[i] = clCreateBuffer(Context, CL_MEM_WRITE_ONLY, dataSize, NULL, &clError);
		V_RETURN_FALSE_CL(clError, "Error allocating device output array");
	}

	return true;
}

void CConvolutionTaskBase::ReleaseResources()
{
	for(int i = 0; i < 3; i++)
	{
		SAFE_DELETE_ARRAY( m_hSourceChannels[i] );
		SAFE_DELETE_ARRAY( m_hCPUResultChannels[i] );
		SAFE_DELETE_ARRAY( m_hGPUResultChannels[i] );

		SAFE_RELEASE_MEMOBJECT( m_dSourceChannels[i] );
		SAFE_RELEASE_MEMOBJECT( m_dResultChannels[i] );
	}
}

bool CConvolutionTaskBase::ValidateResults()
{
	//number of channels to compute
	unsigned int numChannels = m_Monochrome ? 1 : 3;

	//calculate the average squared difference
	float avgError = 0;
	float maxError = 0;
	float numValues = float(numChannels * m_Width * (m_Height - 1));
	float scaling = 1.0f / numValues;

	for(unsigned int y = 0; y < m_Height; y++)
		for(unsigned int x = 0; x < m_Width; x++)
			for(unsigned int i = 0; i < numChannels; i++)
			{
				float L2Error = m_hCPUResultChannels[i][y * m_Pitch + x] - m_hGPUResultChannels[i][y * m_Pitch + x];
				L2Error = L2Error * L2Error;

				// Ignore the last line for the difference computations because we seem to have issues with NANs and other incorrect values in the last line with
				// the current driver version (versions 344.75, 344.11 and 335.23) in the separable kernel exercise.
				// This should be removed ASAP if the driver works again. You will also have to change the numValues initialization above.
				if (y < m_Height - 1)
				{
					maxError = max(maxError, L2Error);
					avgError += L2Error * scaling;
				}

				//to see the difference...
				m_hCPUResultChannels[i][y * m_Pitch + x] = L2Error * 100;
			}
	cout<<"Mean sq. error (MSE): "<<avgError<<endl;
	cout<<"Maximum sq. error: "<<maxError<<endl;

	//save difference image
	std::stringstream strm;
	strm<<"Images/DifferenceImage"<<m_FileNamePostfix<<".pfm";
	SaveImage(strm.str().c_str(), m_hCPUResultChannels);

	return (avgError < 1e-10f && maxError < 1e-8);
}

#ifdef HAVE_BIG_ENDIAN
# define SWAP_32(D) \
#	((D << 24) | ((D << 8) & 0x00FF0000)  \
#	 | ((D >> 8) & 0x0000FF00) | (D >> 24))
# define SWAP_16(D) ((D >> 8) | (D << 8))
#else
# define SWAP_32(D) (D)
# define SWAP_16(D) (D)
#endif

void
save_image_bmp(const char *path, unsigned char *data, int width, int height)
{
	FILE *f;

	if(!(f = fopen(path, "wb"))) {
		cerr << "Could not open \"" << path << "\"" << endl;
		return;
	}
	int numPixels = width * height;

	unsigned char header[] = {
		0x42, 0x4D, 0, 0, 0, 0, 0, 0,
		0, 0, 0x36, 0, 0, 0, 0x28, 0,
		0, 0, 0, 0, 0, 0, 0, 0, 0, 0,
		1, 0, 0x18, 0, 0, 0, 0, 0, 0,
		0, 0, 0, 0, 0, 0, 0, 0, 0, 0,
		0, 0, 0, 0, 0, 0, 0, 0, 0
	};

	int32_t width_end  = SWAP_32(width);
	int32_t height_end = SWAP_32(height);
	int32_t num_pix = numPixels * 3;
	int32_t wtf = 54 + numPixels * 3 + (height) * ((width & 3));

	memcpy(header + 18, &width_end,  sizeof(width_end));
	memcpy(header + 22, &height_end, sizeof(height_end));
	memcpy(header + 34, &num_pix,    sizeof(num_pix));
	memcpy(header +  2, &wtf,        sizeof(wtf));

	size_t w;
	w = fwrite(header, 54, 1, f);
	assert(w == 1);
	for(int y = height - 1; y >= 0; y--) {
		for(int x = 0; x < width; x++) {
			unsigned char* imgPos = data + 3 * (x + y * width);
			for(int i = 2; i >= 0; i--) { // reverse for BGR
				unsigned char c = *(imgPos + i);
				w = fwrite(&c, 1, 1, f);
				assert(w == 1);
			}
		}
		char temp = 0;
		for(int i = 0; i < (width) % 4; i++) {
			w = fwrite(&temp, 1, 1, f); // padding
			assert(w == 1);
		}
	}
	fclose(f);
}

void CConvolutionTaskBase::SaveImage(const std::string& FileName, float* Channels[3])
{
	// Save the result back to the disk
	PFM resPfm;
	resPfm.pImg = new float[m_Width * m_Height * 3];
	unsigned int pfmOffset = 0;
	unsigned int pixOffset = 0;
	for(unsigned int y = 0; y < m_Height; y++)
	{
		for(unsigned int x = 0; x < m_Width; x++)
		{
			if(m_Monochrome)
			{
				resPfm.pImg[pfmOffset] = Channels[0][pixOffset];
				resPfm.pImg[pfmOffset + 1] = Channels[0][pixOffset];
				resPfm.pImg[pfmOffset + 2] = Channels[0][pixOffset];
			}
			else
			{
				resPfm.pImg[pfmOffset] = Channels[0][pixOffset];
				resPfm.pImg[pfmOffset + 1] = Channels[1][pixOffset];
				resPfm.pImg[pfmOffset + 2] = Channels[2][pixOffset];
			}
			pfmOffset += 3;
			pixOffset++;
		}
		pixOffset += m_Pitch - m_Width;
	}
	resPfm.width = m_Width;
	resPfm.height = m_Height;
	if(!resPfm.SaveRGB(FileName.c_str()))
	{
		cerr<<"Error saving "<<FileName<<"."<<endl;
	}

	// stupid to do it here...
	//
	
	vector<uint8_t> img(m_Width * m_Height * 3);
	for(size_t i = 0; i < img.size(); i++) {
		img[i] = std::min<int>(0xff, std::max<int>(0, int(resPfm.pImg[i] * 0xff)));
	}
	string bmp_path = FileName;
	bmp_path.replace(bmp_path.rfind(".pfm"), 4, ".bmp");
	//save_image_bmp(bmp_path.c_str(), img.data(), m_Width, m_Height);
}

void CConvolutionTaskBase::SaveIntImage(const std::string& FileName, int* Channel)
{
	// Write data to the disc
	PFM resPfm;
	resPfm.pImg = new float[m_Width * m_Height];
	unsigned int pfmOffset = 0;
	unsigned int pixOffset = 0;
	for(unsigned int y = 0; y < m_Height; y++)
	{
		for(unsigned int x = 0; x < m_Width; x++)
		{
			resPfm.pImg[pfmOffset]		= (float)Channel[pixOffset];
			pfmOffset++;
			pixOffset++;
		}
		pixOffset += m_Pitch - m_Width;
	}
	resPfm.width = m_Width;
	resPfm.height = m_Height;
	if(!resPfm.SaveGrayscale(FileName.c_str()))
	{
		cout<<"Error saving "<<FileName<<"."<<endl;
		return;
	}
}

float CConvolutionTaskBase::RGBToGrayScale(float R, float G, float B)
{
	return 0.3f * R + 0.59f * G + 0.11f * B;
}

unsigned int CConvolutionTaskBase::To8BitChannel(float Value)
{
	Value = Value * 255.0f;
	if(Value > 255.0f)
		Value = 255.0f;
	else if(Value < 0.0f)
		Value = 0;

	return unsigned(Value);
}

///////////////////////////////////////////////////////////////////////////////
