import subprocess
import re
import os
import math
from typing import List

def compileCode(useSlurm: bool = True):
  if useSlurm:
    subprocess.run(["srun", "make", "clean"], stdout=subprocess.PIPE)
    subprocess.run(["srun", "make"], stdout=subprocess.PIPE)
  else:
    subprocess.run(["make", "clean"], stdout=subprocess.PIPE)
    subprocess.run(["make"], stdout=subprocess.PIPE)

def runMPI(
  numNodes: int,
  numTasks: int,
  numTasksPerNode: int,
  numPoints: int,
  useSlurm: bool = True,
) -> str:
  # srun --mpi=pmix --nodes 8 --ntasks 288 --ntasks-per-node 36 build/QuickHullMPI 10000000000

  result = None

  if useSlurm:
    result = subprocess.run([
      "srun",
      "--mpi=pmix",
      "--nodes", str(numNodes),
      "--ntasks", str(numTasks),
      "--ntasks-per-node", str(numTasksPerNode),
      "build/main",
      "npoints=" + str(numPoints),
    ], stdout=subprocess.PIPE)

  else:
    result = subprocess.run([
      "mpirun",
      "-np", str(numTasks),
      "build/main",
      "npoints=" + str(numPoints),
    ], stdout=subprocess.PIPE)

  return str(result.stdout.decode())

def parseOutput(output: str) -> dict:
  # Number of processes: 6
  # Number of points: 1000
  # Estimated memory usage: 0 MB
  # ========================================
  # calculation: 0.000027s
  # communication: 0.001560s
  # final: 0.001629s
  # Size of hull: 30

  try:
    calcTime = re.search(r"calculation: (\d+(\.\d+)*)ms", output).group(1)
    commTime = re.search(r"communication: (\d+(\.\d+)*)ms", output).group(1)
    finalTime = re.search(r"final: (\d+(\.\d+)*)ms", output).group(1)
    hullSize = re.search(r"size: (\d+)", output).group(1)
  except:
    print("!!! Error extracting output !!!!")
    print(output)

    return {
      "calcTime": 0,
      "commTime": 0,
      "finalTime": 0,
      "hullSize": 0,
    }

  return {
    "calcTime": float(calcTime),
    "commTime": float(commTime),
    "finalTime": float(finalTime),
    "hullSize": int(hullSize),
  }

def formatResults(numPoints: int, numTasks: int, numNodes: int, outputs: List[dict], value: str) -> str:
  formatted = f"{numPoints}, {numTasks}, {numNodes}, "

  for output in outputs:
    formatted += f"{output[value]}, "

  return formatted