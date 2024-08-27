import subprocess
import re
import os
import math
import utils

numpoints = 1_000_000_000
max_nodes = 8
num_tries = 5
numtasks_values = [4, 8, 12, 18]
tasks_per_node = 36

step = 9
next = numtasks_values[-1] + step
while next <= max_nodes * 36:
  numtasks_values.append(next)
  next += step

print(f"numtasks_values: {numtasks_values}")

utils.compileCode()

with open(f"strong_scaling_{numpoints}.csv", "w") as f:
  # test_1, test_2, test_3, test_4, test_5, ...
  tests_string = [f"time_{i+1}" for i in range(num_tries)]
  tests_string = ", ".join(tests_string)

  f.write(f"numpoints, numtasks, hull_size, {tests_string}, avg_time\n")

  calc_times = []
  comm_times = []
  final_times = []
  hull_sizes = []

  for num_tasks in numtasks_values:
    total_elapsed_time = 0
    total_hull_size = 0
    num_nodes = math.ceil(num_tasks / tasks_per_node)

    tests = []

    for i in range(num_tries):
      print("-------------------------------------")
      print(f"Running test {i+1}/{num_tries} with {num_tasks} tasks on {num_nodes} nodes, {numpoints} points")

      result = utils.runMPI(num_nodes, num_tasks, tasks_per_node, numpoints)
      output = utils.parseOutput(result)
      tests.append(output)

      print(result)
      print(output)
      print("-------------------------------------")
      
    calc_times.append(utils.formatResults(numpoints, num_tasks, num_nodes, tests, "calcTime"))
    comm_times.append(utils.formatResults(numpoints, num_tasks, num_nodes, tests, "commTime"))
    final_times.append(utils.formatResults(numpoints, num_tasks, num_nodes, tests, "finalTime"))
    hull_sizes.append(utils.formatResults(numpoints, num_tasks, num_nodes, tests, "hullSize"))

  f.write("calcTime\n")
  for line in calc_times:
    f.write(line + "\n")
  
  f.write("commTime\n")
  for line in comm_times:
    f.write(line + "\n")

  f.write("finalTime\n")
  for line in final_times:
    f.write(line + "\n")

  f.write("hullSize\n")
  for line in hull_sizes:
    f.write(line + "\n")

  f.flush()

print()
print("=====================================")
jobid = os.popen("squeue -u $USER | tail -1| awk '{print $1}'").read().strip()
print(f"cancelling job {jobid}")
os.system(f"scancel {jobid}")