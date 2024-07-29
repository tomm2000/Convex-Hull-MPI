import subprocess
import re
import os

base_numpoints = 100_000_000
numtasks_values = [1, 2, 4, 8, 16, 24, 32, 36]
num_tries = 5

# clean the build directory
result = subprocess.run(["make", "clean"], stdout=subprocess.PIPE)

# run make file instead
result = subprocess.run(["make"], stdout=subprocess.PIPE)
print(f"Compiled QuickHullMPI")


with open(f"benchmark.csv", "w") as f:
  f.write("numpoints,numtasks,hull_size,time_algo\n")
  for num_tasks in numtasks_values:
    numpoints = base_numpoints * num_tasks
    
    total_elapsed_time = 0
    total_hull_size = 0

    for i in range(num_tries):
      print("-------------------------------------")
      print(f"Running test {i+1}/{num_tries} with {num_tasks} tasks on {numpoints} points")

      result = subprocess.run(["srun", "--mpi=pmix", "--nodes", str(1), "--ntasks", str(num_tasks), "--ntasks-per-node", str(num_tasks), "build/QuickHullMPI", str(numpoints)], stdout=subprocess.PIPE)
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

      f.write(f"{numpoints},{num_tasks},{hull_size},{elapsed_time}\n")

    avg_elapsed_time = total_elapsed_time / num_tries
    avg_hull_size = total_hull_size / num_tries

    print("=====================================")
    print(f"Average time: {avg_elapsed_time}ms")

    f.write(f"{numpoints},{num_tasks},{avg_hull_size},{avg_elapsed_time}\n")

print()
print("=====================================")
jobid = os.popen("squeue -u $USER | tail -1| awk '{print $1}'").read().strip()
print(f"cancelling job {jobid}")
os.system(f"scancel {jobid}")