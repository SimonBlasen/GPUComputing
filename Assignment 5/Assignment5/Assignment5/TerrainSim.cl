#define DAMPING 0.02f

#define G_ACCEL (float4)(0.f, -9.81f, 0.f, 0.f)
#define P_ARRAY (int[]) {151,160,137,91,90,15,131, 13, 201, 95, 96, 53, 194, 233, 7, 225, 140, 36, 103, 30, 69, 142, 8, 99, 37, 240, 21, 10, 23,190, 6, 148, 247, 120, 234, 75, 0, 26, 197, 62, 94, 252, 219, 203, 117, 35, 11, 32, 57, 177, 33,88, 237, 149, 56, 87, 174, 20, 125, 136, 171, 168, 68, 175, 74, 165, 71, 134, 139, 48, 27, 166,77, 146, 158, 231, 83, 111, 229, 122, 60, 211, 133, 230, 220, 105, 92, 41, 55, 46, 245, 40, 244,102, 143, 54, 65, 25, 63, 161, 1, 216, 80, 73, 209, 76, 132, 187, 208, 89, 18, 169, 200, 196,135, 130, 116, 188, 159, 86, 164, 100, 109, 198, 173, 186, 3, 64, 52, 217, 226, 250, 124, 123,5, 202, 38, 147, 118, 126, 255, 82, 85, 212, 207, 206, 59, 227, 47, 16, 58, 17, 182, 189, 28, 42,223, 183, 170, 213, 119, 248, 152, 2, 44, 154, 163, 70, 221, 153, 101, 155, 167, 43, 172, 9,129, 22, 39, 253, 19, 98, 108, 110, 79, 113, 224, 232, 178, 185, 112, 104, 218, 246, 97, 228,251, 34, 242, 193, 238, 210, 144, 12, 191, 179, 162, 241, 81, 51, 145, 235, 249, 14, 239, 107,49, 192, 214, 31, 181, 199, 106, 157, 184, 84, 204, 176, 115, 121, 50, 45, 127, 4, 150, 254,138, 236, 205, 93, 222, 114, 67, 29, 24, 72, 243, 141, 128, 195, 78, 66, 215, 61, 156, 180,151, 160, 137, 91, 90, 15,131, 13, 201, 95, 96, 53, 194, 233, 7, 225, 140, 36, 103, 30, 69, 142, 8, 99, 37, 240, 21, 10, 23,190, 6, 148, 247, 120, 234, 75, 0, 26, 197, 62, 94, 252, 219, 203, 117, 35, 11, 32, 57, 177, 33,88, 237, 149, 56, 87, 174, 20, 125, 136, 171, 168, 68, 175, 74, 165, 71, 134, 139, 48, 27, 166,77, 146, 158, 231, 83, 111, 229, 122, 60, 211, 133, 230, 220, 105, 92, 41, 55, 46, 245, 40, 244,102, 143, 54, 65, 25, 63, 161, 1, 216, 80, 73, 209, 76, 132, 187, 208, 89, 18, 169, 200, 196,135, 130, 116, 188, 159, 86, 164, 100, 109, 198, 173, 186, 3, 64, 52, 217, 226, 250, 124, 123,5, 202, 38, 147, 118, 126, 255, 82, 85, 212, 207, 206, 59, 227, 47, 16, 58, 17, 182, 189, 28, 42,223, 183, 170, 213, 119, 248, 152, 2, 44, 154, 163, 70, 221, 153, 101, 155, 167, 43, 172, 9,129, 22, 39, 253, 19, 98, 108, 110, 79, 113, 224, 232, 178, 185, 112, 104, 218, 246, 97, 228,251, 34, 242, 193, 238, 210, 144, 12, 191, 179, 162, 241, 81, 51, 145, 235, 249, 14, 239, 107,49, 192, 214, 31, 181, 199, 106, 157, 184, 84, 204, 176, 115, 121, 50, 45, 127, 4, 150, 254,138, 236, 205, 93, 222, 114, 67, 29, 24, 72, 243, 141, 128, 195, 78, 66, 215, 61, 156, 180}
#define SEED 0

#define WEIGHT_ORTHO	0.138f
#define WEIGHT_DIAG		0.097f
#define WEIGHT_ORTHO_2	0.069f
#define WEIGHT_DIAG_2	0.048f


#define ROOT_OF_2 1.4142135f
#define DOUBLE_ROOT_OF_2 2.8284271f
#define PI_HALF 1.5707963f

#define TILE_X 16 
#define TILE_Y 16
#define HALOSIZE 2



int noise2(int x, int y, unsigned long seed)
{
	int tmp = P_ARRAY[(y + seed) % 256];
	return P_ARRAY[(tmp + x) % 256];
}

float lin_inter(float x, float y, float s)
{
	return x + s * (y - x);
}

float smooth_inter(float x, float y, float s)
{
	return lin_inter(x, y, s * s * (3 - 2 * s));
}

float noise2d(float x, float y, unsigned long seed)
{
	int x_int = x;
	int y_int = y;
	float x_frac = x - x_int;
	float y_frac = y - y_int;
	int s = noise2(x_int, y_int, seed);
	int t = noise2(x_int + 1, y_int, seed);
	int u = noise2(x_int, y_int + 1, seed);
	int v = noise2(x_int + 1, y_int + 1, seed);
	float low = smooth_inter(s, t, x_frac);
	float high = smooth_inter(u, v, x_frac);
	return smooth_inter(low, high, y_frac);
}

