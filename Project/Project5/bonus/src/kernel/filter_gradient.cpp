#include "filter_gradient.h"

#include <cassert>
#include <cmath>
#include <cstddef>
#include <cstdint>
#include <random>

void initialize_filter_gradient(filter_gradient_args* args,
                        std::size_t width,
                        std::size_t height,
                        std::uint_fast64_t seed) {
    if (!args) {
        return;
    }

    assert(width >= 3);
    assert(height >= 3);

    args->width = width;
    args->height = height;
    args->out = 0.0f;

    const std::size_t count = width * height;

    std::mt19937_64 gen(seed);
    std::uniform_real_distribution<float> dist(0.0f, 1.0f);

    args->data.a.resize(count);
    args->data.b.resize(count);
    args->data.c.resize(count);
    args->data.d.resize(count);
    args->data.e.resize(count);
    args->data.f.resize(count);
    args->data.g.resize(count);
    args->data.h.resize(count);
    args->data.i.resize(count);

    for (std::size_t k = 0; k < count; ++k) {
        args->data.a[k] = dist(gen);
        args->data.b[k] = dist(gen);
        args->data.c[k] = dist(gen);
        args->data.d[k] = dist(gen);
        args->data.e[k] = dist(gen);
        args->data.f[k] = dist(gen);
        args->data.g[k] = dist(gen);
        args->data.h[k] = dist(gen);
        args->data.i[k] = dist(gen);
    }
}

void naive_filter_gradient(float& out, const data_struct& data,
                   std::size_t width, std::size_t height) {
    const std::size_t W = width;
    const std::size_t H = height;
    constexpr float inv9 = 1.0f / 9.0f;

    double total = 0.0f;

    for (std::size_t y = 1; y + 1 < H; ++y) {
        for (std::size_t x = 1; x + 1 < W; ++x) {

            double sum_a = 0.0, sum_b = 0.0, sum_c = 0.0;
            for (int dy = -1; dy <= 1; ++dy) {
                const std::size_t row = (y + dy) * W;
                for (int dx = -1; dx <= 1; ++dx) {
                    const std::size_t idx = row + (x + dx);
                    sum_a += data.a[idx];
                    sum_b += data.b[idx];
                    sum_c += data.c[idx];
                }
            }
            const float avg_a = sum_a * inv9;
            const float avg_b = sum_b * inv9;
            const float avg_c = sum_c * inv9;
            const float p1 = avg_a * avg_b + avg_c;

            const std::size_t ym1 = (y - 1) * W;
            const std::size_t y0  = y * W;
            const std::size_t yp1 = (y + 1) * W;

            const std::size_t xm1 = x - 1;
            const std::size_t x0  = x;
            const std::size_t xp1 = x + 1;

            const float sobel_dx =
                -data.d[ym1 + xm1] + data.d[ym1 + xp1]
                -2.0f * data.d[y0 + xm1] + 2.0f * data.d[y0 + xp1]
                -data.d[yp1 + xm1] + data.d[yp1 + xp1];

            const float sobel_ex =
                -data.e[ym1 + xm1] + data.e[ym1 + xp1]
                -2.0f * data.e[y0 + xm1] + 2.0f * data.e[y0 + xp1]
                -data.e[yp1 + xm1] + data.e[yp1 + xp1];

            const float sobel_fx =
                -data.f[ym1 + xm1] + data.f[ym1 + xp1]
                -2.0f * data.f[y0 + xm1] + 2.0f * data.f[y0 + xp1]
                -data.f[yp1 + xm1] + data.f[yp1 + xp1];

            const float p2 = sobel_dx * sobel_ex + sobel_fx;

            const float sobel_gy =
                -data.g[ym1 + xm1] - 2.0f * data.g[ym1 + x0] - data.g[ym1 + xp1]
                + data.g[yp1 + xm1] + 2.0f * data.g[yp1 + x0] + data.g[yp1 + xp1];

            const float sobel_hy =
                -data.h[ym1 + xm1] - 2.0f * data.h[ym1 + x0] - data.h[ym1 + xp1]
                + data.h[yp1 + xm1] + 2.0f * data.h[yp1 + x0] + data.h[yp1 + xp1];

            const float sobel_iy =
                -data.i[ym1 + xm1] - 2.0f * data.i[ym1 + x0] - data.i[ym1 + xp1]
                + data.i[yp1 + xm1] + 2.0f * data.i[yp1 + x0] + data.i[yp1 + xp1];

            const float p3 = sobel_gy * sobel_hy + sobel_iy;

            total += p1 + p2 + p3;
        }
    }

    out = total;
}

void convert_soa_to_aos(filter_gradient_args& args) {
    if (args.is_converted) return;
    const std::size_t count = args.width * args.height;
    args.aos_data.resize(count);
    auto* px = args.aos_data.data();
    for (std::size_t k = 0; k < count; ++k)
    {
        px[k].a = args.data.a[k];
        px[k].b = args.data.b[k];
        px[k].c = args.data.c[k];
        px[k].d = args.data.d[k];
        px[k].e = args.data.e[k];
        px[k].f = args.data.f[k];
        px[k].g = args.data.g[k];
        px[k].h = args.data.h[k];
        px[k].i = args.data.i[k];
    }
    args.is_converted = true;
}

