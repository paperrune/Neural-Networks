#include <fstream>
#include <iostream>
#include <math.h>
#include <memory.h>
#include <random>
#include <sstream>
#include <stdlib.h>
#include <unordered_map>

#include "Neural_Networks.h"

Batch_Normalization::Batch_Normalization(int number_maps, int map_size, double epsilon, double momentum, ::Layer *layer) {
	this->batch_size = 1;
	this->beta_initializer = new ::Initializer(0);
	this->beta_optimizer = nullptr;
	this->epsilon = epsilon;
	this->gamma_initializer = new ::Initializer(1);
	this->gamma_optimizer = nullptr;
	this->layer = layer;
	this->map_size = map_size;
	this->momentum = momentum;
	this->moving_mean_initializer = new ::Initializer(0);
	this->moving_variance_initializer = new ::Initializer(1);
	this->number_maps = number_maps;
	this->number_nodes = number_maps * map_size;

	beta = new float[number_maps];
	gamma = new float[number_maps];
	mean = new float[number_maps];
	variance = new float[number_maps];
	moving_mean = new float[number_maps];
	moving_variance = new float[number_maps];

	error_backup = new float[number_nodes];
	error_normalized = new float[number_nodes];
	neuron_backup = new float[number_nodes];
	neuron_normalized = new float[number_nodes];
}
Batch_Normalization::~Batch_Normalization() {
	if (beta_optimizer) {
		delete beta_optimizer;
	}
	if (gamma_optimizer) {
		delete gamma_optimizer;
	}
	delete[] beta;
	delete[] gamma;
	delete[] mean;
	delete[] variance;
	delete[] moving_mean;
	delete[] moving_variance;

	delete[] error_backup;
	delete[] error_normalized;
	delete[] neuron_backup;
	delete[] neuron_normalized;

	delete beta_initializer;
	delete gamma_initializer;
	delete moving_mean_initializer;
	delete moving_variance_initializer;
}

void Batch_Normalization::Activate(float _neuron[], bool training) {
	if (training) {
		#pragma omp parallel for
		for (int j = 0; j < number_maps; j++) {
			float *neuron = &_neuron[j * map_size];
			float *neuron_backup = &this->neuron_backup[j * map_size];
			float *neuron_normalized = &this->neuron_normalized[j * map_size];

			double standard_deviation;
			double sum = 0;

			for (int h = 0; h < batch_size; h++) {
				int index = h * number_nodes;

				for (int k = 0; k < map_size; k++) {
					sum += neuron[index + k];
				}
			}
			moving_mean[j] = momentum * moving_mean[j] + (1 - momentum) * (mean[j] = sum / (batch_size * map_size));

			sum = 0;
			for (int h = 0; h < batch_size; h++) {
				int index = h * number_nodes;

				for (int k = 0; k < map_size; k++) {
					sum += (neuron[index + k] - mean[j]) * (neuron[index + k] - mean[j]);
				}
			}
			moving_variance[j] = momentum * moving_variance[j] + (1 - momentum) * (variance[j] = sum / (batch_size * map_size));
			standard_deviation = sqrt(variance[j] + epsilon);

			for (int h = 0; h < batch_size; h++) {
				int index = h * number_nodes;

				for (int k = 0; k < map_size; k++) {
					neuron_backup[index + k] = neuron[index + k];
					neuron_normalized[index + k] = (neuron[index + k] - mean[j]) / standard_deviation;
					neuron[index + k] = gamma[j] * neuron_normalized[index + k] + beta[j];
				}
			}
		}
	}
	else {
		#pragma omp parallel for
		for (int j = 0; j < number_maps; j++) {
			float *neuron = &_neuron[j * map_size];
			float *neuron_backup = &this->neuron_backup[j * map_size];

			double standard_deviation = sqrt(moving_variance[j] + epsilon);

			for (int h = 0; h < batch_size; h++) {
				int index = h * number_nodes;

				for (int k = 0; k < map_size; k++) {
					neuron_backup[index + k] = neuron[index + k];
					neuron[index + k] = gamma[j] / standard_deviation * neuron[index + k] + (beta[j] - gamma[j] * moving_mean[j] / standard_deviation);
				}
			}
		}
	}
}
void Batch_Normalization::Adjust_Parameter(int iterations) {
	#pragma omp parallel for
	for (int j = 0; j < number_maps; j++) {
		double sum = 0;

		float *error_backup = &this->error_backup[j * map_size];
		float *neuron_normalized = &this->neuron_normalized[j * map_size];

		for (int h = 0; h < batch_size; h++) {
			int index = h * number_nodes;

			for (int k = 0; k < map_size; k++) {
				sum += error_backup[index + k] * neuron_normalized[index + k];
			}
		}
		gamma[j] += gamma_optimizer->Calculate_Gradient(j, sum, iterations);

		sum = 0;
		for (int h = 0; h < batch_size; h++) {
			int index = h * number_nodes;

			for (int k = 0; k < map_size; k++) {
				sum += error_backup[index + k];
			}
		}
		beta[j] += beta_optimizer->Calculate_Gradient(j, sum, iterations);
	}
}
void Batch_Normalization::Differentiate(float _error[]) {
	#pragma omp parallel for
	for (int j = 0; j < number_maps; j++) {
		float *error = &_error[j * map_size];
		float *error_backup = &this->error_backup[j * map_size];
		float *error_normalized = &this->error_normalized[j * map_size];
		float *neuron_backup = &this->neuron_backup[j * map_size];

		double error_mean;
		double error_variance = 0;
		double standard_deviation = sqrt(variance[j] + epsilon);
		double sum[] = { 0, };

		for (int h = 0; h < batch_size; h++) {
			int index = h * number_nodes;

			for (int k = 0; k < map_size; k++) {
				error_normalized[index + k] = error[index + k] * gamma[j];
				error_variance += error_normalized[index + k] * (neuron_backup[index + k] - mean[j]);
			}
		}
		error_variance *= (-0.5) * pow(variance[j] + epsilon, -1.5);

		for (int h = 0; h < batch_size; h++) {
			int index = h * number_nodes;

			for (int k = 0; k < map_size; k++) {
				sum[0] += error_normalized[index + k];
				sum[1] += (neuron_backup[index + k] - mean[j]);
			}
		}
		error_mean = -sum[0] / standard_deviation + error_variance * (-2) * sum[1] / (batch_size * map_size);

		for (int h = 0; h < batch_size; h++) {
			int index = h * number_nodes;

			for (int k = 0; k < map_size; k++) {
				error_backup[index + k] = error[index + k];
				error[index + k] = error_normalized[index + k] / standard_deviation + error_variance * 2 * (neuron_backup[index + k] - mean[j]) / (batch_size * map_size) + error_mean / (batch_size * map_size);
			}
		}
	}
}
void Batch_Normalization::Initialize() {
	beta_initializer->Random(number_maps, beta, number_maps, number_maps);
	gamma_initializer->Random(number_maps, gamma, number_maps, number_maps);
	moving_mean_initializer->Random(number_maps, moving_mean, number_maps, number_maps);
	moving_variance_initializer->Random(number_maps, moving_mean, number_maps, number_maps);
}
void Batch_Normalization::Load(ifstream &file) {
	for (int j = 0; j < layer->number_maps; j++) {
		file >> gamma[j];
	}
	for (int j = 0; j < layer->number_maps; j++) {
		file >> beta[j];
	}
	for (int j = 0; j < layer->number_maps; j++) {
		file >> moving_mean[j];
	}
	for (int j = 0; j < layer->number_maps; j++) {
		file >> moving_variance[j];
	}
}
void Batch_Normalization::Optimizer(::Optimizer &optimizer) {
	if (beta_optimizer) {
		delete beta_optimizer;
	}
	if (gamma_optimizer) {
		delete gamma_optimizer;
	}
	beta_optimizer = optimizer.Copy(number_maps);
	gamma_optimizer = optimizer.Copy(number_maps);
}
void Batch_Normalization::Resize_Memory(int batch_size) {
	if (this->batch_size != batch_size) {
		int memory_size = sizeof(float) * batch_size * number_nodes;

		error_backup = (float*)realloc(error_backup, memory_size);
		error_normalized = (float*)realloc(error_normalized, memory_size);
		neuron_backup = (float*)realloc(neuron_backup, memory_size);
		neuron_normalized = (float*)realloc(neuron_normalized, memory_size);

		this->batch_size = batch_size;
	}
}
void Batch_Normalization::Save(ofstream &file) {
	for (int j = 0; j < layer->number_maps; j++) {
		file << gamma[j] << endl;
	}
	for (int j = 0; j < layer->number_maps; j++) {
		file << beta[j] << endl;
	}
	for (int j = 0; j < layer->number_maps; j++) {
		file << moving_mean[j] << endl;
	}
	for (int j = 0; j < layer->number_maps; j++) {
		file << moving_variance[j] << endl;
	}
}

