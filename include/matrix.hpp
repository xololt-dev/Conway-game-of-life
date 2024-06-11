#pragma once

#include <sched.h>
#include <tuple>
#include <vector>

// https://codereview.stackexchange.com/questions/142815/generic-matrix-type-in-c
template <class T>
class Matrix {
public:
    using value_type = typename std::vector<T>::value_type;
    using reference = typename std::vector<T>::reference;
    using size_type = typename std::vector<T>::size_type;

    void load(const std::vector<T>& a_data) {
        data = a_data;
    }
    std::vector<T> board() {
        return data;
    }

    Matrix(size_type const a_rows, size_type const a_columns) :
        rows(a_rows), columns(a_columns) {
        data.resize(a_rows * a_columns, 0);
    }
    ~Matrix() {}
    Matrix(Matrix& a_matrix) {
        data = a_matrix.data;
        rows = a_matrix.rows;
        columns = a_matrix.columns;
    }

    reference operator()(size_type const a_row, size_type const a_column) {
        return data[index(a_row, a_column)];
    }

    std::tuple<size_type, size_type> dimensions() {
        return std::make_tuple(rows, columns);
    }

private:
    std::vector<T> data;
    size_type const rows;
    size_type const columns;

    inline int index(size_type const a_row, size_type const a_column) {
        return a_row + (rows * a_column);
    }
};