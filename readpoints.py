# read the points.bin file and convert it to integers
with open('results/points.txt', 'w') as output_file:
  with open('results/points.bin', 'rb') as points_file:
    # read the bytes
    bytes = points_file.read()

    # first 4 bytes are the size of the number of points
    size_num_points = int.from_bytes(bytes[:4], 'little')
    print("size_num_points: ", size_num_points)

    # second 4 bytes are the size of each point
    size_point = int.from_bytes(bytes[4:8], 'little')
    print("size_point: ", size_point)

    # size_num_points bytes are the number of points
    num_points = int.from_bytes(bytes[8:8 + size_num_points], 'little')
    print("num_points: ", num_points)

    for i in range(8 + size_num_points, len(bytes), size_point):
      x = int.from_bytes(bytes[i:i + size_point // 2], 'little', signed=True)
      y = int.from_bytes(bytes[i + size_point // 2:i + size_point], 'little', signed=True)

      output_file.write(f"{x} {y}\n")