Batch_Normalization* Batch_Normalization::Beta_Initializer(Initializer initializer) {
	if (beta_initializer) {
		delete beta_initializer;
	}
	beta_initializer = initializer.Copy();
	return this;
}
Batch_Normalization* Batch_Normalization::Gamma_Initializer(Initializer initializer) {
	if (gamma_initializer) {
		delete gamma_initializer;
	}
	gamma_initializer = initializer.Copy();
	return this;
}
Batch_Normalization* Batch_Normalization::Moving_Mean_Initializer(Initializer initializer) {
	if (moving_mean_initializer) {
		delete moving_mean_initializer;
	}
	moving_mean_initializer = initializer.Copy();
	return this;
}
Batch_Normalization* Batch_Normalization::Moving_Variance_Initializer(Initializer initializer) {
	if (moving_variance_initializer) {
		delete moving_variance_initializer;
	}
	moving_variance_initializer = initializer.Copy();
	return this;
}

Layer* Batch_Normalization::Layer() {
	return layer;
}


Connection::Connection(Layer *layer, Layer *parent_layer, string properties) {
	unordered_map<int, int> weight_index;

	this->from_error = nullptr;
	this->from_neuron = nullptr;
	this->from_weight = nullptr;
	this->initializer = nullptr;
	this->layer = layer;
	this->parent_layer = parent_layer;
	this->properties = properties;
	this->number_weights = 0;
	this->optimizer = nullptr;
	this->weight = nullptr;

	// set kernel / pool size if specified
	if (const char *kernel_size = strstr(properties.c_str(), "kernel")) {
		const char *end = strstr(kernel_size, ")");

		kernel_width = atoi(kernel_size + 7);
		kernel_size = strstr(kernel_size, "x");

		if (kernel_size && kernel_size < end && atoi(kernel_size + 1) > 0) {
			kernel_height = atoi(kernel_size + 1);
			kernel_size = strstr(kernel_size + 1, "x");

			if (kernel_size && kernel_size < end && atoi(kernel_size + 1) > 0) {
				kernel_depth = atoi(kernel_size + 1);
			}
			else {
				kernel_depth = 1;
			}
		}
		else {
			kernel_height = 1;
			kernel_depth = 1;
		}
	}
	else if (const char *pool_size = strstr(properties.c_str(), "pool")) {
		const char *end = strstr(pool_size, ")");

		kernel_width = atoi(pool_size + 5);
		pool_size = strstr(pool_size, "x");

		if (pool_size && pool_size < end && atoi(pool_size + 1) > 0) {
			kernel_height = atoi(pool_size + 1);
			pool_size = strstr(pool_size + 1, "x");

			if (pool_size && pool_size < end && atoi(pool_size + 1) > 0) {
				kernel_depth = atoi(pool_size + 1);
			}
			else {
				kernel_depth = 1;
			}
		}
		else {
			kernel_height = 1;
			kernel_depth = 1;
		}
	}
	else if (properties[0] == 'P') {
		kernel_width = (parent_layer->map_width > layer->map_width) ? (parent_layer->map_width / layer->map_width) : (layer->map_width / parent_layer->map_width);
		kernel_height = (parent_layer->map_height > layer->map_height) ? (parent_layer->map_height / layer->map_height) : (layer->map_height / parent_layer->map_height);
		kernel_depth = (parent_layer->map_depth > layer->map_depth) ? (parent_layer->map_depth / layer->map_depth) : (layer->map_depth / parent_layer->map_depth);
	}
	else {
		kernel_width = abs(parent_layer->map_width - layer->map_width) + 1;
		kernel_height = abs(parent_layer->map_height - layer->map_height) + 1;
		kernel_depth = abs(parent_layer->map_depth - layer->map_depth) + 1;
	}
	kernel_size = kernel_depth * kernel_height * kernel_width;

	// set stride size if specified
	if (const char *stride_size = strstr(properties.c_str(), "stride")) {
		const char *end = strstr(stride_size, ")");

		stride_width = atoi(stride_size + 7);
		stride_size = strstr(stride_size, "x");

		if (stride_size && stride_size < end && atoi(stride_size + 1) > 0) {
			stride_height = atoi(stride_size + 1);
			stride_size = strstr(stride_size + 1, "x");

			if (stride_size && stride_size < end && atoi(stride_size + 1) > 0) {
				stride_depth = atoi(stride_size + 1);
			}
			else {
				stride_depth = 1;
			}
		}
		else {
			stride_height = 1;
			stride_depth = 1;
		}
	}
	else if (properties[0] == 'P') {
		stride_width = (parent_layer->map_width > layer->map_width) ? (parent_layer->map_width / layer->map_width) : (layer->map_width / parent_layer->map_width);
		stride_height = (parent_layer->map_height > layer->map_height) ? (parent_layer->map_height / layer->map_height) : (layer->map_height / parent_layer->map_height);
		stride_depth = (parent_layer->map_depth > layer->map_depth) ? (parent_layer->map_depth / layer->map_depth) : (layer->map_depth / parent_layer->map_depth);
	}
	else {
		stride_width = 1;
		stride_height = 1;
		stride_depth = 1;
	}

	// allocate memory for the weight, if necessary
	if (properties[0] == 'W') {
		for (int j = 0, index = 0; j < layer->number_maps; j++) {
			for (int k = 0; k < parent_layer->number_maps; k++) {
				if (!strstr(properties.c_str(), "depthwise") || j % parent_layer->number_maps == k) {
					for (int l = 0; l < kernel_size; l++) {
						weight_index.insert(pair<int, int>(j * parent_layer->number_maps * kernel_size + k * kernel_size + l, index++));
					}
					number_weights += kernel_size;
				}
			}
		}
		memset(weight = new float[number_weights], 0, sizeof(float) * number_weights);
	}

	if (properties[0] == 'P' || properties[0] == 'W') {
		int offset[3] = { kernel_depth - (abs(layer->map_depth * stride_depth - parent_layer->map_depth) + 1), kernel_height - (abs(layer->map_height * stride_height - parent_layer->map_height) + 1), kernel_width - (abs(layer->map_width * stride_width - parent_layer->map_width) + 1) };

		from_error = new vector<Index>[parent_layer->number_nodes];
		from_neuron = new vector<Index>[layer->number_nodes];
		from_weight = new vector<Index>[number_weights];

		for (int j = 0; j < layer->number_maps; j++) {
			for (int k = 0; k < layer->map_depth; k++) {
				for (int l = 0; l < layer->map_height; l++) {
					for (int m = 0; m < layer->map_width; m++) {
						int node_index[2] = { j * layer->map_size + k * layer->map_height * layer->map_width + l * layer->map_width + m, };

						if (properties[0] == 'W') {
							for (int n = 0; n < parent_layer->number_maps; n++) {
								if (!strstr(properties.c_str(), "depthwise") || j % parent_layer->number_maps == n) {
									int distance[3];

									for (int o = 0; o < parent_layer->map_depth; o++) {
										distance[0] = (layer->map_depth < parent_layer->map_depth) ? (o - k * stride_depth) : (k - o * stride_depth);
										if (-offset[0] <= distance[0] && distance[0] < kernel_depth - offset[0]) {
											for (int p = 0; p < parent_layer->map_height; p++) {
												distance[1] = (layer->map_height < parent_layer->map_height) ? (p - l * stride_height) : (l - p * stride_height);
												if (-offset[1] <= distance[1] && distance[1] < kernel_height - offset[1]) {
													for (int q = 0; q < parent_layer->map_width; q++) {
														distance[2] = (layer->map_width < parent_layer->map_width) ? (q - m * stride_width) : (m - q * stride_width);
														if (-offset[2] <= distance[2] && distance[2] < kernel_width - offset[2]) {
															Index index;

															node_index[1] = n * parent_layer->map_size + o * parent_layer->map_height * parent_layer->map_width + p * parent_layer->map_width + q;

															index.prev_node = node_index[1];
															index.next_node = node_index[0];
															index.weight = weight_index.find(j * parent_layer->number_maps * kernel_size + n * kernel_size + (distance[0] + offset[0]) * kernel_height * kernel_width + (distance[1] + offset[1]) * kernel_width + (distance[2] + offset[2]))->second;

															from_error[node_index[1]].push_back(index);
															from_neuron[node_index[0]].push_back(index);
															from_weight[index.weight].push_back(index);
														}
													}
												}
											}
										}
									}
								}
							}
						}
						else if (properties[0] == 'P') {
							int distance[3];

							for (int o = 0; o < parent_layer->map_depth; o++) {
								distance[0] = (layer->map_depth < parent_layer->map_depth) ? (o - k * stride_depth) : (k - o * stride_depth);
								if (0 <= distance[0] && distance[0] < kernel_depth) {
									for (int p = 0; p < parent_layer->map_height; p++) {
										distance[1] = (layer->map_height < parent_layer->map_height) ? (p - l * stride_height) : (l - p * stride_height);
										if (0 <= distance[1] && distance[1] < kernel_height) {
											for (int q = 0; q < parent_layer->map_width; q++) {
												distance[2] = (layer->map_width < parent_layer->map_width) ? (q - m * stride_width) : (m - q * stride_width);
												if (0 <= distance[2] && distance[2] < kernel_width) {
													Index index;

													node_index[1] = j * parent_layer->map_size + o * parent_layer->map_height * parent_layer->map_width + p * parent_layer->map_width + q;

													index.prev_node = node_index[1];
													index.next_node = node_index[0];

													from_error[node_index[1]].push_back(index);
													from_neuron[node_index[0]].push_back(index);
												}
											}
										}
									}
								}
							}
						}
					}
				}
			}
		}
	}
}
Connection::~Connection() {
	if (from_error) {
		delete[] from_error;
	}
	if (from_neuron) {
		delete[] from_neuron;
	}
	if (from_weight) {
		delete[] from_weight;
	}
	if (initializer) {
		delete initializer;
	}
	if (optimizer) {
		delete optimizer;
	}
	delete[] weight;
}

