// make a simple user mapping.
#include "rpi.h"
#include "pt-vm.h"

enum { N = 64 };

void matmul(unsigned *a, unsigned *b, unsigned *c) {
    for(int i = 0; i < N; i++)
        for(int j = 0; j < N; j++) {
            unsigned sum = 0;
            for(int k = 0; k < N; k++)
                sum += a[i*N+k] * b[k*N+j];
            c[i*N+j] = sum;
        }
}

void notmain(void) { 
    assert(!mmu_is_enabled());

    enum { OneMB = 1024*1024};
    // map the heap: for lab cksums must be at 0x100000.
    kmalloc_init_set_start((void*)OneMB, OneMB);

    vm_pt_t *pt = vm_pt_alloc(PT_LEVEL1_N);


    unsigned *matrix = (unsigned *)(3 * OneMB),
             *matrix2 = matrix + N * N * sizeof(*matrix),
             *matrixOut1 = matrix2 + N * N * sizeof(*matrix),
             *matrixOut2 = matrixOut1 + N * N * sizeof(*matrix);

    for(int i = 0; i < N*N; i++) {
        matrix[i] = i + 1;
        matrix2[i] = i + N * N + 1;
    }

    vm_mmu_init(dom_all_access);

    enum { 
        dom_kern = 1, // domain id for kernel
        dom_user = 2  // domain id for user
    };          

    uint32_t no_user = perm_rw_priv;

    // device memory: kernel domain, no user access, 
    // memory is strongly ordered, not shared.
    pin_t dev  = pin_mk_global(dom_kern, no_user, MEM_device);
    // kernel memory: same, but is only uncached.
    pin_t kern = pin_mk_global(dom_kern, no_user, MEM_uncached);
    pin_t kern_cached = pin_mk_global(dom_kern, no_user, MEM_cached);

    vm_map_sec(pt, 0x20000000, 0x20000000, dev);
    vm_map_sec(pt, 0x20100000, 0x20100000, dev);
    vm_map_sec(pt, 0x20200000, 0x20200000, dev); 
    // map first two MB for the kernel (it would be better to not
    // have 0 (null) mapped of course, but our code starts at 0x8000.
    vm_map_sec(pt, 0, 0, kern);                 
    vm_map_sec(pt, OneMB, OneMB, kern);

    // map kernel stack (or nothing will work)
    vm_map_sec(pt, STACK_ADDR-OneMB, STACK_ADDR-OneMB, kern);
    
    vm_map_sec(pt, (unsigned) matrix, (unsigned) matrix, kern);

    trace("about to enable\n");

    // lockdown_print_entries("about to turn on first time");
    vm_mmu_switch(pt,0x140e,1);

    vm_mmu_enable();

    asm volatile("" ::: "memory");
    unsigned start = timer_get_usec();
    asm volatile("" ::: "memory");
    matmul(matrix, matrix2, matrixOut1);
    asm volatile("" ::: "memory");
    unsigned end = timer_get_usec();
    asm volatile("" ::: "memory");
    
    // Turning on caching
    cp15_ctrl_reg1_t c = cp15_ctrl_reg1_rd();
    c.C_unified_enable = 1;
    c.W_write_buf = 1;
    c.I_icache_enable = 1;
    c.L2_enabled = 1;
    c.Z_branch_pred = 1;
    cp15_ctrl_reg1_wr(c);
    prefetch_flush();

    // caches_enable();
    // prefetch_flush();
    // trace("caches enabled: %d\n", cp15_ctrl_reg1_rd().Z_branch_pred);

    staff_mmu_sync_pte_mods();
    vm_map_sec(pt, 0, 0, kern_cached); 
    vm_map_sec(pt, (unsigned) matrix, (unsigned) matrix, kern_cached);

    matmul(matrix, matrix2, matrixOut1);
    matmul(matrix, matrix2, matrixOut1);
    matmul(matrix, matrix2, matrixOut1);
    
    asm volatile("" ::: "memory");
    unsigned start2 = timer_get_usec();
    asm volatile("" ::: "memory");
    matmul(matrix, matrix2, matrixOut1);
    asm volatile("" ::: "memory");
    unsigned end2 = timer_get_usec();
    asm volatile("" ::: "memory");

    trace("uncached: %d, cached: %d\n", end-start, end2-start2);
    if (end-start > end2-start2)
        trace("SUCCESS! caching sped up matmuls!\n");
    else
        trace("FAILURE! caching did not speed up matmuls!\n");
}