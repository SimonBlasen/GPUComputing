#define DAMPING 0.02f

#define G_ACCEL (float4)(0.f, -9.81f, 0.f, 0.f)

#define WEIGHT_ORTHO	0.138f
#define WEIGHT_DIAG		0.097f
#define WEIGHT_ORTHO_2	0.069f
#define WEIGHT_DIAG_2	0.048f


#define ROOT_OF_2 1.4142135f
#define DOUBLE_ROOT_OF_2 2.8284271f




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
						float elapsedTime,
						float prevElapsedTime,
						float simulationTime) {
							
	// Make sure the work-item does not map outside the cloth
    if(get_global_id(0) >= width || get_global_id(1) >= height)
		return;





	//elapsedTime *= 0.04f;




	unsigned int particleID = get_global_id(0) + get_global_id(1) * width;
	// This is just to keep every 8th particle of the first row attached to the bar
    if(particleID > width-1 || ( particleID & ( 7 )) != 0){

		float4 windA = (float4) (sin(simulationTime * 1.1f) * 2.f, 0.f, sin(simulationTime * 1.0f) * 5.f, 0.f);

		// ADD YOUR CODE HERE!

		// Read the positions
		// Compute the new one position using the Verlet position integration, taking into account gravity and wind
		// Move the value from d_pos into d_prevPos and store the new one in d_pos


		float4 x0 = d_prevPos[particleID];
		float4 x1 = d_pos[particleID];

		float4 v1 = (x1 - x0);
		v1.x /= elapsedTime;
		v1.y /= elapsedTime;
		v1.z /= elapsedTime;
		if (v1.y < 100.f && v1.y > -100.f)
		{
			float4 a1 = windA + G_ACCEL;

			float4 x2 = x1 + v1 * elapsedTime + 0.5f * a1 * elapsedTime * elapsedTime;

			x2.w = 0.f;

			d_pos[particleID] = x2; //x1 + ((float4)(0.f, -0.001f, 0.f, 0.f));
		}
		x1.w = 0.f;
		d_prevPos[particleID] = x1;

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

///////////////////////////////////////////////////////////////////////////////
// Input data:
// width and height - the dimensions of the particle grid
// restDistance     - the distance between two orthogonally neighboring particles at rest
// d_posIn          - the input positions
//
// Output data:
// d_posOut - new positions must be written here
///////////////////////////////////////////////////////////////////////////////

#define TILE_X 16 
#define TILE_Y 16
#define HALOSIZE 2

__kernel __attribute__((reqd_work_group_size(TILE_X, TILE_Y, 1)))
__kernel void SatisfyConstraints(unsigned int width,
								unsigned int height, 
								float restDistance,
								__global float4* d_posOut,
								__global float4 const * d_posIn){
    
    if(get_global_id(0) >= width || get_global_id(1) >= height)
		return;


	// ADD YOUR CODE HERE!
	// Satisfy all the constraints (structural, shear, and bend).
	// You can use weights defined at the beginning of this file.

	// A ping-pong scheme is needed here, so read the values from d_posIn and store the results in d_posOut

	// Hint: you should use the SatisfyConstraint helper function in the following manner:
	//SatisfyConstraint(pos, neighborpos, restDistance) * WEIGHT_XXX


	/*



	unsigned int partID = get_global_id(0) + get_global_id(1) * width;
	// This is just to keep every 8th particle of the first row attached to the bar
    if(partID > width-1 || ( partID & ( 7 )) != 0){

	

	
		int2 GID;
		GID.x = get_global_id(0);
		GID.y = get_global_id(1);
		
		int2 LID;
		LID.x = get_local_id(0);
		LID.y = get_local_id(1);

		int2 LSIZE;
		LSIZE.x = get_local_size(0);
		LSIZE.y = get_local_size(1);



		float4 corVecSum = (float4)(0.f, 0.f, 0.f, 0.f);

		//uint partID = GID.y * width + GID.x;
		
		if (GID.y >= 1)
		{
			corVecSum += SatisfyConstraint(d_posIn[partID], d_posIn[(GID.y - 1) * width + GID.x], restDistance) * WEIGHT_ORTHO;
		}
		if (GID.y <= height - 2)
		{
			corVecSum += SatisfyConstraint(d_posIn[partID], d_posIn[(GID.y + 1) * width + GID.x], restDistance) * WEIGHT_ORTHO;
		}
		if (GID.x >= 1)
		{
			corVecSum += SatisfyConstraint(d_posIn[partID], d_posIn[(GID.y) * width + GID.x - 1], restDistance) * WEIGHT_ORTHO;
		}
		if (GID.x <= width - 2)
		{
			corVecSum += SatisfyConstraint(d_posIn[partID], d_posIn[(GID.y) * width + GID.x + 1], restDistance) * WEIGHT_ORTHO;
		}
		
		

		

		if (GID.y >= 1 && GID.x >= 1)
		{
			corVecSum += SatisfyConstraint(d_posIn[partID], d_posIn[(GID.y - 1) * width + GID.x - 1], restDistance * ROOT_OF_2) * WEIGHT_DIAG;
		}
		if (GID.y <= height - 2 && GID.x >= 1)
		{
			corVecSum += SatisfyConstraint(d_posIn[partID], d_posIn[(GID.y + 1) * width + GID.x - 1], restDistance * ROOT_OF_2) * WEIGHT_DIAG;
		}
		if (GID.x <= width - 2 && GID.y >= 1)
		{
			corVecSum += SatisfyConstraint(d_posIn[partID], d_posIn[(GID.y - 1) * width + GID.x + 1], restDistance * ROOT_OF_2) * WEIGHT_DIAG;
		}
		if (GID.x <= width - 2 && GID.y <= height - 2)
		{
			corVecSum += SatisfyConstraint(d_posIn[partID], d_posIn[(GID.y + 1) * width + GID.x + 1], restDistance * ROOT_OF_2) * WEIGHT_DIAG;
		}


		

		if (GID.y >= 2 && GID.x >= 2)
		{
			corVecSum += SatisfyConstraint(d_posIn[partID], d_posIn[(GID.y - 2) * width + GID.x - 2], restDistance * DOUBLE_ROOT_OF_2) * WEIGHT_DIAG_2;
		}
		if (GID.y <= height - 3 && GID.x >= 2)
		{
			corVecSum += SatisfyConstraint(d_posIn[partID], d_posIn[(GID.y + 2) * width + GID.x - 2], restDistance * DOUBLE_ROOT_OF_2) * WEIGHT_DIAG_2;
		}
		if (GID.x <= width - 3 && GID.y >= 2)
		{
			corVecSum += SatisfyConstraint(d_posIn[partID], d_posIn[(GID.y - 2) * width + GID.x + 2], restDistance * DOUBLE_ROOT_OF_2) * WEIGHT_DIAG_2;
		}
		if (GID.x <= width - 3 && GID.y <= height - 3)
		{
			corVecSum += SatisfyConstraint(d_posIn[partID], d_posIn[(GID.y + 2) * width + GID.x + 2], restDistance * DOUBLE_ROOT_OF_2) * WEIGHT_DIAG_2;
		}
		

		corVecSum.w = 0.f;








		//if (corVecSum.x * corVecSum.x + corVecSum.y * corVecSum.y + corVecSum.z * corVecSum.z > 0.01f * ((restDistance * 0.5f) * (restDistance * 0.5f)))
		if (length(corVecSum) > (restDistance / 2.f))
		{
			corVecSum = normalize(corVecSum) * (restDistance / 2.f);
		}

		float4 posin = d_posIn[partID];
		posin.y -= 0.00002f;

		d_posOut[partID] = d_posIn[partID] + corVecSum;


	}
	else
	{
		d_posOut[partID] = d_posIn[partID];
	}




	*/



	
	

	
		int2 GID;
		GID.x = get_global_id(0);
		GID.y = get_global_id(1);
		
		int2 LID;
		LID.x = get_local_id(0);
		LID.y = get_local_id(1);

		int2 LSIZE;
		LSIZE.x = get_local_size(0);
		LSIZE.y = get_local_size(1);


	

	
	__local float4 tile[TILE_X + HALOSIZE * 2][TILE_Y + HALOSIZE * 2];



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
			tile[1][LID.x + 2] = d_posIn[(GID.y - 1) * width + GID.x];
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
			tile[TILE_Y + 2][LID.x + 2] = d_posIn[(GID.y + 1) * width + GID.x];
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
			tile[LID.y + 2][1] = d_posIn[(GID.y) * width + GID.x - 1];
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
			tile[LID.y + 2][TILE_X + 2] = d_posIn[(GID.y) * width + GID.x + 1];
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
			tile[0][0] = d_posIn[(GID.y - 2) * width + GID.x - 2];
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
			tile[0][1] = d_posIn[(GID.y - 2) * width + GID.x - 2];
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
			tile[1][0] = d_posIn[(GID.y - 2) * width + GID.x - 2];
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
			tile[1][1] = d_posIn[(GID.y - 2) * width + GID.x - 2];
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
			tile[0][TILE_X + 3] = d_posIn[(GID.y - 2) * width + GID.x + 2];
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
			tile[0][TILE_X + 2] = d_posIn[(GID.y - 2) * width + GID.x + 2];
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
			tile[1][TILE_X + 3] = d_posIn[(GID.y - 2) * width + GID.x + 2];
		}
	}
	if (LID.x == LSIZE.x - 4 && LID.y == 3)
	{
		if (GID.x == width - 4 || GID.y == 3)
		{
			tile[1][TILE_X + 4] = 0.f;
		}
		else
		{
			tile[1][TILE_X + 4] = d_posIn[(GID.y - 2) * width + GID.x + 2];
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
			tile[TILE_Y + 3][0] = d_posIn[(GID.y + 2) * width + GID.x - 2];
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
			tile[TILE_Y + 3][1] = d_posIn[(GID.y + 2) * width + GID.x - 2];
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
			tile[TILE_Y + 2][0] = d_posIn[(GID.y + 2) * width + GID.x - 2];
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
			tile[TILE_Y + 2][1] = d_posIn[(GID.y + 2) * width + GID.x - 2];
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
			tile[TILE_Y + 3][TILE_X + 3] = d_posIn[(GID.y + 2) * width + GID.x + 2];
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
			tile[TILE_Y + 3][TILE_X + 2] = d_posIn[(GID.y + 2) * width + GID.x + 2];
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
			tile[TILE_Y + 2][TILE_X + 3] = d_posIn[(GID.y + 2) * width + GID.x + 2];
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
			tile[TILE_Y + 2][TILE_X + 2] = d_posIn[(GID.y + 2) * width + GID.x + 2];
		}
	}












	unsigned int partID = get_global_id(0) + get_global_id(1) * width;
	// This is just to keep every 8th particle of the first row attached to the bar
    if(partID > width-1 || ( partID & ( 7 )) != 0){




		float4 corVecSum = (float4)(0.f, 0.f, 0.f, 0.f);

		//uint partID = GID.y * width + GID.x;
		
		if (GID.y >= 1)
		{
			corVecSum += SatisfyConstraint(tile[LID.y][LID.x], tile[LID.y - 1][LID.x], restDistance) * WEIGHT_ORTHO;
		}
		if (GID.y <= height - 2)
		{
			corVecSum += SatisfyConstraint(tile[LID.y][LID.x], tile[LID.y + 1][LID.x], restDistance) * WEIGHT_ORTHO;
		}
		if (GID.x >= 1)
		{
			corVecSum += SatisfyConstraint(tile[LID.y][LID.x], tile[LID.y][LID.x - 1], restDistance) * WEIGHT_ORTHO;
		}
		if (GID.x <= width - 2)
		{
			corVecSum += SatisfyConstraint(tile[LID.y][LID.x], tile[LID.y][LID.x + 1], restDistance) * WEIGHT_ORTHO;
		}
		
		

		

		if (GID.y >= 1 && GID.x >= 1)
		{
			corVecSum += SatisfyConstraint(tile[LID.y][LID.x], tile[LID.y - 1][LID.x - 1], restDistance * ROOT_OF_2) * WEIGHT_DIAG;
		}
		if (GID.y <= height - 2 && GID.x >= 1)
		{
			corVecSum += SatisfyConstraint(tile[LID.y][LID.x], tile[LID.y + 1][LID.x - 1], restDistance * ROOT_OF_2) * WEIGHT_DIAG;
		}
		if (GID.x <= width - 2 && GID.y >= 1)
		{
			corVecSum += SatisfyConstraint(tile[LID.y][LID.x], tile[LID.y - 1][LID.x + 1], restDistance * ROOT_OF_2) * WEIGHT_DIAG;
		}
		if (GID.x <= width - 2 && GID.y <= height - 2)
		{
			corVecSum += SatisfyConstraint(tile[LID.y][LID.x], tile[LID.y + 1][LID.x + 1], restDistance * ROOT_OF_2) * WEIGHT_DIAG;
		}


		

		if (GID.y >= 2 && GID.x >= 2)
		{
			corVecSum += SatisfyConstraint(tile[LID.y][LID.x], tile[LID.y - 2][LID.x - 2], restDistance * DOUBLE_ROOT_OF_2) * WEIGHT_DIAG_2;
		}
		if (GID.y <= height - 3 && GID.x >= 2)
		{
			corVecSum += SatisfyConstraint(tile[LID.y][LID.x], tile[LID.y + 2][LID.x - 2], restDistance * DOUBLE_ROOT_OF_2) * WEIGHT_DIAG_2;
		}
		if (GID.x <= width - 3 && GID.y >= 2)
		{
			corVecSum += SatisfyConstraint(tile[LID.y][LID.x], tile[LID.y - 2][LID.x + 2], restDistance * DOUBLE_ROOT_OF_2) * WEIGHT_DIAG_2;
		}
		if (GID.x <= width - 3 && GID.y <= height - 3)
		{
			corVecSum += SatisfyConstraint(tile[LID.y][LID.x], tile[LID.y + 2][LID.x + 2], restDistance * DOUBLE_ROOT_OF_2) * WEIGHT_DIAG_2;
		}
		

		corVecSum.w = 0.f;








		//if (corVecSum.x * corVecSum.x + corVecSum.y * corVecSum.y + corVecSum.z * corVecSum.z > 0.01f * ((restDistance * 0.5f) * (restDistance * 0.5f)))
		if (length(corVecSum) > (restDistance / 2.f))
		{
			corVecSum = normalize(corVecSum) * (restDistance / 2.f);
		}

		d_posOut[partID] = tile[LID.y][LID.x] + corVecSum;


	}
	else
	{
		d_posOut[partID] = tile[LID.y][LID.x];
	}








	

}


///////////////////////////////////////////////////////////////////////////////
// Input data:
// width and height - the dimensions of the particle grid
// d_pos            - the input positions
// spherePos        - The position of the sphere (xyz)
// sphereRad        - The radius of the sphere
//
// Output data:
// d_pos            - The updated positions
///////////////////////////////////////////////////////////////////////////////
__kernel void CheckCollisions(unsigned int width,
								unsigned int height, 
								__global float4* d_pos,
								float4 spherePos,
								float sphereRad){
								

	// ADD YOUR CODE HERE!
	// Find whether the particle is inside the sphere.
	// If so, push it outside.
	
    if(get_global_id(0) >= width || get_global_id(1) >= height)
		return;

		
	unsigned int partID = get_global_id(0) + get_global_id(1) * width;
	// This is just to keep every 8th particle of the first row attached to the bar
    if(partID > width-1 || ( partID & ( 7 )) != 0)
	{
		spherePos.w = 0.f;
		float4 vecToMid = d_pos[partID] - spherePos;
		float len = length(vecToMid);
		if (len < sphereRad)
		{
			d_pos[partID] += normalize(vecToMid) * (sphereRad - len) * 0.5f;
		}
	}

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
