import subprocess
import re
import os
from time import sleep

numpoints = 500_000_000
numtasks_values = [1, 2, 4, 8, 16, 24, 32, 36]
task_per_node = 36
num_tries = 5

# clean the build directory
result = subprocess.run(["make", "clean"], stdout=subprocess.PIPE)

# run make file instead
result = subprocess.run(["make"], stdout=subprocess.PIPE)
print(f"Compiled QuickHullMPI")


for num_tasks in numtasks_values:
  with open(f"benchmark_{num_tasks}.csv", "w") as f:
    f.write("numpoints,numtasks,time_read,time_algo\n")

    total_elapsed_time = 0

    for i in range(num_tries):
      print("-------------------------------------")
      print(f"Running program with {numpoints} points and {num_tasks} tasks")

      result = subprocess.run(["srun", "--mpi=pmix", "--nodes", str(1), "--ntasks", str(num_tasks), "--ntasks-per-node", str(num_tasks), "build/QuickHullMPI", str(numpoints)], stdout=subprocess.PIPE)

      # extract the elapsed time
      elapsed_time = re.search(r"Elapsed time: (\d+\.\d+)ms", result.stdout.decode())

      try:
        total_elapsed_time += float(elapsed_time.group(1))
      except:
        print("Error parsing output")
        print(result.stdout)
        continue

      print(f"Elapsed time: {elapsed_time.group(1)}ms")

      f.write(f"{numpoints},{num_tasks},{elapsed_time.group(1)}\n")

    avg_elapsed_time = total_elapsed_time / num_tries

    print("=====================================")
    print(f"Average time: {avg_elapsed_time}ms")

    f.write(f"{numpoints},{num_tasks},{avg_elapsed_time}\n")

print()
print("=====================================")
jobid = os.popen("squeue -u $USER | tail -1| awk '{print $1}'").read().strip()
print(f"cancelling job {jobid}")
os.system(f"scancel {jobid}")