import os
import subprocess
import sys


def execute_test(source_dir, target, num, sec, out_dir, threads=1, iterations=6):
    if not os.path.exists(out_dir):
        os.makedirs(out_dir)

    for i in range(iterations):
        subprocess.run([source_dir + target, f"{num}", f"{sec}", out_dir + f"/{i}.txt", f"{threads}"])


def main():
    release_dir = "./cmake-build-release/"
    padding_dir = "./cmake-build-padding/"
    out_dir = "results/"
    argv = sys.argv
    padding = ("padding" in argv)
    schedule = "dynamic"

    sequential_name = "sequential"
    parallel_AOS = "parallel_AOS"
    parallel_AOS_simd = "parallel_AOS_simd"
    parallel_AOS_simd_manual = "parallel_AOS_simd_manual"
    parallel_SOA = "parallel_SOA"
    parallel_SOA_simd = "parallel_SOA_simd"
    parallel_SOA_simd_manual = "parallel_SOA_simd_manual"

    if padding:
        parallel_list = [parallel_AOS, parallel_AOS_simd, parallel_AOS_simd_manual]
    else:
        parallel_list = [parallel_AOS, parallel_AOS_simd, parallel_AOS_simd_manual, parallel_SOA, parallel_SOA_simd,
                         parallel_SOA_simd_manual]

    base_num = 5000
    base_seconds = 10
    base_threads = 8

    # Parallel Versions Increasing Chunck size
    chunks = [1, 4, 10, 25, 50, 100, 200, 300, 400, 500, 625]
    for c in chunks:
        for name in parallel_list:
            os.environ["OMP_SCHEDULE"] = schedule + f",{c}"
            if not padding:
                execute_test(release_dir, name, base_num, base_seconds, out_dir + name + f"_dynamic{c}", base_threads)
            else:
                execute_test(padding_dir, name, base_num, base_seconds, out_dir + name + f"_padding_dynamic{c}",
                             base_threads)


main()