void Connection::Initialize() {
	if (initializer) {
		int kernel_size = this->kernel_size;
		int number_maps[] = { parent_layer->number_maps, layer->number_maps };
		int pool_size[] = { 1, 1 };

		Layer *layer = this->layer;
		Layer *parent_layer = this->parent_layer;

		if (strstr(properties.c_str(), "depthwise") && layer->Search_Child_Connection("pointwise")) {
			layer = layer->Search_Child_Connection("pointwise")->layer;
			number_maps[1] = layer->number_maps / parent_layer->number_maps;
			number_maps[0] = 1;
		}
		if (strstr(properties.c_str(), "pointwise") && parent_layer->Search_Connection("depthwise")) {
			kernel_size = parent_layer->Search_Connection("depthwise")->kernel_size;
			parent_layer = parent_layer->Search_Connection("depthwise")->parent_layer;

			number_maps[1] = layer->number_maps / parent_layer->number_maps;
			number_maps[0] = 1;
		}
		if (parent_layer->connection.size() > 0 && parent_layer->Search_Connection("P")) {
			pool_size[0] = parent_layer->Search_Connection("P")->kernel_size;
		}
		if (layer->child_connection.size() > 0 && layer->Search_Child_Connection("P")) {
			pool_size[1] = layer->Search_Child_Connection("P")->kernel_size;
		}
		initializer->Random(number_weights, weight, number_maps[0] * kernel_size * pool_size[0], number_maps[1] * kernel_size / pool_size[1]);
	}
}
void Connection::Optimizer(::Optimizer *optimizer) {
	if (this->optimizer) {
		delete this->optimizer;
	}
	this->optimizer = optimizer->Copy(number_weights);
}

