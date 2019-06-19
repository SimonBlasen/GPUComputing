

float4 cross3(float4 a, float4 b){
	float4 c;
	c.x = a.y * b.z - b.y * a.z;
	c.y = a.z * b.x - b.z * a.x;
	c.z = a.x * b.y - b.x * a.y;
	c.w = 0.f;
	return c;
}

float dot3(float4 a, float4 b){
	return a.x*b.x + a.y*b.y + a.z*b.z;
}



#define SPLIT_VELOCITY 3.5f
#define BOUNCE_OFFSET 0.02f
#define PARTICLE_START_LIFETIME 500.0f

#define EPSILON 0.001f

// This function expects two points defining a ray (x0 and x1)
// and three vertices stored in v1, v2, and v3 (the last component is not used)
// it returns true if an intersection is found and sets the isectT and isectN
// with the intersection ray parameter and the normal at the intersection point.
bool LineTriangleIntersection(	float4 x0, float4 x1,
								float4 v1, float4 v2, float4 v3,
								float *isectT, float4 *isectN){

	float4 dir = x1 - x0;
	dir.w = 0.f;

	float4 e1 = v2 - v1;
	float4 e2 = v3 - v1;
	e1.w = 0.f;
	e2.w = 0.f;

	float4 s1 = cross3(dir, e2);
	float divisor = dot3(s1, e1);
	if (divisor == 0.f)
		return false;
	float invDivisor = 1.f / divisor;

	// Compute first barycentric coordinate
	float4 d = x0 - v1;
	float b1 = dot3(d, s1) * invDivisor;
	if (b1 < -EPSILON || b1 > 1.f + EPSILON)
		return false;

	// Compute second barycentric coordinate
	float4 s2 = cross3(d, e1);
	float b2 = dot3(dir, s2) * invDivisor;
	if (b2 < -EPSILON || b1 + b2 > 1.f + EPSILON)
		return false;

	// Compute _t_ to intersection point
	float t = dot3(e2, s2) * invDivisor;
	if (t < -EPSILON || t > 1.f + EPSILON)
		return false;

	// Store the closest found intersection so far
	*isectT = t;
	*isectN = cross3(e1, e2);
	*isectN = normalize(*isectN);
	return true;

}


bool CheckCollisions(	float4 x0, float4 x1,
						__global float4 *gTriangleSoup, 
						__local float4* lTriangleCache,	// The cache should hold as many vertices as the number of threads (therefore the number of triangles is nThreads/3)
						uint nTriangles,
						float  *t,
						float4 *n,
						int LSIZE,
						int LID){


	// ADD YOUR CODE HERE

	// Each vertex of a triangle is stored as a float4, the last component is not used.
	// gTriangleSoup contains vertices of triangles in the following layout:
	// --------------------------------------------------------------
	// | t0_v0 | t0_v1 | t0_v2 | t1_v0 | t1_v1 | t1_v2 | t2_v0 | ...
	// --------------------------------------------------------------

	// First check collisions loading the triangles from the global memory.
	// Iterate over all triangles, load the vertices and call the LineTriangleIntersection test 
	// for each triangle to find the closest intersection. 

	// Notice that each thread has to read vertices of all triangles.
	// As an optimization you should implement caching of the triangles in the local memory.
	// The cache should hold as many float4 vertices as the number of threads.
	// In other words, each thread loads (at most) *one vertex*.
	// Consequently, the number of triangles in the cache will be nThreads/4.
	// Notice that if there are many triangles (e.g. CubeMonkey.obj), not all
	// triangles can fit into the cache at once. 
	// Therefore, you have to load the triangles in chunks, until you process them all.

	// The caching implementation should roughly follow this scheme:
	//uint nProcessed = 0;  
	//while (nProcessed < nTriangles) {
	//	Load a 'k' triangles in to the cache
	//	Iterate over the triangles in the cache and test for the intersection
	//	nProcessed += k;  
	//}

	
	
	bool didIntersec = false;
	float minT = 2.f;
	float4 minN;
	float tIsec;
	float4 nIsec;



	int doneTrias = 0;
	while (doneTrias < nTriangles)
	{
		if (LID + doneTrias * 3 < nTriangles * 3)
		{
			lTriangleCache[LID] = gTriangleSoup[LID + doneTrias * 3];
		}
		
		barrier(CLK_LOCAL_MEM_FENCE);

		int arr = 0;
		while (arr * 3 < LSIZE)
		{
			if (arr + doneTrias < nTriangles && LineTriangleIntersection(x0, x1, lTriangleCache[arr * 3], lTriangleCache[arr * 3 + 1], lTriangleCache[arr * 3 + 2], &tIsec, &nIsec))
			{
				if (tIsec < minT)
				{
					didIntersec = true;
					minT = tIsec;
					minN = nIsec;
					//nIsec = nIsec;
				}
			}
			arr++;
		}
		
		barrier(CLK_LOCAL_MEM_FENCE);

		doneTrias += arr;
	}

	*t = minT;
	*n = minN;

	return didIntersec;

	//return true;
}
								



