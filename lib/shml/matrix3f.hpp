#pragma once

#include <algorithm>
#include "vec3f.hpp"

namespace shml {
	/*	matrix3f is in COLUMN-MAJOR order.
	This means that my_matrix(1,2) is equivalent to the second column, third row.
	*/
	struct matrix3f {
		static matrix3f IDENTITY;

	private:
		static const int length = 9;
		float m_values[3][3];

		inline static int sign(const int pow) {
			if (pow % 2 == 0)
				return 1;
			else
				return -1;
		}
	public:

		// Creates an identity Matrix3f.
		inline matrix3f() {
			for (int i = 0; i < 3; i++) {
				for (int j = 0; j < 3; j++) {
					m_values[i][j] = i == j ? 1.0f : 0.0f;
				}
			}
		}

		// Creates a new matrix with values copied from mat.
		inline matrix3f(const matrix3f& mat) {
			*this = mat;
		}

		// Assigns mat's values to this matrix.
		inline matrix3f& operator=(const matrix3f& mat) {
			for (int i = 0; i < 3; i++) {
				for (int j = 0; j < 3; j++) {
					m_values[i][j] = mat.m_values[i][j];
				}
			}

			return *this;
		}

		// Returns a const pointer to the matrix's values.
		inline const float* getPointer() const {
			return &m_values[0][0];
		}

		// Returns a vec3f for the specified column.
		inline vec3f getColumn(const unsigned& col) const {
			vec3f v;
			v.x = m_values[col][0];
			v.y = m_values[col][1];
			v.z = m_values[col][2];

			return v;
		}

		inline void setColumn(const unsigned& column, const vec3f& v) {
			m_values[column][0] = v.x;
			m_values[column][1] = v.y;
			m_values[column][2] = v.z;
		}

		// Returns a vec3f for the specified row.
		inline vec3f getRow(const unsigned& row) const {
			vec3f v;
			v.x = m_values[0][row];
			v.y = m_values[1][row];
			v.z = m_values[2][row];

			return v;
		}

		inline void setRow(const unsigned& row, const vec3f& v) {
			m_values[0][row] = v.x;
			m_values[1][row] = v.y;
			m_values[2][row] = v.z;
		}

		// Returns the determinant of this matrix.
		inline float determinant() const {
			matrix3f mat(*this);
			return 0;
		}

		// Returns an inverted version of this matrix.
		inline matrix3f inverted() const {
			matrix3f mat(*this);
			mat.invert();

			return mat;
		}

		// Inverts this matrix.
		inline void invert() {
			matrix3f left(*this);
			matrix3f inv;

			// forward elimination from left - right
			for (int i = 0; i < 2; i++) {
				// get pivot within diagonal
				int pivot = i;
				float pivotSize = left(i, i);
				if (pivotSize < 0) pivotSize = -pivotSize;

				// find largest pivot in column (including and below current trace)
				for (int row = i + 1; row < 3; row++) {
					float temp = left(i, row);
					if (temp < 0) temp = -temp;

					if (temp > pivotSize) {
						pivot = row;
						pivotSize = temp;
					}
				}
				// largest pivot value is zero, therefore matrix is singular and cannot be inverted
				if (pivotSize == 0) {
					std::cout << "Singular matrix, cannot be inverted.\n";
					return;
				}
				// if we had to find a different pivot, swap the corresponding rows
				if (pivot != i) {
					// iterate through each column in the current row i and the pivot row pivot
					for (int col = 0; col < 3; col++) {
						float temp = left(col, i);
						left(col, i) = left(col, pivot);
						left(col, pivot) = temp;
						// do the same for inv
						temp = inv(col, i);
						inv(col, i) = inv(col, pivot);
						inv(col, pivot) = temp;
					}
				}
				// next, eliminate all numbers below diagonal
				for (int row = i + 1; row < 3; row++) {
					float f = left(i, row) / left(i, i);
					for (int col = 0; col < 3; col++) {
						left(col, row) -= f * left(col, i);
						inv(col, row) -= f * inv(col, i);
					}
					// set to zero in case of precision error
					left(i, row) = 0.0f;
				}
			}
			// step 3: normalize matrix such that the all diagonal values == 1.0
			for (int i = 0; i < 3; i++) {
				float divisor = left(i, i);
				for (int j = 0; j < 3; j++) {
					left(j, i) = left(j, i) / divisor;
					inv(j, i) = inv(j, i) / divisor;
				}
				// set to one in case of precision error
				left(i, i) = 1.0f;
			}
			// step 4: eliminate all numbers above diagonal
			for (int i = 0; i < 2; i++) {
				for (int j = i + 1; j < 3; j++) {
					float constant = left(j, i);
					for (int k = 0; k < 3; k++) {
						left(k, i) -= left(k, j) * constant;
						inv(k, i) -= inv(k, j) * constant;
					}
					// set to zero in case of precision error
					left(j, i) = 0.0f;
				}
			}

			*this = inv;
		}

