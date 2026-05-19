#include "graph.h"

#include <cmath>
#include <cstddef>
#include <cstdint>
#include <random>
#include <vector>


void initialize_graph(graph_args* args,
                       std::size_t node_count,
                       int avg_degree,
                       std::uint_fast64_t seed) {
    if (!args) {
        return;
    }

    std::mt19937_64 gen(seed);
    std::uniform_int_distribution<int> dist(0, static_cast<int>(node_count) - 1);

    args->nodes.assign(node_count, Node{nullptr});
    args->edge_storage.clear();
    args->edge_storage.resize(node_count * static_cast<std::size_t>(avg_degree));

    args->graph.n = static_cast<int>(node_count);
    args->graph.nodes = args->nodes.data();

    std::size_t edge_pos = 0;

    for (std::size_t u = 0; u < node_count; ++u) {
        std::vector<int> neighbors;
        neighbors.reserve(avg_degree);

        for (int k = 0; k < avg_degree; ++k) {
            neighbors.push_back(dist(gen));
        }

        Edge* head = nullptr;
        for (int k = avg_degree - 1; k >= 0; --k) {
            Edge& e = args->edge_storage[edge_pos + static_cast<std::size_t>(k)];
            e.to = neighbors[static_cast<std::size_t>(k)];
            e.next = head;
            head = &e;
        }

        args->nodes[u].edges = head;
        edge_pos += static_cast<std::size_t>(avg_degree);
    }

    args->out = 0;
}

void naive_graph(std::uint64_t& out, const Graph& graph) {
    std::uint64_t checksum = 0;
    for (int u = 0; u < graph.n; ++u) {
        const Edge* e = graph.nodes[u].edges;
        while (e) {
            checksum += static_cast<std::uint64_t>(e->to);
            e = e->next;
        }
    }
    out = checksum;
}

void stu_graph(std::uint64_t& out, const graph_args& args) {
    // TODO: You may need to add a function to convert data structure (not
    // included in time measurement), then implement your version in
    // stu_graph, whch is called by stu_graph_wrapper.
    uint64_t sum = 0;
    const auto& col_idx = args.csr_col_idx;
    const std::size_t total_edges = col_idx.size();
    const int* data = col_idx.data();
    for (std::size_t i = 0; i < total_edges; ++i)
        sum += static_cast<uint64_t>(data[i]);
    // for (int u = 0; u < n; ++u)
    //     for (int i = row_ptr[u]; i < row_ptr[u + 1]; ++i)
    //         sum += static_cast<uint64_t>(col_idx[i]);
    out = sum;
}

void naive_graph_wrapper(void* ctx) {
    auto& args = *static_cast<graph_args*>(ctx);
    naive_graph(args.out, args.graph);
}
void convert_graph_to_csr(graph_args& args)
{
    if (args.is_convert) return;
    int n = args.graph.n;
    args.csr_row_ptr.assign(n + 1, 0);
    for (int u = 0; u < n; ++u) {
        int degree = 0;
        for (const Edge* e = args.graph.nodes[u].edges; e; e = e->next) ++degree;
        args.csr_row_ptr[u + 1] = args.csr_row_ptr[u] + degree;
    }
    args.csr_col_idx.resize(args.csr_row_ptr[n]);
    std::vector<int> current = args.csr_row_ptr;
    for (int u = 0; u < n; ++u)
        for (const Edge* e = args.graph.nodes[u].edges; e; e = e->next)
            args.csr_col_idx[current[u]++] = e->to;
    args.is_convert = true;
}
void stu_graph_wrapper(void* ctx) {
    auto& args = *static_cast<graph_args*>(ctx);
    // convert_graph_to_csr (args);
    stu_graph(args.out, args);
}

bool graph_check(void* stu_ctx, void* ref_ctx, lab_test_func naive_func) {
    naive_func(ref_ctx);

    auto& stu_args = *static_cast<graph_args*>(stu_ctx);
    auto& ref_args = *static_cast<graph_args*>(ref_ctx);
    const auto eps = ref_args.epsilon;

    const double s = static_cast<double>(stu_args.out);
    const double r = static_cast<double>(ref_args.out);
    const double err = std::abs(s - r);
    const double atol = 0.0;
    const double rel = (std::abs(r) > 1e-12) ? err / std::abs(r) : err;

    debug_log("\tDEBUG: graph stu={} ref={} err={} rel={}\n",
              stu_args.out,
              ref_args.out,
              err,
              rel);

    return err <= (atol + eps * std::abs(r));
}