float perlin2d(float x, float y, float freq, int depth, unsigned long seed)
{
	float xa = x * freq;
	float ya = y * freq;
	float amp = 1.0;
	float fin = 0;
	float div = 0.0;

	int i;
	for (i = 0; i < depth; i++)
	{
		div += 256 * amp;
		fin += noise2d(xa, ya, seed) * amp;
		amp /= 2;
		xa *= 2;
		ya *= 2;
	}

	return fin / div;
}




float inverseTan(float x)
{
	return (3.f * x) / (1.f + 2.f * sqrt(1.f + x * x));
}



/*
double fade(double t)
{
	return t * t * t * (t * (t * 6 - 15) + 10);
}

double lerp(double t, double a, double b)
{
	return a + t * (b - a);
}

double grad(int hash, double x, double y, double z)
{
	int h = hash & 15;                      // CONVERT LO 4 BITS OF HASH CODE
	double u = h < 8 ? x : y,                 // INTO 12 GRADIENT DIRECTIONS.
		v = h < 4 ? y : h == 12 || h == 14 ? x : z;
	return ((h & 1) == 0 ? u : -u) + ((h & 2) == 0 ? v : -v);
}

double noise(double x, double y, double z)
{
	int X = ((int)x) & 255,                  // FIND UNIT CUBE THAT
		Y = ((int)y) & 255,                  // CONTAINS POINT.
		Z = ((int)z) & 255;
	x -= (int)x;                              // FIND RELATIVE X,Y,Z
	y -= (int)y;                                // OF POINT IN CUBE.
	z -= (int)z;
	double u = fade(x),                                // COMPUTE FADE CURVES
		v = fade(y),                                // FOR EACH OF X,Y,Z.
		w = fade(z);
	int A = P_ARRAY[X] + Y, AA = P_ARRAY[A] + Z, AB = P_ARRAY[A + 1] + Z,      // HASH COORDINATES OF
		B = P_ARRAY[X + 1] + Y, BA = P_ARRAY[B] + Z, BB = P_ARRAY[B + 1] + Z;      // THE 8 CUBE CORNERS,

	return lerp(w, lerp(v, lerp(u, grad(P_ARRAY[AA], x, y, z),  // AND ADD
		grad(P_ARRAY[BA], x - 1, y, z)), // BLENDED
		lerp(u, grad(P_ARRAY[AB], x, y - 1, z),  // RESULTS
			grad(P_ARRAY[BB], x - 1, y - 1, z))),// FROM  8
		lerp(v, lerp(u, grad(P_ARRAY[AA + 1], x, y, z - 1),  // CORNERS
			grad(P_ARRAY[BA + 1], x - 1, y, z - 1)), // OF CUBE
			lerp(u, grad(P_ARRAY[AB + 1], x, y - 1, z - 1),
				grad(P_ARRAY[BB + 1], x - 1, y - 1, z - 1))));
}



*/


__kernel void InitHeightfield(unsigned int width,
	unsigned int height,
	unsigned long seed,
	__global float4* d_pos)
{
	// Make sure the work-item does not map outside the cloth
	if (get_global_id(0) >= width || get_global_id(1) >= height)
		return;


	int2 GID;
	GID.x = get_global_id(0);
	GID.y = get_global_id(1);

	int2 LID;
	LID.x = get_local_id(0);
	LID.y = get_local_id(1);

	int2 LSIZE;
	LSIZE.x = get_local_size(0);
	LSIZE.y = get_local_size(1);


	unsigned int particleID = get_global_id(0) + get_global_id(1) * width;

	float4 x0 = d_pos[particleID];

	//x0.y = x0.y + sin(x0.x * 20.f + x0.z * 26.f) * 0.06f + cos(x0.x * 23.f + x0.z * 16.f) * 0.06f;
	x0.y = perlin2d(GID.x, GID.y, 0.006f * 1.f, 1.f, seed) * 0.15f + perlin2d(GID.x, GID.y, 0.0312f * 1.f, 1.f, seed) * 0.003f;

	d_pos[particleID] = x0;



}




#define RAIN_IMPACT 0.001f
#define RAIN_ABSORBATION 1
#define DELTA_T 0.7f
#define SLOPE_FACTOR 100000.f
//#define SMOOTH_STEP 0.01f
#define WEIGHT_SMOOTH_DIAG 0.02f
#define WEIGHT_SMOOTH_ORTO 0.02f
#define RAIN_LIFETIME 100;




