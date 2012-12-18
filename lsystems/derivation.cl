void __kernel pass1(__read_only image2d_t sym,
                    __global char *derivation,
                    __global int *offset,
                    uint n)
{
    const sampler_t s = CLK_NORMALIZED_COORDS_FALSE |
                        CLK_ADDRESS_NONE |
                        CLK_FILTER_NEAREST;
    size_t gid = get_global_id(0);
    size_t gn = get_global_size(0);
    int2 coord;
    int4 rule_len;

    coord.x = 0;

    for (int i = gid; i < n; i += gn) {
        coord.y = derivation[i];
        rule_len = read_imagei(sym, s, coord);

        offset[i] = rule_len.x;
    }
}

void __kernel pass2a(__global int *offset_in,
                     __global int *offset_out,
                     __local int *offset_tmp)
{
    size_t gid = get_global_id(0);
    size_t lid = get_local_id(0);
    size_t gn = get_global_size(0);
    size_t ln = get_local_size(0);
    size_t gr = get_group_id(0);
    size_t ngr = get_num_groups(0);
    int kin = 1;
    int kout = 0;
    int offset;

    offset_tmp[kout*ln + lid] = (gid > 0) ? offset_in[gid - 1] : 0;

    for (offset = 1; offset < ln; offset *= 2) {
        kin = kout;
        kout = 1 - kout;
        barrier(CLK_LOCAL_MEM_FENCE);

        if (lid >= offset) {
            offset_tmp[kout*ln + lid] = offset_tmp[kin*ln + lid - offset]
                                      + offset_tmp[kin*ln + lid];
        } else {
            offset_tmp[kout*ln + lid] = offset_tmp[kin*ln + lid];
        }
    }

    barrier(CLK_LOCAL_MEM_FENCE);
    offset_out[gid] = offset_tmp[kout*ln + lid];
}

void __kernel pass2b(__global int *offset_in,
                     __global int *offset_out)
{
    size_t gid = get_global_id(0);
    size_t lid = get_local_id(0);
    size_t ln = get_local_size(0);
    size_t gr = get_group_id(0);
    size_t ngr = get_num_groups(0);
    int offset;

    int offset_tmp = offset_in[gid];

    for (offset = 1; offset < ngr; offset++) {
        int prev = offset*ln - 1;
        if (gr >= offset) {
            offset_tmp += offset_in[prev];
        }
    }

    offset_out[gid] = offset_tmp;
}

void __kernel pass3(__read_only image2d_t sym,
                    __global char *derivation_in,
                    __global char *derivation_out,
                    __global int *offset,
                    uint n)
{
    const sampler_t s = CLK_NORMALIZED_COORDS_FALSE |
                        CLK_ADDRESS_NONE |
                        CLK_FILTER_NEAREST;
    size_t gid = get_global_id(0);
    size_t gn = get_global_size(0);
    int2 coord;
    int4 rule_len;
    int4 sym_n;
    size_t off;

    for (int i = gid; i < n; i += gn) {
        coord.x = 0;
        coord.y = derivation_in[i];
        rule_len = read_imagei(sym, s, coord);

        off = offset[i];

        for (int j = 0; j < rule_len.x; j++) {
            ++coord.x;
            sym_n = read_imagei(sym, s, coord);
            derivation_out[off + j] = sym_n.x;
        }
    }
}
