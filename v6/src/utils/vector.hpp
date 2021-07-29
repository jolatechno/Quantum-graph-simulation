#pragma once

#include <vector>

/* 
create a resize function that touches the right pages for numa optimization
*/
template<class T>
void resize(std::vector<T, allocator<T>> &v, size_t size) {
//#ifdef IGNORE_NUMA
	/* check if the resize is a downsize */
	bool downsize = size < v.size();

	/* resize the vector */
	v.resize(size);

	/* if the resize is a downsize, and the downsize wasn't effected then force shrink_to_fit */
	if (downsize && v.capacity() > size)
		std::vector<T, allocator<T>>(v).swap(v);
/*#else
	// create new vector
	// reserve does not touch pages
	std::vector<T, allocator<T>> new_vec;
	new_vec.reserve(size);

	// get all size and default element
	size_t old_size = v.size();
	T zero;

	// assign from each thread
	#pragma omp parallel for ordered schedule(static)
	for (unsigned int i = 0; i < size; ++i)
		#pragma omp ordered
		new_vec.push_back(i < old_size ? v[i] : zero);

	// swap vectors
	std::swap(v, new_vec);
#endif*/
}