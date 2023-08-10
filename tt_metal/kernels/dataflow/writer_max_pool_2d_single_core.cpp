#include <cstdint>
#include "dataflow_api.h"
// #include "debug_print.h"


// SliceRange srt = SliceRange{ .h0 = 0, .h1 = 32, .hs = 8, .w0 = 0, .w1 = 32, .ws = 8 };
// SliceRange srr = SliceRange{ .h0 = 0, .h1 = 1, .hs = 8, .w0 = 0, .w1 = 64, .ws = 2 };
// SliceRange srr2 = SliceRange{ .h0 = 0, .h1 = 1, .hs = 8, .w0 = 0, .w1 = 64, .ws = 2 };

/**
 * Max-pool 2D. Highly Unoptimized!!
 */
void kernel_main() {
    const uint32_t out_addr = get_arg_val<uint32_t>(1);
    const int32_t out_h = get_arg_val<int32_t>(10);
    const int32_t out_w = get_arg_val<int32_t>(11);
    const uint32_t out_nbytes_c = get_arg_val<uint32_t>(15);
    constexpr bool is_out_dram = get_compile_time_arg_val(1) == 1;

    constexpr uint32_t out_cb_id = tt::CB::c_out0;

    // ROW_MAJOR output
    const InterleavedAddrGen<is_out_dram> s_out = {
        .bank_base_address = out_addr,
        .page_size = out_nbytes_c   // TODO: Ensure this is 32B aligned
    };

    uint32_t out_row_id = 0;
    // for every output pixel
    for (int32_t out_h_i = 0; out_h_i < out_h; ++ out_h_i) {
        for (int32_t out_w_i = 0; out_w_i < out_w; ++ out_w_i) {
            cb_wait_front(out_cb_id, 1);
            uint32_t out_l1_read_addr = get_read_ptr(out_cb_id);
            uint64_t out_noc_addr = get_noc_addr(out_row_id, s_out);
            noc_async_write(out_l1_read_addr, out_noc_addr, out_nbytes_c);
            noc_async_write_barrier();
            cb_pop_front(out_cb_id, 1);
            ++ out_row_id;
        }
    }
} // kernel_main()