Connection *Connection::Initializer(::Initializer initializer) {
	if (this->initializer) {
		delete this->initializer;
	}
	this->initializer = initializer.Copy();
	return this;
}


Initializer::Initializer(double value) {
	generator = nullptr;
	this->value = value;
	type = 0;
}
Initializer::Initializer(GlorotNormal initializer) {
	generator = initializer.generator;
	seed = initializer.seed;
	type = 4;
}
Initializer::Initializer(GlorotUniform initializer) {
	generator = initializer.generator;
	seed = initializer.seed;
	type = 3;
}
Initializer::Initializer(HeNormal initializer) {
	generator = initializer.generator;
	seed = initializer.seed;
	type = 6;
}
Initializer::Initializer(HeUniform initializer) {
	generator = initializer.generator;
	seed = initializer.seed;
	type = 5;
}
Initializer::Initializer(RandomNormal initializer) {
	generator = initializer.generator;
	mean = initializer.mean;
	stdv = initializer.stdv;
	seed = initializer.seed;
	type = 2;
}
Initializer::Initializer(RandomUniform initializer) {
	generator = initializer.generator;
	max = initializer.max;
	min = initializer.min;
	seed = initializer.seed;
	type = 1;
}
Initializer::~Initializer() {
	delete generator;
}

void Initializer::Random(int memory_size, float memory[], int fan_in, int fan_out) {
	if (type == 0) { // Constant
		for (int i = 0; i < memory_size; i++) {
			memory[i] = value;
		}
	}
	else if (type == 1) { // RandomUniform
		uniform_real_distribution<double> distribution(min, max);

		for (int i = 0; i < memory_size; i++) {
			memory[i] = distribution(*generator);
		}
	}
	else if (type == 2) { // RandomNormal
		normal_distribution<double> distribution(mean, stdv);

		for (int i = 0; i < memory_size; i++) {
			memory[i] = distribution(*generator);
		}
	}
	else if (type == 3) { // GlorotUniform
		double range = sqrt(6.0 / (fan_in + fan_out));

		uniform_real_distribution<double> distribution(-range, range);

		for (int i = 0; i < memory_size; i++) {
			memory[i] = distribution(*generator);
		}
	}
	else if (type == 4) { // GlorotNormal
		double stdv = sqrt(2.0 / (fan_in + fan_out));

		normal_distribution<double> distribution(0, stdv);

		for (int i = 0; i < memory_size; i++) {
			memory[i] = distribution(*generator);
		}
	}
	else if (type == 5) { // HeUniform
		double range = sqrt(6.0 / fan_in);

		uniform_real_distribution<double> distribution(-range, range);

		for (int i = 0; i < memory_size; i++) {
			memory[i] = distribution(*generator);
		}
	}
	else if (type == 6) { // HeNormal
		double stdv = sqrt(2.0 / fan_in);

		normal_distribution<double> distribution(0, stdv);

		for (int i = 0; i < memory_size; i++) {
			memory[i] = distribution(*generator);
		}
	}
}

Initializer* Initializer::Copy() {
	switch (type) {
	case 0: return new Initializer(value);
	case 1: return new Initializer(RandomUniform(min, max, seed));
	case 2: return new Initializer(RandomNormal(stdv, mean, seed));
	case 3: return new Initializer(GlorotUniform(seed));
	case 4: return new Initializer(GlorotNormal(seed));
	case 5: return new Initializer(HeUniform(seed));
	case 6: return new Initializer(HeNormal(seed));
	}
}