///////////////////////////////////////////////////////////////////////////////
// The integration kernel
// Input data:
// width and height - the dimensions of the particle grid
// d_pos - the most recent position of the cloth particle while...
// d_prevPos - ...contains the position from the previous iteration.
// elapsedTime      - contains the elapsed time since the previous invocation of the kernel,
// prevElapsedTime  - contains the previous time step.
// simulationTime   - contains the time elapsed since the start of the simulation (useful for wind)
// All time values are given in seconds.
//
// Output data:
// d_prevPos - Input data from d_pos must be copied to this array
// d_pos     - Updated positions
///////////////////////////////////////////////////////////////////////////////
  __kernel void Integrate(unsigned int width,
						unsigned int height, 
						__global float4* d_pos,
						__global float4* d_prevPos,
						__global unsigned int* d_rain,
						__global unsigned int* d_newRain,
						float elapsedTime,
						float prevElapsedTime,
						float simulationTime,
						unsigned int randomSeedX,
						unsigned int randomSeedY) {

	// Make sure the work-item does not map outside the cloth
    if(get_global_id(0) >= width || get_global_id(1) >= height)
		return;

	int2 GID;
	GID.x = get_global_id(0);
	GID.y = get_global_id(1);

	int2 LID;
	LID.x = get_local_id(0);
	LID.y = get_local_id(1);

	int2 LSIZE;
	LSIZE.x = get_local_size(0);
	LSIZE.y = get_local_size(1);





	__local float4 tile[TILE_Y + HALOSIZE][TILE_X + HALOSIZE];
	__local float tileSlopes[TILE_Y + HALOSIZE][TILE_X + HALOSIZE];
	__local unsigned int tileRain[TILE_Y + HALOSIZE][TILE_X + HALOSIZE];



	// Top side halo
	if (LID.y == 0)
	{
		if (GID.y == 0)
		{
			tile[0][LID.x + 1] = 0.f;
			tileRain[0][LID.x + 1] = 0;
		}
		else
		{
			tile[0][LID.x + 1] = d_pos[(GID.y - 1) * width + GID.x];
			tileRain[0][LID.x + 1] = d_rain[(GID.y - 1) * width + GID.x];
		}
	}

	// Down side halo
	if (LID.y == LSIZE.y - 1)
	{
		if (GID.y == height - 1)
		{
			tile[TILE_Y + 1][LID.x + 1] = 0.f;
			tileRain[TILE_Y + 1][LID.x + 1] = 0;
		}
		else
		{
			tile[TILE_Y + 1][LID.x + 1] = d_pos[(GID.y + 1) * width + GID.x];
			tileRain[TILE_Y + 1][LID.x + 1] = d_rain[(GID.y + 1) * width + GID.x];
		}
	}

	// Left side halo
	if (LID.x == 0)
	{
		if (GID.x == 0)
		{
			tile[LID.y + 1][0] = 0.f;
			tileRain[LID.y + 1][0] = 0;
		}
		else
		{
			tile[LID.y + 1][0] = d_pos[(GID.y) * width + GID.x - 1];
			tileRain[LID.y + 1][0] = d_rain[(GID.y) * width + GID.x - 1];
		}
	}

	// Right side halo
	if (LID.x == LSIZE.x - 1)
	{
		if (GID.x == width - 1)
		{
			tile[LID.y + 1][TILE_X + 1] = 0.f;
			tileRain[LID.y + 1][TILE_X + 1] = 0;
		}
		else
		{
			tile[LID.y + 1][TILE_X + 1] = d_pos[(GID.y) * width + GID.x + 1];
			tileRain[LID.y + 1][TILE_X + 1] = d_rain[(GID.y) * width + GID.x + 1];
		}
	}




	// Corners

	// Left top
	if (LID.x == 1 && LID.y == 1)
	{
		if (GID.x == 1 || GID.y == 1)
		{
			tile[0][0] = 0.f;
			tileRain[0][0] = 0;
		}
		else
		{
			tile[0][0] = d_pos[(GID.y - 2) * width + GID.x - 2];
			tileRain[0][0] = d_rain[(GID.y - 2) * width + GID.x - 2];
		}
	}

	// Right top
	if (LID.x == LSIZE.x - 2 && LID.y == 1)
	{
		if (GID.x == width - 2 || GID.y == 1)
		{
			tile[0][TILE_X + 1] = 0.f;
			tileRain[0][TILE_X + 1] = 0;
		}
		else
		{
			tile[0][TILE_X + 1] = d_pos[(GID.y - 2) * width + GID.x + 2];
			tileRain[0][TILE_X + 1] = d_rain[(GID.y - 2) * width + GID.x + 2];
		}
	}

	// Left bottom
	if (LID.x == 1 && LID.y == LSIZE.y - 2)
	{
		if (GID.x == 1 || GID.y == height - 2)
		{
			tile[TILE_Y + 1][0] = 0.f;
			tileRain[TILE_Y + 1][0] = 0;
		}
		else
		{
			tile[TILE_Y + 1][0] = d_pos[(GID.y + 2) * width + GID.x - 2];
			tileRain[TILE_Y + 1][0] = d_rain[(GID.y + 2) * width + GID.x - 2];
		}
	}

	// Right bottom
	if (LID.x == LSIZE.x - 2 && LID.y == LSIZE.y - 2)
	{
		if (GID.x == width - 1 || GID.y == height - 2)
		{
			tile[TILE_Y + 1][TILE_X + 1] = 0.f;
			tileRain[TILE_Y + 1][TILE_X + 1] = 0;
		}
		else
		{
			tile[TILE_Y + 1][TILE_X + 1] = d_pos[(GID.y + 2) * width + GID.x + 2];
			tileRain[TILE_Y + 1][TILE_X + 1] = d_rain[(GID.y + 2) * width + GID.x + 2];
		}
	}



	tile[LID.y + 1][LID.x + 1] = d_pos[(GID.y) * width + GID.x];
	tileRain[LID.y + 1][LID.x + 1] = d_rain[(GID.y) * width + GID.x];


	barrier(CLK_LOCAL_MEM_FENCE);





	unsigned int particleID = get_global_id(0) + get_global_id(1) * width;

	unsigned int newRain = d_newRain[particleID];

	if (newRain > 0 /*&& (LID.x == 0 || LID.y == 0 || LID.x == LSIZE.x - 1 || LID.y == LSIZE.y - 1)*/)
	{
		//atomic_add(d_rain + particleID, newRain);
		d_rain[particleID] += newRain;
	}

	unsigned int rainHere = tileRain[LID.y + 1][LID.x + 1];






	// Slope Halo
	//
	//














	//
	//
	// Slope Halo


	if (LID.x)


	float	slope0,
		slope1,
		slope2,
		slope3,
		slope4,
		slope5,
		slope6,
		slope7;
	if (GID.x + 1 < width && GID.y < height)
		slope0 = d_pos[(GID.x + 1) + width * GID.y].y - d_pos[particleID].y;
	else
		slope0 = 100000.f;
	if (GID.x + 1 < width && GID.y + 1 < height)
		slope1 = d_pos[(GID.x + 1) + width * (GID.y + 1)].y - d_pos[particleID].y;
	else
		slope1 = 100000.f;
	if (GID.x + 0 < width && GID.y + 1 < height)
		slope2 = d_pos[(GID.x + 0) + width * (GID.y + 1)].y - d_pos[particleID].y;
	else
		slope2 = 100000.f;
	if (GID.x - 1 >= 0 && GID.x - 1 < width && GID.y + 1 < height)
		slope3 = d_pos[(GID.x - 1) + width * (GID.y + 1)].y - d_pos[particleID].y;
	else
		slope3 = 100000.f;
	if (GID.x - 1 >= 0 && GID.x - 1 < width && GID.y + 0 >= 0 && GID.y + 0 < height)
		slope4 = d_pos[(GID.x - 1) + width * (GID.y + 0)].y - d_pos[particleID].y;
	else
		slope4 = 100000.f;
	if (GID.x - 1 >= 0 && GID.x - 1 < width && GID.y - 1 >= 0 && GID.y - 1 < height)
		slope5 = d_pos[(GID.x - 1) + width * (GID.y - 1)].y - d_pos[particleID].y;
	else
		slope5 = 100000.f;
	if (GID.x + 0 >= 0 && GID.x + 0 < width && GID.y - 1 >= 0 && GID.y - 1 < height)
		slope6 = d_pos[(GID.x + 0) + width * (GID.y - 1)].y - d_pos[particleID].y;
	else
		slope6 = 100000.f;
	if (GID.x + 1 >= 0 && GID.x + 1 < width && GID.y - 1 >= 0 && GID.y - 1 < height)
		slope7 = d_pos[(GID.x + 1) + width * (GID.y - 1)].y - d_pos[particleID].y;
	else
		slope7 = 100000.f;


	uint seed = randomSeedX + GID.x;
	//uint t = seed ^ (seed << 11);
	//uint random = randomSeedY ^ (randomSeedY >> 19) ^ (t ^ (t >> 8));

	uint random = (randomSeedX * GID.x * 401267 + randomSeedY * GID.y * 1013) % 32767;
	//uint random = 32767/2;





	unsigned int resultRain = 0;
	if (RAIN_ABSORBATION > rainHere)
	{
		resultRain = 0;
		if (rainHere != 0)
		{
			//atomic_add(d_rain + particleID, -rainHere);
		}
	}
	else
	{
		resultRain = rainHere - RAIN_ABSORBATION;
		//atomic_add(d_rain + particleID, -RAIN_ABSORBATION);
	}

	//atomic_add(d_rain + particleID, (-(rainHere) + (resultRain / 2)));
	atomic_add(d_rain + particleID, -(rainHere));




	float randF = (((float)random) / (32767.f)); //(((float)random) / (4294967296.f));

	//randF = 0.0f;

	slope0 = slope0 > 99990.f ? 0.f : inverseTan(-slope0 * SLOPE_FACTOR) + PI_HALF;
	slope1 = slope1 > 99990.f ? 0.f : inverseTan(-slope1 * SLOPE_FACTOR) + PI_HALF;
	slope2 = slope2 > 99990.f ? 0.f : inverseTan(-slope2 * SLOPE_FACTOR) + PI_HALF;
	slope3 = slope3 > 99990.f ? 0.f : inverseTan(-slope3 * SLOPE_FACTOR) + PI_HALF;
	slope4 = slope4 > 99990.f ? 0.f : inverseTan(-slope4 * SLOPE_FACTOR) + PI_HALF;
	slope5 = slope5 > 99990.f ? 0.f : inverseTan(-slope5 * SLOPE_FACTOR) + PI_HALF;
	slope6 = slope6 > 99990.f ? 0.f : inverseTan(-slope6 * SLOPE_FACTOR) + PI_HALF;
	slope7 = slope7 > 99990.f ? 0.f : inverseTan(-slope7 * SLOPE_FACTOR) + PI_HALF;


	float sumSlopes = slope0
		+ slope1
		+ slope2
		+ slope3
		+ slope4
		+ slope5
		+ slope6
		+ slope7;

	slope0 = slope0 / sumSlopes;
	slope1 = slope1 / sumSlopes;
	slope2 = slope2 / sumSlopes;
	slope3 = slope3 / sumSlopes;
	slope4 = slope4 / sumSlopes;
	slope5 = slope5 / sumSlopes;
	slope6 = slope6 / sumSlopes;
	slope7 = slope7 / sumSlopes;

	slope1 = slope0 + slope1;
	slope2 = slope1 + slope2;
	slope3 = slope2 + slope3;
	slope4 = slope3 + slope4;
	slope5 = slope4 + slope5;
	slope6 = slope5 + slope6;
	slope7 = slope6 + slope7;	// is equal 1

	unsigned int moveDir = 0;
	if (randF < slope0)
		moveDir = 0;
	else if (randF < slope1)
		moveDir = 1;
	else if (randF < slope2)
		moveDir = 2;
	else if (randF < slope3)
		moveDir = 3;
	else if (randF < slope4)
		moveDir = 4;
	else if (randF < slope5)
		moveDir = 5;
	else if (randF < slope6)
		moveDir = 6;
	else
		moveDir = 7;

	//moveDir = 1;

	//resultRain = resultRain / 2;
	
	if (moveDir == 0)
	{
		if (GID.x + 1 < width && GID.y < height)
			atomic_add(d_rain + (GID.x + 1) + width * (GID.y + 0), resultRain);
	}
	else if (moveDir == 1)
	{
		if (GID.x + 1 >= 0 && GID.x + 1 < width && GID.y + 1 >= 0 && GID.y + 1 < height)
			atomic_add(d_rain + (GID.x + 1) + width * (GID.y + 1), resultRain);
	}
	else if (moveDir == 2)
	{
		if (GID.x + 0 >= 0 && GID.x + 0 < width && GID.y + 1 >= 0 && GID.y + 1 < height)
			atomic_add(d_rain + (GID.x + 0) + width * (GID.y + 1), resultRain);
	}
	else if (moveDir == 3)
	{
		if (GID.x - 1 >= 0 && GID.x - 1 < width && GID.y + 1 >= 0 && GID.y + 1 < height)
			atomic_add(d_rain + (GID.x - 1) + width * (GID.y + 1), resultRain);
	}
	else if (moveDir == 4)
	{
		if (GID.x - 1 >= 0 && GID.x - 1 < width && GID.y + 0 >= 0 && GID.y + 0 < height)
			atomic_add(d_rain + (GID.x - 1) + width * (GID.y + 0), resultRain);
	}
	else if (moveDir == 5)
	{
		if (GID.x - 1 >= 0 && GID.x - 1 < width && GID.y - 1 >= 0 && GID.y - 1 < height)
			atomic_add(d_rain + (GID.x - 1) + width * (GID.y - 1), resultRain);
	}
	else if (moveDir == 6)
	{
		if (GID.x + 0 >= 0 && GID.x + 0 < width && GID.y - 1 >= 0 && GID.y - 1 < height)
			atomic_add(d_rain + (GID.x + 0) + width * (GID.y - 1), resultRain);
	}
	else if (moveDir == 7)
	{
		if (GID.x + 1 >= 0 && GID.x + 1 < width && GID.y - 1 >= 0 && GID.y - 1 < height)
			atomic_add(d_rain + (GID.x + 1) + width * (GID.y - 1), resultRain);
	}
		
	





	float4 x0 = d_pos[particleID];

	float lerpFac = 1.f;
	if (x0.y < 1.f)
	{
		lerpFac = x0.y;
	}

	x0.y = x0.y - RAIN_IMPACT * lerpFac* (rainHere > 0 ? 1.f : 0.f) * DELTA_T;


	x0.w = rainHere > 3 ? 100.f : 0.f;



	d_pos[particleID] = x0;

}

