#pragma once

#include <string>
#include <iostream>
#include <cassert>



template<typename T>
class linkedList {
	struct node {
		T item;
		node* next;
		inline node() {
			next = nullptr;
		}
		inline node(T item, node* next) {
			this->item = item;
			this->next = next;
		}
	};

	int m_count;
	node* head = nullptr;
	node* tail = nullptr;

#define ASSERT_IN_RANGE(x) assert(m_count > 0 && index >= 0 && index < m_count)
#define ASSERT_HAS_VALUE(x) assert(x != nullptr)

public:
	typedef bool (*Comparer)(T t1, T t2);

	linkedList() {
		head = nullptr;
		tail = nullptr;
		m_count = 0;
	}
	~linkedList() { clear(); }

	uint32_t size() const { return m_count; }
	
	T first() {
		ASSERT_HAS_VALUE(head);
		return head->item;
	}
	T last() {
		ASSERT_HAS_VALUE(tail);
		return tail->item;
	}

	bool getPtr(uint32_t index, T** pOut) {
		ASSERT_IN_RANGE(index);
		node* temp = head;

		uint32_t current = 0;
		while (temp && ++current < index) {
			temp = temp->next;
		}

		assert(current == index);
		pOut = &temp->item;
		return pOut != nullptr;
	}

	void add(T item) {
		node* newNode = new node(item, nullptr);
		newNode->item = item;
		newNode->next = nullptr;

		if (!head) {
			head = newNode;
			m_count++;
			tail = head;
			return;
		}

		node* temp = head;
		while (temp->next) {
			temp = temp->next;
		}

		temp->next = tail = newNode;
		m_count++;
	}

	void addUnique(T item, Comparer comp) {
		node* temp = head;
		while (temp) {
			if (comp(temp, item)) {
				return;
			}
			temp = temp->next;
		}

		add(item);
	}

	void addRange(const T* arr, uint32_t size) {
		for (uint32_t i = 0; i < size; i++) {
			add(arr[i]);
		}
	}

	void emplace(T item, uint32_t index) {
		if (index < 0) {
			std::cout << "Invalid index!\n";
			return;
		}
		// ignore above range indices, append to tail
		else if (index >= m_count) {
			add(item);
			return;
		}

		node* newNode = new node();
		newNode->item = item;

		// make new entry list head
		if (index == 0) {
			newNode->next = head;
			head = newNode;
			m_count++;
			return;
		}
		// make new entry list tail
		else if (index == m_count - 1) {
			tail->next = newNode;
			tail = newNode;
			m_count++;
			return;
		}

		node* temp = head;
		node* lastTemp = nullptr;
		uint32_t current = 0;

		while (temp && current++ < index) {
			lastTemp = temp;
			temp = temp->next;
		}

		ASSERT_HAS_VALUE(lastTemp);

		newNode->next = lastTemp->next;
		lastTemp->next = newNode;
		m_count++;
	}

	bool contains(T value, Comparer comp) {
		node* current = head;
		while (current->next != nullptr) {
			if (comp(value, current)) {
				return true;
			}
		}

		return false;
	}

	bool remove(T item) {
		node* temp = head;
		node* lastTemp = nullptr;

		// empty list
		if (!temp) {
			return false;
		}

		// remove head
		if (temp->item == item) {
			head = temp->next;
			delete temp;
			m_count--;
			return true;
		}
		else {
			lastTemp = temp;
			temp = temp->next;
		}

		while (temp) {
			if (temp->item == item) {
				if (temp->next) {
					lastTemp->next = temp->next;
				}
				else {
					lastTemp->next = nullptr;
				}

				delete temp;
				m_count--;
				return true;
			}
			else {
				lastTemp = temp;
				temp = temp->next;
			}
		}
		// couldn't find item
		return false;
	}

	bool removeAt(const uint32_t index) {
		if (index >= m_count) {
			return false;
		}

		node* temp = head;
		node* lastTemp = nullptr;

		// empty list
		if (!temp) {
			return false;
		}

		node* current = this->head;
		node* prev = nullptr, next = nullptr;
		for (uint32_t i = 1; i <= index; i++) {
			prev = current;
			current = next;
			if (current == nullptr) {
				return false;
			}

			next = current->next;
		}

		delete current;
		prev->next = next;
		return true;
	}

	void clear() {
		if (!head) {
			return;
		}

		node* temp = head;
		node* n = nullptr;
		while (temp) {
			n = temp->next;
			delete temp;
			temp = n;
		}

		m_count = 0;
		head = tail = nullptr;
	}

	inline void toArray(T*& outArray) const {
		if (m_count <= 0) return;

		if (outArray != nullptr) {
			delete[] outArray;
		}

		outArray = new T[this->m_count];

		int index = 0;
		node* current = head;
		while (index < m_count && current != nullptr) {
			outArray[index++] = current->item;
			current = current->next;
		}
	}

	T operator[](uint32_t index) const {
		ASSERT_IN_RANGE(index);

		node* temp = head;
		uint32_t current = 0;

		// prefix increment so we get the last valid node
		while (temp && ++current < index) {
			temp = temp->next;
		}

		ASSERT_HAS_VALUE(temp);
		return temp->item;
	}
};