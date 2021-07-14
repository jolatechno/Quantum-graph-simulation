#include <vector>

template<class T>
void resize(std::vector<T, allocator<T>> &v, size_t size) {
	bool downsize = size < v.size();

	v.resize(size);

	if (downsize && v.capacity() > size) {
		std::vector<T, allocator<T>>(v).swap(v);
	}
}