/*float Gauss2D(float x, float y, float uX, float uY, float variant)
{
	return (1.f / (2.f * CL_M_PI * variant * variant)) * exp(-1.f * ((x - uX) * (x - uX) + (y - uY) * (y - uY)) / (2.f * variant * variant));
}*/

float Abso(float x)
{
	if (x < 0.f)
	{
		return -x;
	}
	return x;
}

float RainProb(float4 pos, float4 normal)
{
	float prob = (pos.y - 0.07f) / 0.15f;
	if (prob < 0.f)
	{
		prob = 0.f;
	}
	prob *= prob;
	if (prob > 1.f)
	{
		prob = 1.f;
	}
	else if (prob < 0.f)
	{
		prob = 0.f;
	}

	//prob *= 0.7f;
	prob += ((inverseTan(0.01f * ((max(Abso(normal.x), Abso(normal.z)) / normal.y))) + 0.f) / (PI_HALF * 1.f));
	
	return prob;
}

__kernel void IntegrateRain(unsigned int width,
	unsigned int height,
	__global float4* d_pos,
	__global float4* d_prevPos,
	__global unsigned int* d_rain,
	__global unsigned int* d_newRain,
	float elapsedTime,
	float prevElapsedTime,
	float simulationTime,
	unsigned int randomSeedX,
	unsigned int randomSeedY)
{
	// Make sure the work-item does not map outside the cloth
	if (get_global_id(0) >= width || get_global_id(1) >= height)
		return;


	int2 GID;
	GID.x = get_global_id(0);
	GID.y = get_global_id(1);

	int2 LID;
	LID.x = get_local_id(0);
	LID.y = get_local_id(1);

	int2 LSIZE;
	LSIZE.x = get_local_size(0);
	LSIZE.y = get_local_size(1);





	__local float4 tile[TILE_Y + HALOSIZE][TILE_X + HALOSIZE];



	// Top side halo
	if (LID.y == 0)
	{
		if (GID.y == 0)
		{
			tile[0][LID.x + 1] = 0.f;
		}
		else
		{
			tile[0][LID.x + 1] = d_pos[(GID.y - 1) * width + GID.x];
		}
	}

	// Down side halo
	if (LID.y == LSIZE.y - 1)
	{
		if (GID.y == height - 1)
		{
			tile[TILE_Y + 1][LID.x + 1] = 0.f;
		}
		else
		{
			tile[TILE_Y + 1][LID.x + 1] = d_pos[(GID.y + 1) * width + GID.x];
		}
	}

	// Left side halo
	if (LID.x == 0)
	{
		if (GID.x == 0)
		{
			tile[LID.y + 1][0] = 0.f;
		}
		else
		{
			tile[LID.y + 1][0] = d_pos[(GID.y) * width + GID.x - 1];
		}
	}

	// Right side halo
	if (LID.x == LSIZE.x - 1)
	{
		if (GID.x == width - 1)
		{
			tile[LID.y + 1][TILE_X + 1] = 0.f;
		}
		else
		{
			tile[LID.y + 1][TILE_X + 1] = d_pos[(GID.y) * width + GID.x + 1];
		}
	}




	// Corners

	// Left top
	if (LID.x == 1 && LID.y == 1)
	{
		if (GID.x == 1 || GID.y == 1)
		{
			tile[0][0] = 0.f;
		}
		else
		{
			tile[0][0] = d_pos[(GID.y - 2) * width + GID.x - 2];
		}
	}

	// Right top
	if (LID.x == LSIZE.x - 2 && LID.y == 1)
	{
		if (GID.x == width - 2 || GID.y == 1)
		{
			tile[0][TILE_X + 1] = 0.f;
		}
		else
		{
			tile[0][TILE_X + 1] = d_pos[(GID.y - 2) * width + GID.x + 2];
		}
	}

	// Left bottom
	if (LID.x == 1 && LID.y == LSIZE.y - 2)
	{
		if (GID.x == 1 || GID.y == height - 2)
		{
			tile[TILE_Y + 1][0] = 0.f;
		}
		else
		{
			tile[TILE_Y + 1][0] = d_pos[(GID.y + 2) * width + GID.x - 2];
		}
	}

	// Right bottom
	if (LID.x == LSIZE.x - 2 && LID.y == LSIZE.y - 2)
	{
		if (GID.x == width - 1 || GID.y == height - 2)
		{
			tile[TILE_Y + 1][TILE_X + 1] = 0.f;
		}
		else
		{
			tile[TILE_Y + 1][TILE_X + 1] = d_pos[(GID.y + 2) * width + GID.x + 2];
		}
	}



	tile[LID.y + 1][LID.x + 1] = d_pos[(GID.y) * width + GID.x];


	barrier(CLK_LOCAL_MEM_FENCE);
















	unsigned int particleID = get_global_id(0) + get_global_id(1) * width;


	uint random = (randomSeedX * GID.x * 301267 + randomSeedY * GID.y * 2013) % 32767;

	float randF = (((float)random) / (32767.f)); //(((float)random) / (4294967296.f));


	if (GID.x > 0 && GID.x < width - 1 && GID.y > 0 && GID.y < height - 1)
	{
		float4 self = tile[LID.y + 1][LID.x + 1];
		float4 left = tile[LID.y + 1][LID.x];
		float4 right = tile[LID.y + 1][LID.x + 2];
		float4 up = tile[LID.y + 2][LID.x + 1];
		float4 down = tile[LID.y][LID.x + 1];


		float4 normal = (float4)(left.y - right.y, Abso(self.x - left.x), down.y - up.y, 0.f);

		if (randF < RainProb(self, normal))
		{
			d_rain[particleID] = d_rain[particleID] + RAIN_LIFETIME;
		}
	}



}


