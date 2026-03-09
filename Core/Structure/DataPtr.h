#pragma once

struct DataPtr {
	void* Data = nullptr;

	inline operator void*() {
		return Data;
	}
};