Layer::Layer(int number_maps, int map_width, int map_height, int map_depth, string properties) {
	this->activation = Activation::linear;
	this->batch_normalization = nullptr;
	this->batch_size = 1;
	this->initializer = nullptr;
	this->map_width = map_width;
	this->map_height = map_height;
	this->map_depth = map_depth;
	this->map_size = map_depth * map_height * map_width;
	this->number_maps = number_maps;
	this->number_nodes = number_maps * map_depth * map_height * map_width;
	this->optimizer = nullptr;
	this->properties = properties;

	bias = nullptr;
	error = new float[number_nodes];
	neuron = new float[number_nodes];
}
Layer::~Layer() {
	if (batch_normalization) {
		delete batch_normalization;
	}
	if (bias) {
		delete[] bias;
	}
	if (initializer) {
		delete initializer;
	}
	if (optimizer) {
		delete optimizer;
	}
	for (int i = 0; i < connection.size(); i++) {
		delete connection[i];
	}
	delete[] error;
	delete[] neuron;
}

void Layer::Activate(bool training) {
	if (bias) {
		#pragma omp parallel for
		for (int h = 0; h < batch_size; h++) {
			float *neuron = &this->neuron[h * number_nodes];

			for (int j = 0; j < number_maps; j++) {
				for (int k = 0; k < map_size; k++) {
					neuron[j * map_size + k] += bias[j];
				}
			}
		}
	}
	if (batch_normalization) {
		batch_normalization->Activate(neuron, training);
	}

	#pragma omp parallel for
	for (int h = 0; h < batch_size; h++) {
		float *neuron = &this->neuron[h * number_nodes];

		if (activation == Activation::softmax) {
			double max;
			double sum = 0;

			for (int j = 0; j < number_nodes; j++) {
				if (j == 0 || max < neuron[j]) {
					max = neuron[j];
				}
			}
			for (int j = 0; j < number_nodes; j++) {
				sum += (neuron[j] = exp(neuron[j] - max));
			}
			for (int j = 0; j < number_nodes; j++) {
				neuron[j] /= sum;
			}
		}
		else {
			if (activation == Activation::linear) {
				// neuron = neuron;
			}
			else if (activation == Activation::relu) {
				for (int j = 0; j < number_nodes; j++) {
					neuron[j] = (neuron[j] > 0) ? (neuron[j]) : (0);
				}
			}
			else if (activation == Activation::sigmoid) {
				for (int j = 0; j < number_nodes; j++) {
					neuron[j] = 1 / (1 + exp(-neuron[j]));
				}
			}
			else if (activation == Activation::tanh) {
				for (int j = 0; j < number_nodes; j++) {
					neuron[j] = 2 / (1 + exp(-2 * neuron[j])) - 1;
				}
			}

			if (strstr(properties.c_str(), "dropout")) {
				double rate = atof(strstr(properties.c_str(), "dropout") + 7);

				for (int j = 0; j < number_nodes; j++) {
					neuron[j] *= (training) ? (mask[h * number_nodes + j]) : (1 - rate);
				}
			}
		}
	}
}
void Layer::Adjust_Parameter(int iterations) {
	if (batch_normalization) {
		batch_normalization->Adjust_Parameter(iterations);
	}

	// adjust bias
	if (bias) {
		#pragma omp parallel for
		for (int j = 0; j < number_maps; j++) {
			double sum = 0;

			for (int h = 0; h < batch_size; h++) {
				for (int k = 0; k < map_size; k++) {
					sum += error[h * number_nodes + j * map_size + k];
				}
			}
			bias[j] += optimizer->Calculate_Gradient(j, sum, iterations);
		}
	}

	// adjust weight
	for (int i = 0; i < connection.size(); i++) {
		Connection *connection = this->connection[i];

		if (connection->properties[0] == 'W') {
			Layer *parent_layer = connection->parent_layer;

			#pragma omp parallel for
			for (int j = 0; j < connection->number_weights; j++) {
				double sum = 0;

				vector<Index> &from_weight = connection->from_weight[j];

				for (int h = 0; h < batch_size; h++) {
					float *error = &this->error[h * number_nodes];
					float *neuron = &parent_layer->neuron[h * parent_layer->number_nodes];

					for (auto index = from_weight.begin(); index != from_weight.end(); index++) {
						sum += error[index->next_node] * neuron[index->prev_node];
					}
				}
				connection->weight[j] += connection->optimizer->Calculate_Gradient(j, sum, iterations);
			}
		}
	}
}
void Layer::Backward() {
	memset(error, 0, sizeof(float) * batch_size * number_nodes);

	for (int k = 0; k < child_connection.size(); k++) {
		Connection *connection = child_connection[k];

		Layer *child_layer = connection->layer;

		if (connection->properties[0] == 'P') {
			if (strstr(connection->properties.c_str(), "average")) {
				#pragma omp parallel for
				for (int h = 0; h < batch_size; h++) {
					float *error = &this->error[h * number_nodes];
					float *next_error = &child_layer->error[h * child_layer->number_nodes];

					for (int j = 0; j < number_nodes; j++) {
						double sum = 0;

						vector<Index> &from_error = connection->from_error[j];

						for (auto index = from_error.begin(); index != from_error.end(); index++) {
							sum += next_error[index->next_node] / connection->from_neuron[index->next_node].size();
						}
						error[j] += sum;
					}
				}
			}
			else if (strstr(connection->properties.c_str(), "max")) {
				#pragma omp parallel for
				for (int h = 0; h < batch_size; h++) {
					float *error = &this->error[h * number_nodes];
					float *neuron = &this->neuron[h * number_nodes];
					float *next_error = &child_layer->error[h * child_layer->number_nodes];
					float *next_neuron = &child_layer->neuron[h * child_layer->number_nodes];

					for (int j = 0; j < number_nodes; j++) {
						double sum = 0;

						vector<Index> &from_error = connection->from_error[j];

						for (auto index = from_error.begin(); index != from_error.end(); index++) {
							if (neuron[j] == next_neuron[index->next_node]) {
								sum += next_error[index->next_node];
							}
						}
						error[j] += sum;
					}
				}
			}
		}
		else if (connection->properties[0] == 'W') {
			#pragma omp parallel for
			for (int h = 0; h < batch_size; h++) {
				float *error = &this->error[h * number_nodes];
				float *next_error = &child_layer->error[h * child_layer->number_nodes];

				for (int j = 0; j < number_nodes; j++) {
					double sum = 0;

					vector<Index> &from_error = connection->from_error[j];

					for (auto index = from_error.begin(); index != from_error.end(); index++) {
						sum += next_error[index->next_node] * connection->weight[index->weight];
					}
					error[j] += sum;
				}
			}
		}
	}
}
void Layer::Differentiate(int loss, float **y_batch) {
	if (y_batch) {
		#pragma omp parallel for
		for (int h = 0; h < batch_size; h++) {
			float *error = &this->error[h * number_nodes];
			float *neuron = &this->neuron[h * number_nodes];

			if (loss == Loss::cross_entropy) {
				if (activation == Activation::sigmoid) {
					for (int j = 0; j < number_nodes; j++) {
						error[j] = (neuron[j] - y_batch[h][j]) / (number_nodes * batch_size);
					}
				}
				else if (activation == Activation::softmax) {
					for (int j = 0; j < number_nodes; j++) {
						error[j] = (neuron[j] - y_batch[h][j]) / batch_size;
					}
				}
			}
			else if (loss == Loss::mean_squared_error) {
				for (int j = 0; j < number_nodes; j++) {
					error[j] = 2 * (neuron[j] - y_batch[h][j]) / (number_nodes * batch_size);
				}
			}
		}
		Differentiate(loss);
	}
	else {
		#pragma omp parallel for
		for (int h = 0; h < batch_size; h++) {
			float *error = &this->error[h * number_nodes];
			float *neuron = &this->neuron[h * number_nodes];

			if (activation == Activation::linear) {
				// error *= 1;
			}
			else if (activation == Activation::relu) {
				for (int j = 0; j < number_nodes; j++) {
					error[j] *= (neuron[j] > 0);
				}
			}
			else if (activation == Activation::sigmoid && loss != Loss::cross_entropy) {
				for (int j = 0; j < number_nodes; j++) {
					error[j] *= (1 - neuron[j]) * neuron[j];
				}
			}
			else if (activation == Activation::tanh) {
				for (int j = 0; j < number_nodes; j++) {
					error[j] *= (1 - neuron[j]) * (1 + neuron[j]);
				}
			}

			if (strstr(properties.c_str(), "dropout")) {
				for (int j = 0; j < number_nodes; j++) {
					error[j] *= mask[h * number_nodes + j];
				}
			}
		}
		if (batch_normalization) {
			batch_normalization->Differentiate(error);
		}
	}
}
void Layer::Forward() {
	memset(neuron, 0, sizeof(float) * batch_size * number_nodes);

	for (int k = 0; k < connection.size(); k++) {
		Connection *connection = this->connection[k];

		Layer *parent_layer = connection->parent_layer;

		if (connection->properties[0] == 'P') {
			if (strstr(connection->properties.c_str(), "average")) {
				#pragma omp parallel for
				for (int h = 0; h < batch_size; h++) {
					float *neuron = &this->neuron[h * number_nodes];
					float *prev_neuron = &parent_layer->neuron[h * parent_layer->number_nodes];

					for (int j = 0; j < number_nodes; j++) {
						double sum = 0;

						vector<Index> &from_neuron = connection->from_neuron[j];

						for (auto index = from_neuron.begin(); index != from_neuron.end(); index++) {
							sum += prev_neuron[index->prev_node];
						}
						neuron[j] += sum / from_neuron.size();
					}
				}
			}
			else if(strstr(connection->properties.c_str(), "max")) {
				#pragma omp parallel for
				for (int h = 0; h < batch_size; h++) {
					float *neuron = &this->neuron[h * number_nodes];
					float *prev_neuron = &parent_layer->neuron[h * parent_layer->number_nodes];

					for (int j = 0; j < number_nodes; j++) {
						double max = 0;

						vector<Index> &from_neuron = connection->from_neuron[j];

						for (auto index = from_neuron.begin(); index != from_neuron.end(); index++) {
							if (index == from_neuron.begin() || max < prev_neuron[index->prev_node]) {
								max = prev_neuron[index->prev_node];
							}
						}
						neuron[j] += max;
					}
				}
			}
		}
		else if (connection->properties[0] == 'W') {
			#pragma omp parallel for
			for (int h = 0; h < batch_size; h++) {
				float *neuron = &this->neuron[h * number_nodes];
				float *prev_neuron = &parent_layer->neuron[h * parent_layer->number_nodes];

				for (int j = 0; j < number_nodes; j++) {
					double sum = 0;

					vector<Index> &from_neuron = connection->from_neuron[j];

					for (auto index = from_neuron.begin(); index != from_neuron.end(); index++) {
						sum += prev_neuron[index->prev_node] * connection->weight[index->weight];
					}
					neuron[j] += sum;
				}
			}
		}
	}
}
void Layer::Initialize() {
	if (batch_normalization) {
		batch_normalization->Initialize();
	}
	if (bias) {
		initializer->Random(number_maps, bias, 1, number_maps);
	}
}
void Layer::Optimizer(::Optimizer *optimizer) {
	if (batch_normalization) {
		batch_normalization->Optimizer(*optimizer);
	}
	if (bias) {
		if (this->optimizer) {
			delete this->optimizer;
		}
		this->optimizer = optimizer->Copy(number_maps);
	}
}
void Layer::Resize_Memory(int batch_size) {
	if (this->batch_size != batch_size) {
		if (batch_normalization) {
			batch_normalization->Resize_Memory(batch_size);
		}
		error = (float*)realloc(error, sizeof(float) * batch_size * number_nodes);
		neuron = (float*)realloc(neuron, sizeof(float) * batch_size * number_nodes);

		this->batch_size = batch_size;
	}
}

