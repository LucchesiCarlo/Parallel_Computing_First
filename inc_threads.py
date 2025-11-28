import os
import subprocess


def execute_test(source_dir, target, num, sec, out_dir, threads=1, iterations=6):
    if not os.path.exists(out_dir):
        os.makedirs(out_dir)

    for i in range(iterations):
        subprocess.run([source_dir + target, f"{num}", f"{sec}", out_dir + f"/{i}.txt", f"{threads}"])


def main():
    release_dir = "./cmake-build-release/"
    padding_dir = "./cmake-build-padding/"
    out_dir = "results/"

    sequential_name = "sequential"
    parallel_AOS = "parallel_AOS"
    parallel_AOS_simd = "parallel_AOS_simd"
    parallel_AOS_simd_manual = "parallel_AOS_simd_manual"
    parallel_SOA = "parallel_SOA"
    parallel_SOA_simd = "parallel_SOA_simd"
    parallel_SOA_simd_manual = "parallel_SOA_simd_manual"

    parallel_list = [parallel_AOS, parallel_AOS_simd, parallel_AOS_simd_manual, parallel_SOA, parallel_SOA_simd,
                     parallel_SOA_simd_manual]

    base_num = 5000
    base_seconds = 10
    # Sequential Version Base Case
    execute_test(release_dir, sequential_name, base_num, base_seconds, out_dir + sequential_name)
    # Parallel Versions Base Case
    for t in range(1, 11):
        for name in parallel_list:
            execute_test(release_dir, name, base_num, base_seconds, out_dir + name + f"_{t}", t)


main()
