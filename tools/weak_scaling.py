import subprocess
import re
import os
import math

base_numpoints = 100_000_000
max_nodes = 8
num_tries = 5
numtasks_values = [1, 2, 4, 8, 12, 18, 27, 36]
tasks_per_node = 36

step = 9
next = numtasks_values[-1] + step
while next <= max_nodes * 36:
  numtasks_values.append(next)
  next += step

print(f"numtasks_values: {numtasks_values}")

# clean the build directory
result = subprocess.run(["make", "clean"], stdout=subprocess.PIPE)

# run make file instead
result = subprocess.run(["make"], stdout=subprocess.PIPE)
print(f"Compiled QuickHullMPI")


with open(f"weak_scaling_{base_numpoints}.csv", "w") as f:
  # test_1, test_2, test_3, test_4, test_5, ...
  tests_string = [f"time_{i+1}" for i in range(num_tries)]
  tests_string = ", ".join(tests_string)

  f.write(f"numpoints, numtasks, numnodes, hull_size, {tests_string}, avg_time\n")

  for num_tasks in numtasks_values:
    numpoints = base_numpoints * num_tasks

    total_elapsed_time = 0
    total_hull_size = 0

    test_times = []
    nnodes = math.ceil(num_tasks / tasks_per_node)
    # nnodes = min(num_tasks, max_nodes)

    for i in range(num_tries):
      print("-------------------------------------")
      print(f"Running test {i+1}/{num_tries} with {num_tasks} tasks on {nnodes} nodes, {numpoints} points")

      result = subprocess.run(["srun", "--mpi=pmix", "--nodes", str(nnodes), "--ntasks", str(num_tasks), "build/QuickHullMPI", str(numpoints)], stdout=subprocess.PIPE)
      output = str(result.stdout.decode())

      print("=====================================")
      print(output)
      print("=====================================")

      # extract the elapsed time
      try:
        elapsed_time = re.search(r"Elapsed time: (\d+(\.\d+)*)ms", result.stdout.decode()).group(1)
        hull_size = re.search(r"Hull size: (\d+)", result.stdout.decode()).group(1)
      except:
        print("Error extracting output : ---------------------------")
        print(output)
        print("-----------------------------------------------------")
        continue

      try:
        total_elapsed_time += float(elapsed_time)
        total_hull_size += int(hull_size)
      except:
        print("Error converting output : ---------------------------")
        print(output)
        print("-----------------------------------------------------")
        continue

      print(f"Elapsed time: {elapsed_time}ms")
      
      test_times.append(elapsed_time)

    avg_elapsed_time = total_elapsed_time / num_tries
    avg_hull_size = total_hull_size / num_tries

    print("=====================================")
    print(f"Average time: {avg_elapsed_time}ms")

    f.write(f"{numpoints}, {num_tasks}, {nnodes}, {avg_hull_size}, {', '.join(test_times)}, {round(avg_elapsed_time, 2)}\n")
    f.flush()

print()
print("=====================================")
jobid = os.popen("squeue -u $USER | tail -1| awk '{print $1}'").read().strip()
print(f"cancelling job {jobid}")
os.system(f"scancel {jobid}")