Batch_Normalization* Layer::Batch_Normalization(double epsilon, double momentum) {
	if (batch_normalization) {
		delete batch_normalization;
	}
	return (batch_normalization = new ::Batch_Normalization(number_maps, map_size, epsilon, momentum, this));
}

Connection* Layer::Search_Child_Connection(string properties) {
	for (int i = 0; i < child_connection.size(); i++) {
		if (strstr(child_connection[i]->properties.c_str(), properties.c_str())) {
			return child_connection[i];
		}
	}
	return nullptr;
}
Connection* Layer::Search_Connection(string properties) {
	for (int i = 0; i < connection.size(); i++) {
		if (strstr(connection[i]->properties.c_str(), properties.c_str())) {
			return connection[i];
		}
	}
	return nullptr;
}

Layer* Layer::Activation(int activation) {
	this->activation = activation;
	return this;
}
Layer* Layer::Initializer(::Initializer initializer) {
	if (bias == nullptr) {
		bias = new float[number_maps];
	}
	if (this->initializer) {
		delete this->initializer;
	}
	this->initializer = initializer.Copy();
	return this;
}


void Optimizer::Construct(int type, double learning_rate, double momentum, double decay, int number_parameters) {
	this->decay = decay;
	this->gradient = nullptr;
	this->learning_rate = learning_rate;
	this->momentum = momentum;
	this->type = type;

	if (type) {
		memset(gradient = new float[number_parameters], 0, sizeof(float) * number_parameters);
	}
}