///////////////////////////////////////////////////////////////////////////////
// Input data:
// pos1 and pos2 - The positions of two particles
// restDistance  - the distance between the given particles at rest
//
// Return data:
// correction vector for particle 1
///////////////////////////////////////////////////////////////////////////////
  float4 SatisfyConstraint(float4 pos1,
						 float4 pos2,
						 float restDistance){
	float4 toNeighbor = pos2 - pos1;
	return (toNeighbor - normalize(toNeighbor) * restDistance);
}



float SatisfyContConstraint(float4 pos0,
	  float4 pos1,
	  float4 pos2,
	  float4 pos3,
	  float4 pos4)
{
	float hD0 = pos1.y - pos0.y;
	float hD1 = pos2.y - pos1.y;
	float hD2 = pos3.y - pos2.y;
	float hD3 = pos4.y - pos3.y;

	float an0 = hD1 - hD0;
	float an1 = hD2 - hD1;
	float an2 = hD3 - hD2;

	float mid = (an0 + an2) * 0.5f;

	return an1 - mid;
}


///////////////////////////////////////////////////////////////////////////////
// Input data:
// width and height - the dimensions of the particle grid
// restDistance     - the distance between two orthogonally neighboring particles at rest
// d_posIn          - the input positions
//
// Output data:
// d_posOut - new positions must be written here
///////////////////////////////////////////////////////////////////////////////


