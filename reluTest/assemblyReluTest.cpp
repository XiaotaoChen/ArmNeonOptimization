#include <cstdio>
#include <chrono>
#include <string>

#ifdef __ANDROID__
#include <sys/syscall.h>
#include <unistd.h>
#include <stdint.h>
#endif

// #if defined(__ARM_NEON)
#include <arm_neon.h>
// #endif // __ARM_NEON


extern "C" {
void reluAssembleV2(const float* src, const int aligned_size, float* dst);
}


static int set_sched_affinity(size_t thread_affinity_mask)
{
#ifdef __GLIBC__
    pid_t pid = syscall(SYS_gettid);
#else
#ifdef PI3
    pid_t pid = getpid();
#else
    pid_t pid = gettid();
#endif
#endif
    int syscallret = syscall(__NR_sched_setaffinity, pid, sizeof(thread_affinity_mask), &thread_affinity_mask);
    if (syscallret)
    {
        fprintf(stderr, "syscall error %d\n", syscallret);
        return -1;
    }
    return 0;
} 

bool reluC(const float* src, const int size, float* dst) {
    for (int i=0; i<size; i++) {
        dst[i] = src[i] > 0 ? src[i] : 0.f;
    }
    return true;
}

// #if defined(__ARM_NEON)

bool reluNeon(const float* src, const int size, float* dst) {
    int len = size >> 2;
    int remain = len <<2;
    float32x4_t zero = vdupq_n_f32(0.0f);
    for (int i =0; i<len; i++) {
        float32x4_t data = vld1q_f32(src + i*4);
        float32x4_t result = vmaxq_f32(data, zero);
        vst1q_f32(dst+i*4, result);
    }
    for (int i=remain; i<size; i++) {
        dst[i] = src[i] > 0 ? src[i] : 0.0f;
    }
    return true;
}

bool reluAssemble(const float* src, const int size, float* dst) {
    int len = size >> 2;
    int remain = len <<2;
#ifdef __aarch64__  // armv8
    __asm__ volatile(
    "movi v0.4s, #0           \n"
    "scvtf v0.4s, v0.4s       \n"
    "0:                       \n"
    "ld1 {v1.4s}, [%[src]], #16  \n"
    "fmax v1.4s, v0.4s, v1.4s        \n"
    "subs       %[len], %[len], #1    \n"
    "st1 {v1.4s}, [%[dst]], #16  \n"
    "bgt 0b                   \n"
    :[src]    "+r"(src),
    [dst]    "+r"(dst)
    :[len]    "r"(len)
    :"cc", "memory", "v0", "v1"
    );
#else   // armv7
    __asm__ volatile(
    "vmov.f32 q0, #0.0           \n"
    "0:                       \n"
    // "pld        [%[src], #128]   \n"
    "vld1.32 {d2-d3}, [%[src]]!  \n"
    "vmax.f32 q1, q0, q1        \n"
    "subs       %[len], #1    \n"
    "vst1.32 {d2-d3}, [%[dst]]!  \n"
    "bgt 0b                   \n"
    :[src]    "+r"(src),
     [dst]    "+r"(dst)
    :[len]    "r"(len)
    :"cc", "memory", "q0", "q1"
    );
#endif
    for (int i=remain; i<size; i++) {
        dst[i] = src[i] > 0 ? src[i] : 0.0f;
    }
    return true;
}


bool reluAssembleSplit(const float* src, const int size, float* dst) {
    int len = size >> 2;
    int remain = len <<2;
    reluAssembleV2(src, remain, dst);
    for (int i=remain; i<size; i++) {
        dst[i] = src[i] > 0 ? src[i] : 0.0f;
    }
    return true;
}

// #endif // __ARM_NEON

int checkResult(const float* rights, const float* to_check, int size, const std::string& check_type) {
    int failCount = 0;
    for (int i=0; i<size; i++) {
        if (rights[i] != to_check[i]) {
            failCount++;
        }
    }
    if (failCount>0) {
      printf("[%s check] Error! Results mismatch, fail count = %d\n", check_type.c_str(), failCount);
    } else {
      printf("[%s check] Correct! Results match\n", check_type.c_str());
    }
    return failCount;
}

int main(int argc, char *argv[]) {
    size_t mask = 0;
    for (int i = 0; i < 8; ++i) {
      if (i >= 5) {
        mask |= (1 << i);
      }
    }
    int ret = set_sched_affinity(mask);

    int sizeMb = 8;

    int size = 1024 * 1024 * sizeMb;
    float* src = new float[size];
    float* dstC = new float[size];
    float* dstNeon = new float[size];
    float* dstAsm = new float[size];
    float* dstAsmSplit = new float[size];

    // init
    for (int i=0; i<size; i++) {
        if (i%2==0) {
            src[i] = i;
        }
        else {
            src[i] = -i;
        }
    }

    int loop = 1000;

    float durationC = 0.0f;
    for (int i=0; i<loop; i++) {
        auto start = std::chrono::high_resolution_clock::now();
        reluC(src, size, dstC);   
        auto stop = std::chrono::high_resolution_clock::now();
        durationC += std::chrono::duration<double, std::milli>(stop - start).count();;
    }
    float durationC_avg = durationC / loop;

    float durationNeon = 0.0f;
    for (int i=0; i<loop; i++) {
        auto start = std::chrono::high_resolution_clock::now();
        reluNeon(src, size, dstNeon);
        auto stop = std::chrono::high_resolution_clock::now();
        durationNeon += std::chrono::duration<double, std::milli>(stop - start).count();;
    }
    float durationNeon_avg = durationNeon / loop;

    float durationAsm = 0.0f;
    for (int i=0; i<loop; i++) {
        auto start = std::chrono::high_resolution_clock::now();
        reluAssemble(src, size, dstAsm);
        auto stop = std::chrono::high_resolution_clock::now();
        durationAsm += std::chrono::duration<double, std::milli>(stop - start).count();;
    }
    float durationAsm_avg = durationAsm / loop;

    float durationAsmSplit = 0.0f;
    for (int i=0; i<loop; i++) {
        auto start = std::chrono::high_resolution_clock::now();
        reluAssembleSplit(src, size, dstAsm);
        auto stop = std::chrono::high_resolution_clock::now();
        durationAsmSplit += std::chrono::duration<double, std::milli>(stop - start).count();;
    }
    float durationAsmSplit_avg = durationAsmSplit / loop;


    // check
    // checkResult(dstC, dstNeon, size, "neon");
    // checkResult(dstC, dstAsm, size, "asm");
    checkResult(dstC, dstAsmSplit, size, "asmSplit");
    
    printf("relu time with %d Mb element: \n", sizeMb);
    printf("    plain C        : %f ms\n", durationC_avg);
    printf("    neon intrinsics: %f ms\n", durationNeon_avg);
    printf("    assembly: %f ms\n", durationAsm_avg);
    printf("    assemblySplit: %f ms\n", durationAsmSplit_avg);

    delete[] src;
    delete[] dstC;
    delete[] dstNeon;
    delete[] dstAsm;
    delete[] dstAsmSplit;

}