void stu_filter_gradient(float& out, const std::vector<filter_gradient_args::Pixel>& aos_data,
                   std::size_t width, std::size_t height) {
    const std::size_t W = width;
    const std::size_t H = height;
    constexpr float inv9 = 1.0f / 9.0f;
    double total = 0.0;
    const auto* px = aos_data.data();
    #pragma omp parallel for reduction(+:total)
    for (std::size_t y = 1; y < H - 1; ++y)
    {
        const auto* row_m1 = px + (y - 1) * W;
        const auto* row_0  = px + y * W;
        const auto* row_p1 = px + (y + 1) * W;
        double row_total = 0.0; 
        for (std::size_t x = 1; x + 1 < W; ++x)
        {
            float sum_a = row_m1[x-1].a + row_m1[x].a + row_m1[x+1].a
                        + row_0[x-1].a  + row_0[x].a  + row_0[x+1].a
                        + row_p1[x-1].a + row_p1[x].a + row_p1[x+1].a;
            float sum_b = row_m1[x-1].b + row_m1[x].b + row_m1[x+1].b
                        + row_0[x-1].b  + row_0[x].b  + row_0[x+1].b
                        + row_p1[x-1].b + row_p1[x].b + row_p1[x+1].b;
            float sum_c = row_m1[x-1].c + row_m1[x].c + row_m1[x+1].c
                        + row_0[x-1].c  + row_0[x].c  + row_0[x+1].c
                        + row_p1[x-1].c + row_p1[x].c + row_p1[x+1].c;
            float p1 = (sum_a * inv9) * (sum_b * inv9) + (sum_c * inv9);
            float sobel_dx = -row_m1[x-1].d + row_m1[x+1].d
                             -2.0f * row_0[x-1].d + 2.0f * row_0[x+1].d
                             -row_p1[x-1].d + row_p1[x+1].d;
            float sobel_ex = -row_m1[x-1].e + row_m1[x+1].e
                             -2.0f * row_0[x-1].e + 2.0f * row_0[x+1].e
                             -row_p1[x-1].e + row_p1[x+1].e;
            float sobel_fx = -row_m1[x-1].f + row_m1[x+1].f
                             -2.0f * row_0[x-1].f + 2.0f * row_0[x+1].f
                             -row_p1[x-1].f + row_p1[x+1].f;
            float p2 = sobel_dx * sobel_ex + sobel_fx;
            float sobel_gy = -row_m1[x-1].g - 2.0f * row_m1[x].g - row_m1[x+1].g
                             +row_p1[x-1].g + 2.0f * row_p1[x].g + row_p1[x+1].g;
            float sobel_hy = -row_m1[x-1].h - 2.0f * row_m1[x].h - row_m1[x+1].h
                             +row_p1[x-1].h + 2.0f * row_p1[x].h + row_p1[x+1].h;
            float sobel_iy = -row_m1[x-1].i - 2.0f * row_m1[x].i - row_m1[x+1].i
                             +row_p1[x-1].i + 2.0f * row_p1[x].i + row_p1[x+1].i;
            float p3 = sobel_gy * sobel_hy + sobel_iy;
            row_total += p1 + p2 + p3;
        }
        total += row_total;
    }
    out = total;
}
void naive_filter_gradient_wrapper(void* ctx) {
    auto& args = *static_cast<filter_gradient_args*>(ctx);
    args.out = 0.0f;
    naive_filter_gradient(args.out, args.data, args.width, args.height);
}
void stu_filter_gradient_wrapper(void* ctx) {
    auto& args = *static_cast<filter_gradient_args*>(ctx);
    args.out = 0.0f;
    // convert_to_aos(args);
    stu_filter_gradient(args.out, args.aos_data, args.width, args.height);
}

bool filter_gradient_check(void* stu_ctx, void* ref_ctx, lab_test_func naive_func) {
    auto& stu_args = *static_cast<filter_gradient_args*>(stu_ctx);
    auto& ref_args = *static_cast<filter_gradient_args*>(ref_ctx);

    ref_args.out = 0.0f;
    naive_func(ref_ctx);

    const auto eps = ref_args.epsilon;
    const double s = static_cast<double>(stu_args.out);
    const double r = static_cast<double>(ref_args.out);
    const double err = std::abs(s - r);
    const double atol = 1e-6;
    const double rel = (std::abs(r) > atol) ? err / std::abs(r) : err;
    debug_log("DEBUG: filter_gradient stu={} ref={} err={} rel={}\n",
              stu_args.out,
              ref_args.out,
              err,
              rel);

    return err <= (atol + eps * std::abs(r));
}
