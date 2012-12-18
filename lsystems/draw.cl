
void __kernel draw1(__constant float *matrices_in,
                    __global char *derivation,
                    __global float *matrices_out,
                    __global int *offset,
                    uint n)
{
    size_t gid = get_global_id(0);
    size_t gn = get_global_size(0);
    size_t gid_ = gn - 1 - gid;
    char s;

    s = derivation[gid];

    offset[gid_] = (int) matrices_in[s * 10];

    for (int i = 0; i < 9; i++) {
        matrices_out[gid_ * 9 + i] = matrices_in[s * 10 + 1 + i];
    }
}

void __kernel draw2a(__global float *matrices_in,
                     __global float *matrices_out,
                     __local float *matrices_tmp)
{
    size_t gid = get_global_id(0);
    size_t lid = get_local_id(0);
    size_t ln = get_local_size(0);
    int kin = 1;
    int kout = 0;
    int offset;
    float accum;


    for (int i = 0; i < 9; i++) {
        matrices_tmp[kout*ln*9 + lid*9 + i] = matrices_in[gid*9 + i];
    }

    for (offset = 1; offset < ln; offset *= 2) {
        kin = kout;
        kout = 1 - kout;
        barrier(CLK_LOCAL_MEM_FENCE);

        if (lid >= offset) {
            for (int i = 0; i < 3; i++) {
                for (int j = 0; j < 3; j++) {
                    accum = 0.0f;

                    for (int k = 0; k < 3; k++) {
                        accum += matrices_tmp[kin*ln*9 + lid*9 + i*3 + k]
                                * matrices_tmp[kin*ln*9 + (lid-offset)*9 + k*3 + j];
                    }

                    matrices_tmp[kout*ln*9 + lid*9 + i*3 + j] = accum;
                }
            }
        } else {
            for (int i = 0; i < 9; i++) {
                matrices_tmp[kout*ln*9 + lid*9 + i] =
                    matrices_tmp[kin*ln*9 + lid*9 + i];
            }
        }
    }

    barrier(CLK_LOCAL_MEM_FENCE);
    for (int i = 0; i < 9; i++) {
        matrices_out[gid*9 + i] = matrices_tmp[kout*ln*9 + lid*9 + i];
    }
}

void __kernel draw2b(__global float *matrices_in,
                     __global float *matrices_out)
{
    size_t gid = get_global_id(0);
    size_t lid = get_local_id(0);
    size_t ln = get_local_size(0);
    size_t gr = get_group_id(0);
    size_t ngr = get_num_groups(0);
    int offset;
    float matrix_tmp[9];
    float accum[3];

    for (int i = 0; i < 9; i++) {
        matrix_tmp[i] = matrices_in[gid*9 + i];
    }

    for (offset = 1; offset < ngr; offset++) {
        int prev = gr*9*ln - (offset-1)*9*ln - 9;
        if (gr >= offset) {
            for (int i = 0; i < 3; i++) {

                accum[0] = accum[1] = accum[2] = 0.0f;

                for (int j = 0; j < 3; j++) {
                    for (int k = 0; k < 3; k++) {
                        accum[j] += matrix_tmp[i*3 + k]
                                    * matrices_in[prev + k*3 + j];
                    }
                }

                for (int j = 0; j < 3; j++) {
                    matrix_tmp[i*3 + j] = accum[j];
                }
            }
        } else {
            for (int i = 0; i < 9; i++) {
                matrix_tmp[i] = matrix_tmp[i];
            }
        }
    }

    for (int i = 0; i < 9; i++) {
        matrices_out[gid*9 + i] = matrix_tmp[i];
    }
}

void __kernel draw2c(__global int *offset,
                     __global int *address,
                     __global float *matrices_in,
                     __global float *matrices_out)
{
    size_t gid = get_global_id(0);

    if (offset[gid] == 1) {
        int d = address[gid] * 9;

        for (int i = 0; i < 9; i++) {
            matrices_out[d + i] = matrices_in[gid*9 + i];
        }
    }
}

void __kernel draw3(__global float *matrices,
                    __global float *points)
{
    size_t gid = get_global_id(0);
    size_t gid_ = gid*2 - 1;
    size_t gn = get_global_size(0);
    float init[3] = {0.0f, 0.0f, 1.0f};
    float accum;

    if (gid == 0) {
        for (int j = 0; j < 3; j++) {
            points[j] = init[j];
        }
    } else {
        for (int i = 0; i < 3; i++) {
            accum = 0.0f;

            for (int j = 0; j < 3; j++) {
                accum += init[j] * matrices[(gid-1)*9 + j*3 + i];
            }

            points[gid_*3 + i] = accum;
            if (gid + 1 != gn) {
                points[(gid_+1)*3 + i] = accum;
            }
        }
    }
}
