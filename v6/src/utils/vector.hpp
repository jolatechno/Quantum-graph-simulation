#include <vector>

template<class T>
void resize(std::vector<T, allocator<T>> &v, size_t size) {
	/* check if the resize is a downsize */
	bool downsize = size < v.size();

	/* resize the vector */
	v.resize(size);

	/* if the resize is a downsize, and the downsize wasn't effected then force shrink_to_fit */
	if (downsize && v.capacity() > size)
		std::vector<T, allocator<T>>(v).swap(v);
}