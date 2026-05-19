#include "relu.h"
#include <algorithm>
#include <cstdint>
#include <random>

void initialize_relu(relu_args *args, const size_t size,
                     const std::uint_fast64_t seed) {
    if (!args) {
        return;
    }

    constexpr float mean = 0.0f;
    constexpr float stddev = 1.0f;

    std::mt19937_64 gen(seed);
    std::normal_distribution<float> dist(mean, stddev);

    args->data.resize(size);

    for (auto &value : args->data) {
        value = dist(gen);
    }
}

void naive_relu(std::span<float> data) {
    for (auto &&value : data) {
        if (value < 0.0f) {
            value = 0.0f;
        }
    }
}

void stu_relu(std::span<float> data) {
    // TODO: Implement your version, and call it in stu_relu_wrapper
    float* ptr = data.data();
    const std::size_t n = data.size();
    std::size_t pos = 0;
    while (pos + 8 < n)
    {
        ptr[pos] = (ptr[pos] + std::abs (ptr[pos])) * 0.5f;
        ptr[pos + 1] = (ptr[pos + 1] + std::abs(ptr[pos + 1])) * 0.5f;
        ptr[pos + 2] = (ptr[pos + 2] + std::abs(ptr[pos + 2])) * 0.5f;
        ptr[pos + 3] = (ptr[pos + 3] + std::abs(ptr[pos + 3])) * 0.5f;
        ptr[pos + 4] = (ptr[pos + 4] + std::abs(ptr[pos + 4])) * 0.5f;
        ptr[pos + 5] = (ptr[pos + 5] + std::abs(ptr[pos + 5])) * 0.5f;
        ptr[pos + 6] = (ptr[pos + 6] + std::abs(ptr[pos + 6])) * 0.5f;
        ptr[pos + 7] = (ptr[pos + 7] + std::abs(ptr[pos + 7])) * 0.5f;
        // ptr[pos] = std::max(0.0f, ptr[pos]);
        // ptr[pos + 1] = std::max(0.0f, ptr[pos + 1]);
        // ptr[pos + 2] = std::max(0.0f, ptr[pos + 2]);
        // ptr[pos + 3] = std::max(0.0f, ptr[pos + 3]);
        // ptr[pos + 4] = std::max(0.0f, ptr[pos + 4]);
        // ptr[pos + 5] = std::max(0.0f, ptr[pos + 5]);
        // ptr[pos + 6] = std::max(0.0f, ptr[pos + 6]);
        // ptr[pos + 7] = std::max(0.0f, ptr[pos + 7]);
        pos += 8;
    }
    while (pos < n) 
    {
        ptr[pos] = (ptr[pos] + std::abs (ptr[pos])) * 0.5f;
        // ptr[pos] = std::max(0.0f, ptr[pos]);
        ++pos;
    }
}

void naive_relu_wrapper(void *ctx) {
    auto &args = *static_cast<relu_args *>(ctx);
    naive_relu(args.data);
}

void stu_relu_wrapper(void *ctx) {
    auto &args = *static_cast<relu_args *>(ctx);
    stu_relu(args.data);
}

bool relu_check(void *stu_ctx, void *ref_ctx, lab_test_func naive_func) {
    // Compute reference
    naive_func(ref_ctx);

    auto &stu_args = *static_cast<relu_args *>(stu_ctx);
    auto &ref_args = *static_cast<relu_args *>(ref_ctx);
    const auto eps = ref_args.epsilon;

    if (stu_args.data.size() != ref_args.data.size()) {
        debug_log("\tDEBUG: size mismatch: stu={} ref={}\n",
                  stu_args.data.size(),
                  ref_args.data.size());
        return false;
    }

    double max_rel = 0.0;
    size_t worst_i = 0;
    const double atol = 1e-6;

    for (size_t i = 0; i < ref_args.data.size(); ++i) {
        const double r = static_cast<double>(ref_args.data[i]);
        const double s = static_cast<double>(stu_args.data[i]);
        const double err = std::abs(s - r);
        const double rel = (std::abs(r) > atol) ? err / std::abs(r) : err;

        if (rel > max_rel) {
            max_rel = rel;
            worst_i = i;
        }

        if (err > (atol + eps * std::abs(r))) {
            debug_log("\tDEBUG: fail at {}: ref={} stu={} err={} rel={} thr={}\n",
                      i,
                      ref_args.data[i],
                      stu_args.data[i],
                      err,
                      rel,
                      (atol + eps * std::abs(r)));
            return false;
        }
    }

    debug_log("\tDEBUG: relu_check passed. max_rel={} at i={}\n",
              max_rel,
              worst_i);
    return true;
}
