#pragma once

#include <iostream>
#include <iomanip>

#include "vec3f.hpp"
#include "vec4f.hpp"
#include "quat.hpp"
#include "matrix3f.hpp"

namespace shml {
	/*	matrix4f is in COLUMN-MAJOR order.
	This means that my_matrix(1,2) is equivalent to the second column, third row.
	*/
	struct matrix4f {
		static matrix4f IDENTITY;

	private:
		static const int length = 16;
		float m_values[4][4];
		inline static unsigned to1D(const unsigned& column, const unsigned& row) {
			return (column * 4) + row;
		}
	public:
		// Constructs an identity matrix.
		inline matrix4f() {
			for (int i = 0; i < 4; i++) {
				for (int j = 0; j < 4; j++) {
					m_values[i][j] = (i == j) ? 1.0f : 0.0f;
				}
			}
		}

		// Copies the given matrix.
		inline matrix4f(matrix4f const& mat) {
			*this = mat;
		}

		// Assigns the given matrix's values to this matrix.
		inline matrix4f& operator=(const matrix4f& mat) {
			for (int i = 0; i < 4; i++) {
				for (int j = 0; j < 4; j++) {
					m_values[i][j] = mat.m_values[i][j];
				}
			}

			return *this;
		}

		// Returns a pointer to the matrix's values (should only be used for OpenGL).
		inline const float* getPointer() const {
			return &m_values[0][0];
		}

		// Returns a 4D vector representing the given column.
		inline vec4f getColumn(const unsigned& column) const {
			vec4f v;
			v.x = m_values[column][0];
			v.y = m_values[column][1];
			v.z = m_values[column][2];
			v.w = m_values[column][3];

			return v;
		}

		inline void setColumn(const unsigned& col, const vec4f& v) {
			m_values[col][0] = v.x;
			m_values[col][1] = v.y;
			m_values[col][2] = v.z;
			m_values[col][3] = v.w;
		}

		// Returns a 4D vector representing the given row.
		inline vec4f getRow(const unsigned& row) const {
			vec4f v;
			v.x = m_values[0][row];
			v.y = m_values[1][row];
			v.z = m_values[2][row];
			v.w = m_values[3][row];

			return v;
		}

		inline void setRow(const unsigned& row, const vec4f& v) {
			m_values[0][row] = v.x;
			m_values[1][row] = v.y;
			m_values[2][row] = v.z;
			m_values[3][row] = v.w;
		}

		// Returns the determinant of this matrix.
		inline float determinant() const {
			int laplace = 1;
			float det = 0;

			for (int i = 0; i < 4; i++) {
				det += m_values[i][0] * submatrix(i, 0).determinant() * laplace;
				laplace *= -1;
			}

			return det;
		}

		// Returns a given submatrix defined by eliminating the given column and row.
		inline matrix3f submatrix(const unsigned& deletedColumn, const unsigned& deletedRow) const {
			unsigned matrixIndex = 0;
			matrix3f mat;
			for (int row = 0; row < 4; row++) {
				if (row == deletedColumn)
					continue;

				for (int col = 0; col < 4; col++) {
					if (col == deletedRow)
						continue;

					mat(matrixIndex % 3, matrixIndex / 3) = m_values[row][col];
					matrixIndex++;
				}
			}

			return mat;
		}

		// Returns the cofactor of this matrix.
		inline matrix4f cofactor() const {
			matrix4f cofactor;
			int laplace;
			for (int row = 0; row < 4; row++) {
				for (int col = 0; col < 4; col++) {

					laplace = pow(-1, row + col);
					matrix3f sub = this->submatrix(col, row);
					float det = sub.determinant();
					cofactor.m_values[col][row] = laplace * det;
				}
			}

			return cofactor;
		}

		// Returns a copy of this matrix inverted by using Gauss-Jordan Elimination.
		inline matrix4f inverted() const {
			matrix4f mat(*this);
			mat.invert();

			return mat;
		}