Optimizer::Optimizer(int type, double learning_rate, double momentum, double decay, int number_parameters) {
	Construct(type, learning_rate, momentum, decay, number_parameters);
}
Optimizer::Optimizer(SGD SGD) {
	Construct(0, SGD.learning_rate, 0, SGD.decay, 0);
}
Optimizer::Optimizer(Momentum Momentum) {
	Construct(1, Momentum.learning_rate, Momentum.momentum, Momentum.decay, 0);
}
Optimizer::Optimizer(Nesterov Nesterov) {
	Construct(2, Nesterov.learning_rate, Nesterov.momentum, Nesterov.decay, 0);
}
Optimizer::~Optimizer() {
	if (gradient) {
		delete[] gradient;
	}
}

double Optimizer::Calculate_Gradient(int index, double gradient, int iterations) {
	double learning_rate = this->learning_rate / (1 + decay * iterations);
	double output = 0;

	switch (type) {
	case 0: // SGD
		output = -learning_rate * gradient;
		break;
	case 1: // Momentum
		output = this->gradient[index] * momentum - learning_rate * gradient;
		this->gradient[index] = output;
		break;
	case 2: // Nesterov
		output = (this->gradient[index] * momentum - learning_rate * gradient) * momentum - learning_rate * gradient;
		this->gradient[index] = output;
		break;
	}
	return output;
}

Optimizer* Optimizer::Copy(int number_parameters) {
	return new Optimizer(type, learning_rate, momentum, decay, number_parameters);
}


void Neural_Networks::Resize_Memory(int batch_size) {
	if (this->batch_size != batch_size) {
		for (int i = 0; i < layer.size(); i++) {
			layer[i]->Resize_Memory(batch_size);
		}
		this->batch_size = batch_size;
	}
}

double Neural_Networks::Calculate_Loss(Layer *layer, float **y_batch) {
	double sum = 0, *batch_sum = new double[batch_size];

	memset(batch_sum, 0, sizeof(double) * batch_size);

	#pragma omp parallel for
	for (int h = 0; h < batch_size; h++) {
		float *neuron = &layer->neuron[h * layer->number_nodes];

		if (loss == Loss::cross_entropy) {
			if (layer->activation == Activation::sigmoid) {
				for (int j = 0; j < layer->number_nodes; j++) {
					batch_sum[h] -= (y_batch[h][j] * log(neuron[j] + 0.00000001) + (1 - y_batch[h][j]) * log(1.00000001 - neuron[j])) / layer->number_nodes;
				}
			}
			else if (layer->activation == Activation::softmax) {
				for (int j = 0; j < layer->number_nodes; j++) {
					batch_sum[h] -= y_batch[h][j] * log(neuron[j] + 0.00000001);
				}
			}
		}
		else if (loss == Loss::mean_squared_error) {
			for (int j = 0; j < layer->number_nodes; j++) {
				batch_sum[h] += (neuron[j] - y_batch[h][j]) * (neuron[j] - y_batch[h][j]) / layer->number_nodes;
			}
		}
	}
	for (int h = 0; h < batch_size; h++) {
		sum += batch_sum[h];
	}
	return sum;
}

Neural_Networks::Neural_Networks() {
	batch_size = 1;
	optimizer = nullptr;
}
Neural_Networks::~Neural_Networks() {
	if (optimizer) {
		delete optimizer;
	}
	for (int i = 0; i < layer.size(); i++) {
		delete layer[i];
	}
}

void Neural_Networks::Compile(int loss, Optimizer *optimizer) {
	if (this->optimizer) {
		delete this->optimizer;
	}
	this->iterations = 0;
	this->loss = loss;
	this->optimizer = optimizer;

	for (int i = 0; i < connection.size(); i++) {
		if (connection[i]->properties[0] == 'W') {
			connection[i]->Optimizer(optimizer);
			connection[i]->Initialize();
		}
	}
	for (int i = 0; i < layer.size(); i++) {
		layer[i]->Optimizer(optimizer);
		layer[i]->Initialize();
	}
}
void Neural_Networks::Load_Weights(string path) {
	ifstream file(path);

	if (file.is_open()) {
		for (int i = 0; i < layer.size(); i++) {
			Layer *layer = this->layer[i];

			if (layer->batch_normalization) {
				layer->batch_normalization->Load(file);
			}
			if (layer->bias) {
				for (int j = 0; j < layer->number_maps; j++) {
					file >> layer->bias[j];
				}
			}
		}
		for (int i = 0; i < connection.size(); i++) {
			Connection *connection = this->connection[i];

			if (connection->properties[0] == 'W') {
				for (int j = 0; j < connection->number_weights; j++) {
					file >> connection->weight[j];
				}
			}
		}
		file.close();
	}
	else {
		cerr << "[Load_Weights], " + path + " not found" << endl;
	}
}
void Neural_Networks::Save_Weights(string path) {
	ofstream file(path);

	for (int i = 0; i < layer.size(); i++) {
		Layer *layer = this->layer[i];

		if (layer->batch_normalization) {
			layer->batch_normalization->Save(file);
		}
		if (layer->bias) {
			for (int j = 0; j < layer->number_maps; j++) {
				file << layer->bias[j] << endl;
			}
		}
	}
	for (int i = 0; i < connection.size(); i++) {
		Connection *connection = this->connection[i];

		if (connection->properties[0] == 'W') {
			for (int j = 0; j < connection->number_weights; j++) {
				file << connection->weight[j] << endl;
			}
		}
	}
	file.close();
}
void Neural_Networks::Predict(float input[], float output[]) {
	Predict(&input, &output);
}
void Neural_Networks::Predict(float **input, float **output, int batch_size) {
	Resize_Memory(batch_size);

	for (int h = 0, i = 0; h < batch_size; h++) {
		memcpy(&layer[i]->neuron[h * layer[i]->number_nodes], input[h], sizeof(float) * layer[i]->number_nodes);
	}
	for (int i = 1; i < layer.size(); i++) {
		layer[i]->Forward();
		layer[i]->Activate();
	}
	for (int h = 0, i = layer.size() - 1; h < batch_size; h++) {
		memcpy(output[h], &layer[i]->neuron[h * layer[i]->number_nodes], sizeof(float) * layer[i]->number_nodes);
	}
}

