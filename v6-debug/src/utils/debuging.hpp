#pragma once

#include <vector>
#include <iostream>

template<class T>
void print_vector(std::vector<T> const &vect) {
	for (auto const &l : vect)
		std::cout << l << " ";
	std::cout << "\n";
}