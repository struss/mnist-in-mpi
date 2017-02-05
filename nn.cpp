#pragma once

#include "matrix.cpp"
#include <random> // normal_distribution


#include "evaluate.cpp"
#include <iostream>


using namespace std;


/*** Requisites ***/
vector<double> softmax(const vector<double>& v) {
    // Subtract maximum to avoid overflow
    double max = *max_element(v.begin(), v.end());

    vector<double> expd = v; // copy, does not mutate v
    double sum = 0;
    for (double& x : expd) {
        x = exp(x - max);
        sum += x;
    }

    vector<double> result(v.size(), 0.);
    for (int i = 0; i < v.size(); ++i)
        result[i] = expd[i] / sum;

    return result;
}
Matrix softmax(const Matrix& m) {
    Matrix result = m;
    for (auto& row : result)
        row = softmax(row);
    return result;
}

double cross_entropy(const Matrix& Y, const Matrix& Y_prob) {
    // Cost function
    Matrix result = hadamard(Y, log(Y_prob));
    return 1/double(n_rows(Y)) * sum(result);
}

Matrix random_init(int rows, int cols, double mean=0., double std=1.) {
    default_random_engine generator;
    normal_distribution<double> distribution(mean, std);

    Matrix result = blank_matrix(rows, cols, 0.);
    for (auto& row : result)
        for (double& elem : row)
            elem = distribution(generator);

    return result;
}




class NeuralNetwork {

    Matrix W;
    Matrix b;
    int n_classes;
    int data_dim;

public:

    NeuralNetwork(int n_classes=10, int data_dim=784) : n_classes(n_classes), data_dim(data_dim) {
        b = blank_matrix(1, n_classes, 0.);
        W = random_init(data_dim, n_classes);
        // TODO? more layers
    }

    // TODO: parallelization
    double grad(Matrix X, Matrix Y, Matrix &grad_W, Matrix &grad_b) {
        Matrix Y_prob = softmax(add_to_each(X * W, b));

        cout << accuracy(argmax_matrix(Y_prob), form_one_hot_matrix(Y)) << endl;
        // Error at last layer
        Matrix delta = Y_prob - Y;

        // Gradient of cross entropy cost
        grad_W = transpose(X) * delta;
        Matrix ones = blank_matrix(1, delta.size(), 1.);
        grad_b = ones * delta;

        return cross_entropy(Y, Y_prob);
    }

    vector<double> train(Matrix X, Matrix Y, int epochs=10, int batch_size=100, double lr=0.1) {
        vector<double> cost_history;

        Matrix grad_W = blank_matrix(data_dim, n_classes, 0.);
        Matrix grad_b = blank_matrix(1, n_classes, 0.);

        for (int epoch=0; epoch<epochs; epoch++) {
            for (int i=0; i<n_rows(X); i+=batch_size) {
                // TODO: make random batch generator
                auto cost = grad(chunk(X, i, i+batch_size),
                                 chunk(Y, i, i+batch_size),
                                 grad_W,  grad_b);

                // TODO regularization
                W = W - lr * 1/double(batch_size) * grad_W;
                b = b - lr * 1/double(batch_size) * grad_b;

                cost_history.push_back(cost);
            }
        }
        return cost_history;
    }

};
