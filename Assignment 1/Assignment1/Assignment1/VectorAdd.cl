
// TO DO: Add kernel code function



__kernel void VecAdd(__global const int* a, __global const int* b, __global int* c, int numElements)
{
	int GID = get_global_id(0);

	// Check if thread is inside of memory bounds
	if (GID < numElements)
	{
		// Add a and b together at the correct positions, reading b in reverse order

		c[GID] = a[GID] + b[numElements - GID - 1];
	}
}