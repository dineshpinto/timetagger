#include <algorithm>
#include <limits>

template<typename T> void get_min_max(std::vector<T> &data, T &vmin, T &vmax) {

	vmin = std::numeric_limits<T>::min();
	vmax = std::numeric_limits<T>::max();

	for (size_t n=0; n<data.size(); ++n) {
		T v=data[n];
		vmin=std::min(vmin, v);
		vmax=std::max(vmax, v);
	}
}

template<typename T> void get_min_max(std::vector< std::vector< T > > &data, T &vmin, T &vmax) {

	vmin = std::numeric_limits<T>::min();
	vmax = std::numeric_limits<T>::max();

	for (size_t n=0; n<data.size(); ++n) {
		std::vector<T> &row=data[n];
		for (size_t j=0; j<row.size(); ++j) {
			T v=row[j];
			vmin=std::min(vmin, v);
			vmax=std::max(vmax, v);
		}
	}
}