		// Returns a copy of this matrix which is transposed.
		inline matrix3f transposed() const {
			matrix3f mat(*this);
			mat.transpose();

			return mat;
		}

		// Transposes this matrix.
		inline void transpose() {
			matrix3f temp;

			for (int i = 0; i < 3; i++) {
				for (int j = 0; j < 3; j++) {
					temp(i, j) = this->operator()(j, i);
				}
			}

			*this = temp;
		}

		// Returns the value at specified column and row.
		inline float operator() (const unsigned& column, const unsigned& row) const {
			return m_values[column][row];
		}

		// Returns the address at the specified column and row.
		inline float& operator() (const unsigned& column, const unsigned& row) {
			return m_values[column][row];
		}

		inline vec3f operator*(const vec3f& v) const {
			vec3f temp;
			temp.x = v.x * m_values[0][0] + v.y * m_values[1][0] + v.z * m_values[2][0];
			temp.y = v.x * m_values[0][1] + v.y * m_values[1][1] + v.z * m_values[2][1];
			temp.z = v.x * m_values[0][2] + v.y * m_values[1][2] + v.z * m_values[2][2];

			return temp;
		}

		inline matrix3f operator+(const matrix3f& mat) const {
			matrix3f m(*this);
			for (int i = 0; i < 3; i++) {
				for (int j = 0; j < 3; j++) {
					m.m_values[i][j] += mat.m_values[i][j];
				}
			}

			return m;
		}

		inline matrix3f& operator+=(const matrix3f& mat) {
			*this = *this + mat;
			return *this;
		}

		inline matrix3f operator-(const matrix3f& mat) const {
			matrix3f m(*this);
			for(int i = 0; i < 3; i++) {
				for (int j = 0; j < 3; j++) {
					m.m_values[i][j] -= mat.m_values[i][j];
				}
			}
			return m;
		}

		inline matrix3f& operator-=(const matrix3f& mat) {
			*this = *this - mat;
			return *this;
		}

		inline matrix3f operator*(const float& scalar) const {
			matrix3f m(*this);
			for (int i = 0; i < 3; i++) {
				for (int j = 0; j < 3; j++) {
					m.m_values[i][j] *= scalar;
				}
			}

			return m;
		}

		inline matrix3f& operator*=(const float& scalar) {
			*this = *this * scalar;
			return *this;
		}

		inline matrix3f operator*(const matrix3f& mat) const {
			matrix3f m;
			for (int i = 0; i < 3; i++) {
				for (int j = 0; j < 3; j++) {
					m(j, i) = vec3f::dot(this->getRow(i), mat.getColumn(j));
				}
			}
			return m;
		}

		inline matrix3f& operator*=(const matrix3f& mat) {
			*this = *this * mat;
			return *this;
		}

		inline bool operator==(const matrix3f& other) const {
			for (int i = 0; i < 3; i++) {
				for (int j = 0; j < 3; j++) {
					if (m_values[i][j] != other.m_values[i][j])
						return false;
				}
			}

			return true;
		}

		inline bool operator!=(const matrix3f& other) const {
			return !(*this == other);
		}
	};
}