		// Inverts this matrix using Gauss-Jordan Elimination.
		inline void invert() {
			matrix4f left(*this);
			matrix4f inv;

			// forward elimination from left - right
			for (int i = 0; i < 3; i++) {
				// get pivot within diagonal
				int pivot = i;
				float pivotSize = left(i, i);
				if (pivotSize < 0) pivotSize = -pivotSize;

				// find largest pivot in column (including and below current trace)
				for (int row = i + 1; row < 4; row++) {
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
					for (int col = 0; col < 4; col++) {
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
				for (int row = i + 1; row < 4; row++) {
					float f = left(i, row) / left(i, i);
					for (int col = 0; col < 4; col++) {
						left(col, row) -= f * left(col, i);
						inv(col, row) -= f * inv(col, i);
					}
					// set to zero in case of precision error
					left(i, row) = 0.0f;
				}
			}
			// step 3: normalize matrix such that the all diagonal values == 1.0
			for (int i = 0; i < 4; i++) {
				float divisor = left(i, i);
				for (int j = 0; j < 4; j++) {
					left(j, i) = left(j, i) / divisor;
					inv(j, i) = inv(j, i) / divisor;
				}
				// set to one in case of precision error
				left(i, i) = 1.0f;
			}
			// step 4: eliminate all numbers above diagonal
			for (int i = 0; i < 3; i++) {
				for (int j = i + 1; j < 4; j++) {
					float constant = left(j, i);
					for (int k = 0; k < 4; k++) {
						left(k, i) -= left(k, j) * constant;
						inv(k, i) -= inv(k, j) * constant;
					}
					// set to zero in case of precision error
					left(j, i) = 0.0f;
				}
			}

			*this = inv;
		}

	private:
		inline bool validatePivots(matrix4f& left, matrix4f& inv) {
			// iterate through each column and ensure pivots are not equal to zero
			for (int currentColumn = 0; currentColumn < 4; ++currentColumn) {
				int pivot = currentColumn;
				// pivot value is zero, find row to swap with
				if (left(pivot, pivot) == 0) {
					// iterate through all rows including and below pivot
					for (int row = 0; row < 4; ++row) {
						// find the highest value in the current column, that will be the row to swap
						if (std::abs(left(currentColumn, row)) > std::abs(left(currentColumn, pivot))) {
							pivot = row;
						}
					}
					// if pivot is unchanged after trying to find highest value, all values below pivot are also zero
					// therefore, matrix is singular and cannot be inverted
					if (pivot == currentColumn) {
						std::cout << "Singular matrix, cannot be inverted.\n";
						return false;
					}
					// valid pivot found, swap the corresponding rows
					else for (int j = 0; j < 4; ++j) {
						std::swap(left(currentColumn, j), left(pivot, j));
						std::swap(inv(currentColumn, j), inv(pivot, j));
					}
				}
			}

			return true;
		}

		void eliminateBelowPivot(matrix4f& left, matrix4f& inv) {
			// process all columns (except last) to set the values their pivots to zero
			// we skip the last column as there are no entries below matrix(3, 3)
			for (int column = 0; column < 3; ++column) {
				for (int row = column + 1; row < 4; ++row) {
					float constant = left(column, row) / left(column, column);

					// process each entry on the current row
					for (int j = 0; j < 4; ++j) {
						left(j, row) -= constant * left(j, column);
						inv(j, row) -= constant * inv(j, column);
					}

					left(column, row) = 0;
				}
			}
		}

		void normalizePivots(matrix4f& left, matrix4f& inv) {
			for (int row = 0; row < 4; ++row) {
				float divisor = 1.0f / left(row, row);

				for (int column = 0; column < 4; ++column) {
					left(column, row) *= divisor;
					inv(column, row) *= divisor;
				}
			}
		}

	public:
		// Returns a copy of this matrix which is transposed.
		inline matrix4f transposed() const {
			matrix4f mat(*this);
			mat.transpose();

			return mat;
		}

		// Transposes this matrix.
		inline void transpose() {
			matrix4f temp;

			for (int i = 0; i < 4; i++) {
				for (int j = 0; j < 4; j++) {
					temp.m_values[i][j] = m_values[j][i];
				}
			}

			*this = temp;
		}



		// Returns a 3D vector representing this matrix's translation.
		inline vec3f getPosition() const {
			return (vec3f)getColumn(3);
		}

		// Sets this matrix's position.
		inline void setPosition(const vec3f& position) {
			this;
			vec4f pos(position);
			setColumn(3, pos);
		}

		// Applies the translation to this matrix.
		inline void translate(const vec3f& translation) {
			m_values[3][0] += translation.x;
			m_values[3][1] += translation.y;
			m_values[3][2] += translation.z;
		}

		// Returns the extracted rotation matrix of this matrix.
		inline matrix3f getRotationMatrix(const bool& removeScale = false) const {
			matrix3f rm;
			for (int i = 0; i < 3; i++) {
				for (int j = 0; j < 3; j++) {
					rm(i, j) = this->m_values[i][j];
				}
			}

			if (!removeScale)
				return rm;

			vec3f scale;
			scale.x = rm.getRow(0).length();
			scale.y = rm.getRow(1).length();
			scale.z = rm.getRow(2).length();

			rm(0, 0) /= scale.x; rm(1, 0) /= scale.y; rm(2, 0) /= scale.z;
			rm(0, 1) /= scale.x; rm(1, 1) /= scale.y; rm(2, 1) /= scale.z;
			rm(0, 2) /= scale.x; rm(1, 2) /= scale.y; rm(2, 2) /= scale.z;

			return rm;
		}

		// Returns the quaternion representation of this matrix's rotation.
		inline quat getRotation() const {
			return quat(getRotationMatrix());
		}

		// Sets this matrix's rotation.
		inline void setRotation(const quat& rotation) {
			setRotation(rotation.rotationMatrix());
		}

		// Sets this matrix's rotation.
		inline void setRotation(const matrix3f& rotationMatrix) {
			for (int i = 0; i < 3; i++) {
				for (int j = 0; j < 3; j++) {
					m_values[i][j] = rotationMatrix(i, j);
				}
			}
		}

		// Applies a rotation to this matrix.
		inline void rotate(const quat& rotation) {
			rotate(rotation.rotationMatrix());
		}

		// Applies a rotation to this matrix.
		inline void rotate(const matrix3f& rotationMatrix) {
			setRotation(getRotationMatrix() * rotationMatrix);
			orthonormalize();
		}

		inline void orthonormalize() {
			vec3f p = getPosition();
			matrix3f rm = getRotationMatrix(true);

			vec3f basisX = rm.getColumn(0);
			vec3f basisY = rm.getColumn(1);
			vec3f basisZ = rm.getColumn(2);

			vec3f u1 = basisX.normalized();

			vec3f w2 = basisY - vec3f::project(basisY, basisX);
			vec3f u2 = w2.normalized();

			vec3f w3 = basisZ - vec3f::project(basisZ, basisY);
			w3 = w3 - vec3f::project(w3, basisX);
			vec3f u3 = w3.normalized();

			m_values[0][0] = u1.x; m_values[1][0] = u2.x; m_values[2][0] = u3.x; m_values[3][0] = p.x;
			m_values[0][1] = u1.y; m_values[1][1] = u2.y; m_values[2][1] = u3.y; m_values[3][1] = p.y;
			m_values[0][2] = u1.z; m_values[1][2] = u2.z; m_values[2][2] = u3.z; m_values[3][2] = p.z;
			m_values[0][3] =    0; m_values[1][3] =    0; m_values[2][3] =    0; m_values[3][3] =   1;
		}

		// Returns the value at specified column and row.
		inline float operator() (const unsigned& column, const unsigned& row) const {
			float v = m_values[column][row];
			return v;
		}

		// Returns the modifiable l-value at the specified column and row.
		inline float& operator() (const unsigned& column, const unsigned& row) {
			return m_values[column][row];
		}

		// Returns a copy of v multiplied by this matrix.
		inline vec3f operator*(const vec3f& v) const {
			vec4f v4(v);
			return (vec3f)(*this * v4);
		}

		// Returns a copy of v multiplied by this matrix.
		inline vec4f operator*(const vec4f& v) const {
			vec4f result;
			result.x = vec4f::dot(getRow(0), v);
			result.y = vec4f::dot(getRow(1), v);
			result.z = vec4f::dot(getRow(2), v);
			result.w = vec4f::dot(getRow(3), v);

			return result;
		}

		// Returns a matrix whose elements are this matrix + mat.
		inline matrix4f operator+(const matrix4f& mat) const {
			matrix4f m(*this);
			for (int i = 0; i < 4; i++) {
				for (int j = 0; j < 4; j++) {
					m(i, j) += mat(i, j);
				}
			}

			return m;
		}

		// Adds the elements of mat to this matrix.
		inline matrix4f& operator+=(const matrix4f& mat) {
			for (int i = 0; i < 4; i++) {
				for (int j = 0; j < 4; j++) {
					m_values[i][j] += mat(i, j);
				}
			}

			return *this;
		}

		// Returns a matrix whose elements are this matrix - mat.
		inline matrix4f operator-(const matrix4f& mat) const {
			matrix4f m(*this);
			for (int i = 0; i < 4; i++) {
				for (int j = 0; j < 4; j++) {
					m(i, j) -= mat(i, j);
				}
			}

			return m;
		}

		// Subtracts the elements of mat from this matrix.
		inline matrix4f& operator-=(const matrix4f& mat) {
			for (int i = 0; i < 4; i++) {
				for (int j = 0; j < 4; j++) {
					m_values[i][j] -= mat(i, j);
				}
			}

			return *this;
		}

		// Returns a copy of this matrix scaled by scalar.
		inline matrix4f operator*(const float& scalar) const {
			matrix4f m(*this);
			for (int i = 0; i < 4; i++) {
				for (int j = 0; j < 4; j++) {
					m(i, j) *= scalar;
				}
			}

			return m;
		}

		// Scales this matrix's elements by scalar.
		inline matrix4f& operator*=(const float& scalar) {
			for (int i = 0; i < 4; i++) {
				for (int j = 0; j < 4; j++) {
					m_values[i][j] *= scalar;
				}
			}

			return *this;
		}

		// Returns a matrix whose elements are this * mat matrix.
		inline matrix4f operator*(const matrix4f& mat) const {
			matrix4f m;

			for (int row = 0; row < 4; row++) {
				for (int col = 0; col < 4; col++) {
					float val = vec4f::dot(this->getRow(row), mat.getColumn(col));
					m(col, row) = val;
				}
			}

			return m;
		}

		// Multiplies this matrix's elements by mat (right multiplication).
		inline matrix4f& operator*=(const matrix4f& mat) {
			matrix4f m;
			for (int row = 0; row < 4; row++) {
				for (int col = 0; col < 4; col++) {
					float val = vec4f::dot(this->getRow(row), mat.getColumn(col));
					m(col, row) = val;
				}
			}

			*this = m;
			return *this;
		}

		// Checks if both matrices contain the same values.
		inline bool operator==(const matrix4f& other) const {
			for (int i = 0; i < 4; i++) {
				for (int j = 0; j < 4; j++) {
					if (m_values[i][j] != other.m_values[i][j]) {
						return false;
					}
				}
			}

			return true;
		}

		// Checks if both matrices contain differing values.
		inline bool operator!=(const matrix4f& other) const {
			return !(*this == other);
		}

		inline friend std::ostream& operator<<(std::ostream& os, const matrix4f& m) {
			os << "[MAT4]\n" << std::fixed << std::setprecision(2);
			for (int row = 0; row < 4; row++) {
				os << "|--[ ";
				for (int col = 0; col < 4; col++) {
					os << m(col, row) << ' ';
				}
				os << "]\n";
			}
			os << std::endl;
			return os;
		}
	};
}