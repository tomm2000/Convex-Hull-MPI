import subprocess
import re
import os
from time import sleep

startfrom = 0
numpoints_values = [500_000_000]
numtasks_values = [36]
n_nodes = 4
num_tries = 5

result = subprocess.run(["srun", "-N", "1", "g++", "randompoints.cpp", "-o", "randompoints.out", "-O3"], stdout=subprocess.PIPE)
print(f"Compiled randompoints.out")

result = subprocess.run(["srun", "-N", "1", "mpicxx", "QuickHullMPI.cxx", "-o", "QuickHullMPI.out", "-O3"], stdout=subprocess.PIPE)
print(f"Compiled QuickHullMPI.out")


for num_tasks in numtasks_values:
  with open(f"benchmark_{num_tasks}.csv", "w") as f:
    f.write("numpoints,numtasks,time_read,time_algo\n")

    for numpoints in numpoints_values:
      if numpoints < startfrom:
        continue

      total_elapsed_time = 0
      total_read_time = 0

      for i in range(num_tries):
        result = subprocess.run(["srun", "-N", "1", "randompoints.out", str(numpoints)], stdout=subprocess.PIPE)
        print()
        print(result.stdout)

        print("-------------------------------------")
        print(f"Running QuickHullMPI with {numpoints} points and {num_tasks}*{n_nodes} tasks")

        result = subprocess.run(["srun", "--mpi=pmix", "--nodes", str(n_nodes), "--ntasks", str(num_tasks * n_nodes), "--ntasks-per-node", str(num_tasks), "QuickHullMPI.out"], stdout=subprocess.PIPE)

        # extract the elapsed time
        elapsed_time = re.search(r"Elapsed time: (\d+\.\d+)ms", result.stdout.decode())
        read_time = re.search(r"Init time: (\d+\.\d+)ms", result.stdout.decode())

        try:
          total_elapsed_time += float(elapsed_time.group(1))
          total_read_time += float(read_time.group(1))
        except:
          print("Error parsing output")
          print(result.stdout)
          continue

        print(f"Init time: {read_time.group(1)}ms, Elapsed time: {elapsed_time.group(1)}ms")

      avg_elapsed_time = total_elapsed_time / num_tries
      avg_read_time = total_read_time / num_tries

      print("=====================================")
      print(f"Average read time: {avg_read_time}ms, Average elapsed time: {avg_elapsed_time}ms")

      f.write(f"{numpoints},{num_tasks},{avg_read_time},{avg_elapsed_time}\n")

print()
print("=====================================")
jobid = os.popen("squeue -u $USER | tail -1| awk '{print $1}'").read().strip()
print(f"cancelling job {jobid}")
os.system(f"scancel {jobid}")