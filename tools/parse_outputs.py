# MPI tasks: 24 | Use hybrid: true | Seed: 1 | Total points: 2000000000 
# --------
# calculation: 586.643987ms
# communication: 144.265583ms
# final: 833.312035ms
# points: 4945.370915ms
# size: 4159
# ====================
# MPI tasks: 24 | Use hybrid: true | Seed: 2 | Total points: 2000000000 
# --------
# calculation: 613.541011ms
# communication: 133.966498ms
# final: 831.174883ms
# points: 4952.775605ms
# size: 4171
# ====================
# MPI tasks: 24 | Use hybrid: true | Seed: 3 | Total points: 2000000000 
# --------
# calculation: 583.232314ms
# communication: 270.155503ms
# final: 869.820399ms
# points: 4944.343065ms
# size: 4148
# ====================
# MPI tasks: 24 | Use hybrid: true | Seed: 4 | Total points: 2000000000 
# --------
# calculation: 708.450390ms
# communication: 84.482718ms
# final: 817.314122ms
# points: 4937.695451ms
# size: 4108
# ====================
# MPI tasks: 24 | Use hybrid: true | Seed: 5 | Total points: 2000000000 
# --------
# calculation: 553.630430ms
# communication: 46.239609ms
# final: 848.821218ms
# points: 4980.687082ms
# size: 4176


import os
import re

# open the output folder
output_folder = 'output'
output_files = os.listdir(output_folder)

calc_times_results = []
comm_times_results = []
final_times_results = []
points_times_results = []
sizes_results = []

# parse the output files
for output_file in output_files:
  print(f'Parsing {output_file}...')
  if output_file.endswith('.txt'):
    with open(os.path.join(output_folder, output_file), 'r') as file:
      text = file.read()

      # find the number of MPI tasks and number of points
      mpi_tasks = re.search(r'MPI tasks: (\d+)', text).group(1)
      number_of_points = re.search(r'Total points: (\d+)', text).group(1)

      # find the calculation times
      calculation_times = re.findall(r'calculation: (\d+\.\d+)ms', text)
      out = f'{mpi_tasks},{number_of_points},{",".join(calculation_times)}\n'
      calc_times_results.append(out)

      # find the communication times
      communication_times = re.findall(r'communication: (\d+\.\d+)ms', text)
      out = f'{mpi_tasks},{number_of_points},{",".join(communication_times)}\n'
      comm_times_results.append(out)

      # find the final times
      final_times = re.findall(r'final: (\d+\.\d+)ms', text)
      out = f'{mpi_tasks},{number_of_points},{",".join(final_times)}\n'
      final_times_results.append(out)

      # find the points times
      points_times = re.findall(r'points: (\d+\.\d+)ms', text)
      out = f'{mpi_tasks},{number_of_points},{",".join(points_times)}\n'
      points_times_results.append(out)

      # find the sizes
      sizes = re.findall(r'size: (\d+)', text)
      out = f'{mpi_tasks},{number_of_points},{",".join(sizes)}\n'
      sizes_results.append(out)

# sort by MPI tasks
calc_times_results.sort(key=lambda x: int(x.split(',')[0]) )
comm_times_results.sort(key=lambda x: int(x.split(',')[0]) )
final_times_results.sort(key=lambda x: int(x.split(',')[0]) )
points_times_results.sort(key=lambda x: int(x.split(',')[0]) )
sizes_results.sort(key=lambda x: int(x.split(',')[0]) )

# write the results to files
with open('results/output.csv', 'w') as file:
  file.write('mpi_tasks,points,calculation_times\n')
  for result in calc_times_results:
    file.write(result)

  file.write('mpi_tasks,points,communication_times\n')
  for result in comm_times_results:
    file.write(result)

  file.write('mpi_tasks,points,final_times\n')
  for result in final_times_results:
    file.write(result)

  file.write('mpi_tasks,points,points_times\n')
  for result in points_times_results:
    file.write(result)

  file.write('mpi_tasks,points,sizes\n')
  for result in sizes_results:
    file.write(result)