#ifndef UTIL_H
#define UTIL_H

template <typename Container, typename Containable>
	bool contains(Container C, Containable x, int size_){
		for (int i = 0; i < size_; ++i){
			if (C[i] == x){
				return true;
			}
		}
		return false;
	}
#endif /* UTIL_H */