//////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////
// This is the integration kernel. Implement the missing functionality
//
// Input data:
// gAlive         - Field of flag indicating whether the particle with that index is alive (!= 0) or dead (0). You will have to modify this
// gForceField    - 3D texture with the  force field
// sampler        - 3D texture sampler for the force field (see usage below)
// nParticles     - Number of input particles
// nTriangles     - Number of triangles in the scene (for collision detection)
// lTriangleCache - Local memory cache to be used during collision detection for the triangles
// gTriangleSoup  - The triangles in the scene (layout see the description of CheckCollisions())
// gPosLife       - Position (xyz) and remaining lifetime (w) of a particle
// gVelMass       - Velocity vector (xyz) and the mass (w) of a particle
// dT             - The timestep for the integration (the has to be subtracted from the remaining lifetime of each particle)
//
// Output data:
// gAlive   - Updated alive flags
// gPosLife - Updated position and lifetime
// gVelMass - Updated position and mass
//////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////
__kernel void Integrate(__global uint *gAlive,
						__read_only image3d_t gForceField, 
						sampler_t sampler,
						uint nParticles,
						uint nTriangles,
						__local float4 *lTriangleCache,
						__global float4 *gTriangleSoup,
						__global float4 *gPosLife, 
						__global float4 *gVelMass,
						float dT
						)  {

						
	int GID = get_global_id(0);
	
	int LID = get_local_id(0);

	int LSIZE = get_local_size(0);



	float4 gAccel = (float4)(0.f, -9.81f, 0.f, 0.f);

	// Verlet Velocity Integration
	float4 x0 = gPosLife[get_global_id(0)];
	float4 v0 = gVelMass[get_global_id(0)];
	float4 x1 = x0;

	float mass = v0.w;
	float life = x0.w;

	float4 lookUp = x0;
	lookUp.w = 0.f;

	float4 F0 = read_imagef(gForceField, sampler, lookUp);
	//F0 = (float4) (0,0,0,0);


	// ADD YOUR CODE HERE (INSTEAD OF THE LINES BELOW) 
	// to finish the implementation of the Verlet Velocity Integration

	float4 a0 = (float4)(gAccel.x + (F0.x / v0.w), gAccel.y + (F0.y / v0.w), gAccel.z + (F0.z / v0.w), 0.f);

	x1.x = x0.x + v0.x * dT + 0.5f * (a0.x) * dT * dT;
	x1.y = x0.y + v0.y * dT + 0.5f * (a0.y) * dT * dT;
	x1.z = x0.z + v0.z * dT + 0.5f * (a0.z) * dT * dT;
	
	float4 lookUpT1 = x1;
	float4 F1 = read_imagef(gForceField, sampler, lookUpT1);
	//F1 = (float4) (0,0,0,0);
	
	float4 a1 = (float4)(gAccel.x + (F1.x / v0.w), gAccel.y + (F1.y / v0.w), gAccel.z + (F1.z / v0.w), 0.f);

	v0.x = v0.x + 0.5f * (a0.x + a1.x) * dT;
	v0.y = v0.y + 0.5f * (a0.y + a1.y) * dT;
	v0.z = v0.z + 0.5f * (a0.z + a1.z) * dT;
	

	
	
	
	// Check for collisions and correct the position and velocity of the particle if it collides with a triangle
	// - Don't forget to offset the particles from the surface a little bit, otherwise they might get stuck in it.
	// - Dampen the velocity (e.g. by a factor of 0.7) to simulate dissipation of the energy.
	
	float t;
	float4 n;

	if (CheckCollisions(x0, x1, gTriangleSoup, lTriangleCache, nTriangles, &t, &n, LSIZE, LID))
	{
		float4 ray = x1 - x0;
		ray.w = 0.f;
		ray = normalize(ray);

		float4 newVel = v0 - 2.f * (dot3(v0, n)) * n;
		newVel.w = v0.w;
		newVel.x *= 0.7f;
		newVel.y *= 0.7f;
		newVel.z *= 0.7f;

		v0 = newVel;
		x1 = (x1 - x0) * t + x0;



		x1 = x1 - BOUNCE_OFFSET * ray;
		
	}

	












	
	// Kill the particle if its life is < 0.0 by setting the corresponding flag in gAlive to 0.

	// Independently of the status of the particle, possibly create a new one.
	// For instance, if the particle gets too fast (or too high, or passes through some region), it is split into two...

	
	x1.w = life - dT;

	if (x1.w <= 0.f)
	{
		gAlive[GID] = 0;
	}
	else if (gAlive[GID] != 1)
	{
		gAlive[GID] = 1;
		gAlive[GID + nParticles] = 0;
	}

	if (v0.x * v0.x + v0.y * v0.y + v0.z * v0.z >= SPLIT_VELOCITY * SPLIT_VELOCITY)
	{
		v0.x *= 0.5f;
		v0.y *= 0.5f;
		v0.z *= 0.5f;

		float4 newX1 = x1;
		newX1.w = PARTICLE_START_LIFETIME;

		gAlive[GID + nParticles] = 1;
		gPosLife[get_global_id(0) + nParticles] = newX1;
		gVelMass[get_global_id(0) + nParticles] = v0;
	}

	
	gPosLife[get_global_id(0)] = x1;
	

	gVelMass[get_global_id(0)] = v0;
}


//////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////
__kernel void Clear(__global float4* gPosLife, __global float4* gVelMass) {
	uint GID = get_global_id(0);
	gPosLife[GID] = 0.f;
	gVelMass[GID] = 0.f;
}


//////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////
__kernel void Reorganize(	__global uint* gAlive, __global uint* gRank,	
							__global float4* gPosLifeIn,  __global float4* gVelMassIn,
							__global float4* gPosLifeOut, __global float4* gVelMassOut,
							uint nParticles) {


	// Re-order the particles according to the gRank obtained from the parallel prefix sum
	// ADD YOUR CODE HERE
	
	int GID = get_global_id(0);
	int LID = get_local_id(0);
	int LSIZE = get_local_size(0);



	if (GID == 0 || ((gRank[GID] - gRank[GID - 1]) == 1))
	{
		uint rank = gRank[GID] - 1;
		
		if (rank >= 0 && rank < nParticles)
		{
			gAlive[rank] = gAlive[GID];
			gPosLifeOut[rank] = gPosLifeIn[GID];
			gVelMassOut[rank] = gVelMassIn[GID];

			if (GID >= nParticles)
			{
				gAlive[GID] = 0;
			}
		}
	}
}