__kernel __attribute__((reqd_work_group_size(TILE_X, TILE_Y, 1)))
__kernel void SatisfyConstraints(unsigned int width,
								unsigned int height, 
								float restDistance,
								__global float4* d_posOut,
								__global float4 const * d_posIn){
    
    if(get_global_id(0) >= width || get_global_id(1) >= height)
		return;


	
	int2 GID;
	GID.x = get_global_id(0);
	GID.y = get_global_id(1);
	
	int2 LID;
	LID.x = get_local_id(0);
	LID.y = get_local_id(1);

	int2 LSIZE;
	LSIZE.x = get_local_size(0);
	LSIZE.y = get_local_size(1);


	

	
	__local float4 tile[TILE_Y + HALOSIZE * 2][TILE_X + HALOSIZE * 2];



	// Top side halo

	if (LID.y == 0)
	{
		if (GID.y == 0)
		{
			tile[0][LID.x + 2] = 0.f;
		}
		else
		{
			tile[0][LID.x + 2] = d_posIn[(GID.y - 2) * width + GID.x];
		}
	}
	if (LID.y == 1)
	{
		if (GID.y == 1)
		{
			tile[1][LID.x + 2] = 0.f;
		}
		else
		{
			tile[1][LID.x + 2] = d_posIn[(GID.y - 2) * width + GID.x];
		}
	}






	// Down side halo

	if (LID.y == LSIZE.y - 1)
	{
		if (GID.y == height - 1)
		{
			tile[TILE_Y + 3][LID.x + 2] = 0.f;
		}
		else
		{
			tile[TILE_Y + 3][LID.x + 2] = d_posIn[(GID.y + 2) * width + GID.x];
		}
	}
	if (LID.y == LSIZE.y - 2)
	{
		if (GID.y == height - 2)
		{
			tile[TILE_Y + 2][LID.x + 2] = 0.f;
		}
		else
		{
			tile[TILE_Y + 2][LID.x + 2] = d_posIn[(GID.y + 2) * width + GID.x];
		}
	}












	


	// Left side halo

	if (LID.x == 0)
	{
		if (GID.x == 0)
		{
			tile[LID.y + 2][0] = 0.f;
		}
		else
		{
			tile[LID.y + 2][0] = d_posIn[(GID.y) * width + GID.x - 2];
		}
	}
	if (LID.x == 1)
	{
		if (GID.x == 1)
		{
			tile[LID.y + 2][1] = 0.f;
		}
		else
		{
			tile[LID.y + 2][1] = d_posIn[(GID.y) * width + GID.x - 2];
		}
	}






	// Right side halo

	if (LID.x == LSIZE.x - 1)
	{
		if (GID.x == width - 1)
		{
			tile[LID.y + 2][TILE_X + 3] = 0.f;
		}
		else
		{
			tile[LID.y + 2][TILE_X + 3] = d_posIn[(GID.y) * width + GID.x + 2];
		}
	}
	if (LID.x == LSIZE.x - 2)
	{
		if (GID.x == width - 2)
		{
			tile[LID.y + 2][TILE_X + 2] = 0.f;
		}
		else
		{
			tile[LID.y + 2][TILE_X + 2] = d_posIn[(GID.y) * width + GID.x + 2];
		}
	}
	
	












	

	// Corners

	// Left top
	if (LID.x == 2 && LID.y == 2)
	{
		if (GID.x == 2 || GID.y == 2)
		{
			tile[0][0] = 0.f;
		}
		else
		{
			tile[0][0] = d_posIn[(GID.y - 4) * width + GID.x - 4];
		}
	}
	if (LID.x == 3 && LID.y == 2)
	{
		if (GID.x == 3 || GID.y == 2)
		{
			tile[0][1] = 0.f;
		}
		else
		{
			tile[0][1] = d_posIn[(GID.y - 4) * width + GID.x - 4];
		}
	}
	if (LID.x == 2 && LID.y == 3)
	{
		if (GID.x == 2 || GID.y == 3)
		{
			tile[1][0] = 0.f;
		}
		else
		{
			tile[1][0] = d_posIn[(GID.y - 4) * width + GID.x - 4];
		}
	}
	if (LID.x == 3 && LID.y == 3)
	{
		if (GID.x == 3 || GID.y == 3)
		{
			tile[1][1] = 0.f;
		}
		else
		{
			tile[1][1] = d_posIn[(GID.y - 4) * width + GID.x - 4];
		}
	}



	// Right top
	if (LID.x == LSIZE.x - 3 && LID.y == 2)
	{
		if (GID.x == width - 3 || GID.y == 2)
		{
			tile[0][TILE_X + 3] = 0.f;
		}
		else
		{
			tile[0][TILE_X + 3] = d_posIn[(GID.y - 4) * width + GID.x + 4];
		}
	}
	if (LID.x == LSIZE.x - 4 && LID.y == 2)
	{
		if (GID.x == width - 4 || GID.y == 2)
		{
			tile[0][TILE_X + 2] = 0.f;
		}
		else
		{
			tile[0][TILE_X + 2] = d_posIn[(GID.y - 4) * width + GID.x + 4];
		}
	}
	if (LID.x == LSIZE.x - 3 && LID.y == 3)
	{
		if (GID.x == width - 3 || GID.y == 3)
		{
			tile[1][TILE_X + 3] = 0.f;
		}
		else
		{
			tile[1][TILE_X + 3] = d_posIn[(GID.y - 4) * width + GID.x + 4];
		}
	}
	if (LID.x == LSIZE.x - 4 && LID.y == 3)
	{
		if (GID.x == width - 4 || GID.y == 3)
		{
			tile[1][TILE_X + 2] = 0.f;
		}
		else
		{
			tile[1][TILE_X + 2] = d_posIn[(GID.y - 4) * width + GID.x + 4];
		}
	}



	// Left bottom
	if (LID.x == 2 && LID.y == LSIZE.y - 3)
	{
		if (GID.x == 2 || GID.y == height - 3)
		{
			tile[TILE_Y + 3][0] = 0.f;
		}
		else
		{
			tile[TILE_Y + 3][0] = d_posIn[(GID.y + 4) * width + GID.x - 4];
		}
	}
	if (LID.x == 3 && LID.y == LSIZE.y - 3)
	{
		if (GID.x == 3 || GID.y == height - 3)
		{
			tile[TILE_Y + 3][1] = 0.f;
		}
		else
		{
			tile[TILE_Y + 3][1] = d_posIn[(GID.y + 4) * width + GID.x - 4];
		}
	}
	if (LID.x == 2 && LID.y == LSIZE.y - 4)
	{
		if (GID.x == 2 || GID.y == height - 4)
		{
			tile[TILE_Y + 2][0] = 0.f;
		}
		else
		{
			tile[TILE_Y + 2][0] = d_posIn[(GID.y + 4) * width + GID.x - 4];
		}
	}
	if (LID.x == 3 && LID.y == LSIZE.y - 4)
	{
		if (GID.x == 3 || GID.y == height - 4)
		{
			tile[TILE_Y + 2][1] = 0.f;
		}
		else
		{
			tile[TILE_Y + 2][1] = d_posIn[(GID.y + 4) * width + GID.x - 4];
		}
	}



	// Right bottom
	if (LID.x == LSIZE.x - 3 && LID.y == LSIZE.y - 3)
	{
		if (GID.x == width - 3 || GID.y == height - 3)
		{
			tile[TILE_Y + 3][TILE_X + 3] = 0.f;
		}
		else
		{
			tile[TILE_Y + 3][TILE_X + 3] = d_posIn[(GID.y + 4) * width + GID.x + 4];
		}
	}
	if (LID.x == LSIZE.x - 4 && LID.y == LSIZE.y - 3)
	{
		if (GID.x == width - 4 || GID.y == height - 3)
		{
			tile[TILE_Y + 3][TILE_X + 2] = 0.f;
		}
		else
		{
			tile[TILE_Y + 3][TILE_X + 2] = d_posIn[(GID.y + 4) * width + GID.x + 4];
		}
	}
	if (LID.x == LSIZE.x - 3 && LID.y == LSIZE.y - 4)
	{
		if (GID.x == width - 3 || GID.y == height - 4)
		{
			tile[TILE_Y + 2][TILE_X + 3] = 0.f;
		}
		else
		{
			tile[TILE_Y + 2][TILE_X + 3] = d_posIn[(GID.y + 4) * width + GID.x + 4];
		}
	}
	if (LID.x == LSIZE.x - 4 && LID.y == LSIZE.y - 4)
	{
		if (GID.x == width - 4 || GID.y == height - 4)
		{
			tile[TILE_Y + 2][TILE_X + 2] = 0.f;
		}
		else
		{
			tile[TILE_Y + 2][TILE_X + 2] = d_posIn[(GID.y + 4) * width + GID.x + 4];
		}
	}




	tile[LID.y + 2][LID.x + 2] = d_posIn[(GID.y) * width + GID.x];

	
	barrier(CLK_LOCAL_MEM_FENCE);
	




	unsigned int partID = get_global_id(0) + get_global_id(1) * width;
	// This is just to keep every 8th particle of the first row attached to the bar
    

		float corVecSum = 0.f;

		
		if (GID.y >= 2 && GID.y <= height - 3)
		{
			corVecSum += SatisfyContConstraint(tile[LID.y + 2 - 2][LID.x + 2], tile[LID.y + 2 - 1][LID.x + 2], tile[LID.y + 2][LID.x + 2], tile[LID.y + 2 + 1][LID.x + 2], tile[LID.y + 2 + 2][LID.x + 2]) * WEIGHT_SMOOTH_ORTO;
		}
		if (GID.x >= 2 && GID.x <= height - 3)
		{
			corVecSum += SatisfyContConstraint(tile[LID.y + 2][LID.x + 2 - 2], tile[LID.y + 2][LID.x + 2 - 1], tile[LID.y + 2][LID.x + 2], tile[LID.y + 2][LID.x + 2 + 1], tile[LID.y + 2][LID.x + 2 + 2]) * WEIGHT_SMOOTH_ORTO;
		}

		if (GID.y >= 2 && GID.x >= 2 && GID.y <= height - 3 && GID.x <= height - 3)
		{
			corVecSum += SatisfyContConstraint(tile[LID.y + 2 - 2][LID.x + 2 - 2], tile[LID.y + 2 - 1][LID.x + 2 - 1], tile[LID.y + 2][LID.x + 2], tile[LID.y + 2 + 1][LID.x + 2 + 1], tile[LID.y + 2 + 2][LID.x + 2 + 2]) * WEIGHT_SMOOTH_DIAG;
			corVecSum += SatisfyContConstraint(tile[LID.y + 2 + 2][LID.x + 2 - 2], tile[LID.y + 2 + 1][LID.x + 2 - 1], tile[LID.y + 2][LID.x + 2], tile[LID.y + 2 - 1][LID.x + 2 + 1], tile[LID.y + 2 - 2][LID.x + 2 + 2]) * WEIGHT_SMOOTH_DIAG;
		}
		

		tile[LID.y + 2][LID.x + 2].y += corVecSum;
		d_posOut[partID] = tile[LID.y + 2][LID.x + 2];


	





	

	

}


