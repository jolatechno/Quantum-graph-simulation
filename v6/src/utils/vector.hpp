#pragma once

#include <iterator>     // std::iterator, std::input_iterator_tag

#include <vector>

template <typename value_type>
class numa_vector {
private:
    value_type* ptr = 0;

    size_t size_ = 0;
 
public:
    explicit numa_vector(size_t n = 0) {
    	resize(n);
	}
 
    // NOT SUPPORTED !!!
    size_t push_back(value_type) {
    	exit(0);
    	return 0;
    }
 
    // function that returns the popped element
    value_type pop_back() {
    	return ptr[size_-- - 1];
    }
 
    // Function that return the size of vector
    size_t size() const {
    	return size_;
    }

    value_type& operator[](size_t index) {
	    if (index >= size_)
	        throw;
	 
	    return *(ptr + index);
	}

	value_type operator[](size_t index) const {
	    if (index >= size_)
	        throw;
	 
	    return *(ptr + index);
	}

    void resize(size_t n) {
    	value_type* new_ptr = static_cast<value_type*>(malloc(n*sizeof(value_type)));
		value_type zero;

		#pragma omp parallel for schedule(static)
		for (unsigned long int i = 0; i < n; ++i)
			new_ptr[i] = i < size_ ? ptr[i] : zero;

		if (ptr != 0)
			free(ptr);

		ptr = new_ptr;
		size_ = n;
    }

    void iota_resize(size_t n) {
    	value_type* new_ptr = static_cast<value_type*>(malloc(n*sizeof(value_type)));

		#pragma omp parallel for schedule(static)
		for (unsigned long int i = 0; i < n; ++i)
			new_ptr[i] = i;

		if (ptr != 0)
			free(ptr);

		ptr = new_ptr;
		size_ = n;
    }
 
    // Begin iterator
    inline value_type* begin() const {
    	return ptr;
    }
 
    // End iterator
    inline value_type* end() const {
    	return begin() + size_;
    }
};