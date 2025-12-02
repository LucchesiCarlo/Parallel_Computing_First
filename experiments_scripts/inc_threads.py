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

    sequential_AOS = "sequentialAOS"
    parallelAOS = "parallelAOS"
    parallelSIMD_AOS = "parallelSIMD_AOS"
    parallelAVX_AOS = "parallelAVX_AOS"
    parallelSOA = "parallelSOA"
    parallelSIMD_SOA = "parallelSIMD_SOA"
    parallelAVX_SOA = "parallelAVX_SOA"

    if padding:
        parallel_list = [parallelAOS, parallelSIMD_AOS, parallelAVX_AOS]
    else:
        parallel_list = [parallelAOS, parallelSIMD_AOS, parallelAVX_AOS, parallelSOA, parallelSIMD_SOA,
                         parallelAVX_SOA]

    base_num = 5000
    base_seconds = 10
    # Sequential Version Base Case
    if not padding:
        execute_test(release_dir, sequential_AOS, base_num, base_seconds, out_dir + sequential_AOS)
    else:
        execute_test(padding_dir, sequential_AOS, base_num, base_seconds, out_dir + sequential_AOS + "_padding")

    # Parallel Versions Base Case
    for t in range(1, 11):
        for name in parallel_list:
            if not padding:
                execute_test(release_dir, name, base_num, base_seconds, out_dir + name + f"_{t}", t)
            else:
                execute_test(padding_dir, name, base_num, base_seconds, out_dir + name + f"_padding_{t}", t)


main()
