#pragma once

typedef int int32_t;
typedef unsigned int uint32_t;

template <typename T>
class IDVector {
	T* pData = nullptr;
	int32_t* pIds = nullptr;
	int32_t* pDataIndices = nullptr;
	uint32_t dataSize = 0;
	uint32_t capacity = 0;

	void resize() {
		uint32_t oldCapacity = capacity;
		this->capacity <<= 1;

		T* newValues = new T[capacity];
		int32_t* newIndices = new int32_t[capacity];
		int32_t* newIds = new int32_t[capacity];

		for (uint32_t i = 0; i < oldCapacity; i++) {
			int32_t id = this->pIds[i];

			int32_t idx = this->pDataIndices[id];
			if (idx < 0) {
				continue;
			}

			newIds[i] = id;
			newValues[idx] = pData[idx];
			newIndices[id] = idx;
		}

		delete[] pData;
		this->pData = newValues;

		delete[]pDataIndices;
		this->pDataIndices = newIndices;

		delete[] pIds;
		this->pIds = newIds;
	}

public:
	IDVector(uint32_t initialCapacity = 0) {
		if (initialCapacity == 0) {
			initialCapacity = 2;
		}

		capacity = initialCapacity;
		dataSize = 0;

		pData = new T[initialCapacity];
		pDataIndices = new int32_t[initialCapacity]{ -1 };
		pIds = new int32_t[initialCapacity] { -1 };
	}

	~IDVector() {
		dispose();
	}

	T* const data() const {
		return pData;
	}

	uint32_t size() const {
		return dataSize;
	}

	int32_t add(T value) {
		if (dataSize >= capacity) {
			resize();
		}

		pData[dataSize] = value;
		pIds[dataSize] = dataSize;
		pDataIndices[dataSize] = dataSize;
		dataSize++;
		return dataSize - 1;
	}

	bool get(int32_t id, T*& outData) {
		if (id < 0 || id >= capacity) {
			return false;
		}

		int32_t dataIndex = pDataIndices[id];
		if (dataIndex == -1) {
			return false;
		}

		*outData = pData[dataIndex];
		return true;
	}

	bool remove(int32_t id) {
		if (id < 0 || id >= capacity) {
			return false;
		}
		else if (dataSize == 0) {
			return false;
		}

		int32_t dataIndex = pDataIndices[id];
		if (dataIndex == -1) {
			return false;
		}

		T temp = pData[dataSize - 1];
		pData[dataIndex] = temp;

		int32_t tempIndex = pDataIndices[dataSize - 1];
		pDataIndices[id] = tempIndex;

		pDataIndices[dataSize - 1] = -1;
		dataSize--;
		return true;
	}

	void dispose() {
		dataSize = 0;
		capacity = 0;

		delete[] pData;
		pData = nullptr;
		delete[] pIds;
		pIds = nullptr;
		delete[] pDataIndices;
		pDataIndices = nullptr;
	}
};