///////////////////////////////////////////////////////////////////////////////
// There is no need to change this function!
///////////////////////////////////////////////////////////////////////////////
float4 CalcTriangleNormal( float4 p1, float4 p2, float4 p3) {
    float4 v1 = p2-p1;
    float4 v2 = p3-p1;

    return cross( v1, v2);
}

///////////////////////////////////////////////////////////////////////////////
// There is no need to change this kernel!
///////////////////////////////////////////////////////////////////////////////
__kernel void ComputeNormals(unsigned int width,
								unsigned int height, 
								__global float4* d_pos,
								__global float4* d_normal){
								
    int particleID = get_global_id(0) + get_global_id(1) * width;
    float4 normal = (float4)( 0.0f, 0.0f, 0.0f, 0.0f);
    
    int minX, maxX, minY, maxY, cntX, cntY;
    minX = max( (int)(0), (int)(get_global_id(0)-1));
    maxX = min( (int)(width-1), (int)(get_global_id(0)+1));
    minY = max( (int)(0), (int)(get_global_id(1)-1));
    maxY = min( (int)(height-1), (int)(get_global_id(1)+1));
    
    for( cntX = minX; cntX < maxX; ++cntX) {
        for( cntY = minY; cntY < maxY; ++cntY) {
            normal += normalize( CalcTriangleNormal(
                d_pos[(cntX+1)+width*(cntY)],
                d_pos[(cntX)+width*(cntY)],
                d_pos[(cntX)+width*(cntY+1)]));
            normal += normalize( CalcTriangleNormal(
                d_pos[(cntX+1)+width*(cntY+1)],
                d_pos[(cntX+1)+width*(cntY)],
                d_pos[(cntX)+width*(cntY+1)]));
        }
    }
    d_normal[particleID] = normalize( normal);
}
