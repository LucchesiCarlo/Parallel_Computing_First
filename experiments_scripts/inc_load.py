import os
import subprocess
import sys

def execute_test(source_dir, target, num, sec, out_dir, threads=1, iterations=6):
    if not os.path.exists(out_dir):
        os.makedirs(out_dir)

    for i in range(iterations):
        subprocess.run([source_dir + target, f"{num}", f"{sec}", out_dir + f"/{i}.txt", f"{threads}"])


def main():
    release_dir = "../cmake-build-release/"
    padding_dir = "../cmake-build-padding/"
    out_dir = "results/"
    padding = ("padding" in sys.argv)
    
    sequential_name = "sequential"
    parallel_AOS = "parallel_AOS"
    parallel_AOS_simd = "parallel_AOS_simd"
    parallel_AOS_simd_manual = "parallel_AOS_simd_manual"
    parallel_SOA = "parallel_SOA"
    parallel_SOA_simd = "parallel_SOA_simd"
    parallel_SOA_simd_manual = "parallel_SOA_simd_manual"

    if not padding:
        source_list = [sequential_name, parallel_AOS, parallel_AOS_simd, parallel_AOS_simd_manual, parallel_SOA,
                   parallel_SOA_simd, parallel_SOA_simd_manual]
    else:
        source_list = [sequential_name, parallel_AOS, parallel_AOS_simd, parallel_AOS_simd_manual]

    base_threads = 8
    base_seconds = 10

    # Generating test for increasing load
    for l in range(1000, 15001, 1000):
        for name in source_list:
            if not padding:
                execute_test(release_dir, name, l, base_seconds, out_dir + name + f"_load{l}", base_threads)
            else:
                execute_test(padding_dir, name, l, base_seconds, out_dir + name + f"_padding_load{l}", base_threads)


main()