float** Neural_Networks::Shuffle(float **data, int data_size, int seed) {
	srand(seed);

	for (int i = 0, index; i < data_size; i++) {
		float *d = data[index = rand() % data_size];

		data[index] = data[i];
		data[i] = d;
	}
	return data;
}

double Neural_Networks::Evaluate(float **x_test, float **y_test, int test_size, int batch_size) {
	float **x_batch = new float*[batch_size];
	float **y_batch = new float*[batch_size];
	
	double loss = 0;

	for (int g = 0, h = 0; g < test_size; g++) {
		x_batch[h] = x_test[g];
		y_batch[h] = y_test[g];

		if (++h == batch_size || g == test_size - 1) {
			Resize_Memory(h);

			// copy x_test to neuron
			while (--h >= 0) {
				memcpy(&layer[0]->neuron[h * layer[0]->number_nodes], x_batch[h], sizeof(float) * layer[0]->number_nodes);
			}
			h = 0;

			// forward propagation
			for (int i = 1; i < layer.size(); i++) {
				layer[i]->Forward();
				layer[i]->Activate();
			}

			// calculate loss
			loss += Calculate_Loss(layer.back(), y_batch);
		}
	}
	delete[] x_batch;
	delete[] y_batch;

	return loss / test_size;
}
double Neural_Networks::Fit(float **x_train, float **y_train, int train_size, int batch_size) {
	float **x_batch = new float*[batch_size];
	float **y_batch = new float*[batch_size];
	
	double loss = 0;

	for (int g = 0, h = 0; g < train_size; g++) {
		x_batch[h] = x_train[g];
		y_batch[h] = y_train[g];

		if (++h == batch_size || g == train_size - 1) {
			Resize_Memory(h);

			// prepare dropout mask if specified
			for (int i = 1; i < layer.size(); i++) {
				Layer *layer = this->layer[i];

				if (strstr(layer->properties.c_str(), "dropout")) {
					double rate = atof(strstr(layer->properties.c_str(), "dropout") + 7);

					layer->mask = new bool[h * layer->number_nodes];

					for (int j = 0; j < h * layer->number_nodes; j++) {
						layer->mask[j] = (rand() >= rate * (RAND_MAX + 1));
					}
				}
			}

			// copy x_train to neuron
			while (--h >= 0) {
				memcpy(&layer[0]->neuron[h * layer[0]->number_nodes], x_batch[h], sizeof(float) * layer[0]->number_nodes);
			}
			h = 0;

			// forward propagation
			for (int i = 1; i < layer.size(); i++) {
				layer[i]->Forward();
				layer[i]->Activate(true);
			}

			// calculate loss
			loss += Calculate_Loss(layer.back(), y_batch);

			// calculate error
			layer.back()->Differentiate(this->loss, y_batch);

			// error backpropagation
			for (int i = layer.size() - 2; i > 0; i--) {
				layer[i]->Backward();
				layer[i]->Differentiate();
			}

			// adjust parameter
			for (int i = 0; i < layer.size(); i++) {
				layer[i]->Adjust_Parameter(iterations);
			}
			iterations++;

			for (int i = 1; i < layer.size(); i++) {
				Layer *layer = this->layer[i];

				if (strstr(layer->properties.c_str(), "dropout")) {
					delete[] layer->mask;
				}
			}
		}
	}
	delete[] x_batch;
	delete[] y_batch;

	return loss / train_size;
}

Connection* Neural_Networks::Connect(int from, int to, string properties) {
	Connection *connection = new Connection(layer[from], layer[to], properties);

	if (properties[0] == 'W'){
		if (!strstr(properties.c_str(), "depthwise") && layer[from]->bias == nullptr) {
			Layer *layer = this->layer[from];

			layer->bias = new float[layer->number_maps];
			layer->initializer = new Initializer(0);
		}
		connection->initializer = new Initializer(GlorotUniform());
	}
	if (properties[0] == 'P' && !(strstr(properties.c_str(), "average") || strstr(properties.c_str(), "max"))) {
		cerr << "[Connect], The pooling layer must have 'average' or 'max' property" << endl;
		return nullptr;
	}
	this->connection.push_back(connection);
	layer[from]->connection.push_back(connection);
	layer[to]->child_connection.push_back(connection);

	return connection;
}

Layer* Neural_Networks::Add(int number_nodes, string properties) {
	return Add(number_nodes, 1, 1, 1, properties);
}
Layer* Neural_Networks::Add(int number_maps, int map_width, string properties) {
	return Add(number_maps, map_width, 1, 1, properties);
}
Layer* Neural_Networks::Add(int number_maps, int map_width, int map_height, string properties) {
	return Add(number_maps, map_width, map_height, 1, properties);
}
Layer* Neural_Networks::Add(int number_maps, int map_width, int map_height, int map_depth, string properties) {
	Layer *layer = new Layer(number_maps, map_width, map_height, map_depth, properties);

	this->layer.push_back(layer);
	return layer;
}