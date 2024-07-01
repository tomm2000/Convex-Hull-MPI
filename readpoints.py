# create a list to store the integers
integers = []

# read the points.bin file and convert it to integers
with open('points.bin', 'rb') as f:
  # read the bytes
  bytes = f.read()

  # points file contains the points as binary data
  # each point is represented by two integers
  # each integer is 4 bytes long and little-endian
  for i in range(0, len(bytes), 4):
    # convert 4 bytes to an integer
    x = int.from_bytes(bytes[i:i+4], 'little', signed=True)
    integers.append(x)


# write the integers to a file
with open('points.txt', 'w') as f:
  # write 2 integers per line
  for i in range(1, len(integers), 2):
    f.write(f'{integers[i]} {integers[i+1]}\n')