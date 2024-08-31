# read the points.bin file
from PIL import Image, ImageDraw, ImageFont
import math
import time

start = time.time()
PADDING = 100
IMG_WIDTH = 1024
IMG_HEIGHT = 1024

def drawCircle(draw, x, y, min_x, max_x, min_y, max_y, PADDING, color, size):
  if (size == 0):
    drawPoint(draw, x, y, min_x, max_x, min_y, max_y, PADDING, color)
    return

  x = int((x - min_x) / (max_x - min_x) * (IMG_WIDTH - PADDING * 2) + PADDING)
  y = int((y - min_y) / (max_y - min_y) * (IMG_HEIGHT - PADDING * 2) + PADDING)

  draw.ellipse((x - size, y - size, x + size, y + size), fill=color)

def drawPoint(draw, x, y, min_x, max_x, min_y, max_y, PADDING, color):
  x = int((x - min_x) / (max_x - min_x) * (IMG_WIDTH - PADDING * 2) + PADDING)
  y = int((y - min_y) / (max_y - min_y) * (IMG_HEIGHT - PADDING * 2) + PADDING)
  
  draw.point((x, y), fill=color)

def drawLine(draw, start_x, start_y, end_x, end_y, min_x, max_x, min_y, max_y, PADDING, color, width):
  start_x = int((start_x - min_x) / (max_x - min_x) * (IMG_WIDTH - PADDING * 2) + PADDING)
  start_y = int((start_y - min_y) / (max_y - min_y) * (IMG_HEIGHT - PADDING * 2) + PADDING)
  end_x = int((end_x - min_x) / (max_x - min_x) * (IMG_WIDTH - PADDING * 2) + PADDING)
  end_y = int((end_y - min_y) / (max_y - min_y) * (IMG_HEIGHT - PADDING * 2) + PADDING)

  draw.line((start_x, start_y, end_x, end_y), fill=color, width=width)

with open('results/points.bin', 'rb') as points_file:
  with open('results/hull.txt', 'r') as hull_file:
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

    # create a 1000x1000 image
    img = Image.new('RGB', (IMG_WIDTH, IMG_HEIGHT))
    draw = ImageDraw.Draw(img)

    # white background
    draw.rectangle((0, 0, IMG_WIDTH, IMG_HEIGHT), fill='white')

    max_x, max_y, min_x, min_y = 0, 0, 0, 0

    points_read = 0
    for i in range(8 + size_num_points, len(bytes), size_point):
      x = int.from_bytes(bytes[i:i + size_point // 2], 'little', signed=True)
      y = int.from_bytes(bytes[i + size_point // 2:i + size_point], 'little', signed=True)

      # update the min and max values
      max_x = max(max_x, x)
      max_y = max(max_y, y)
      min_x = min(min_x, x)
      min_y = min(min_y, y)

      points_read += 1
      if points_read >= 10_000_000:
        print("Too many points, breaking")
        break

    PADDING = 100

    # draw the points
    points_read = 0
    for i in range(8 + size_num_points, len(bytes), size_point):
      x = int.from_bytes(bytes[i:i + size_point // 2], 'little', signed=True)
      y = int.from_bytes(bytes[i + size_point // 2:i + size_point], 'little', signed=True)

      # scale the points to fit the image, add padding
      drawCircle(draw, x, y, min_x, max_x, min_y, max_y, PADDING, 'black', 1)

      points_read += 1
      if points_read >= 1_000_000:
        print("Too many points, breaking")
        break

    # ========================== DRAW THE HULL ==========================
    sum_x = 0
    sum_y = 0

    hull_size = len(hull_file.readlines())

    if hull_size > 0:
      hull_file.seek(0)
      hull = []

      # draw the hull
      for line in hull_file:
        # each line has two points
        coords = line.split(" ")

        if len(coords) < 2:
          continue

        x = int(coords[0])
        y = int(coords[1])

        sum_x += x
        sum_y += y

        hull.append((x, y))
        
        drawCircle(draw, x, y, min_x, max_x, min_y, max_y, PADDING, 'red', 4)

        sum_x += x
        sum_y += y

      center_x = sum_x // hull_size
      center_y = sum_y // hull_size

      # sort the hull points based on the angle from the center
      hull.sort(key=lambda point: math.atan2(point[1] - center_y, point[0] - center_x))

      # draw the hull lines
      for i in range(hull_size):
        start_x, start_y = hull[i]
        end_x, end_y = hull[(i + 1) % hull_size]

        drawLine(draw, start_x, start_y, end_x, end_y, min_x, max_x, min_y, max_y, PADDING, 'red', 2)


      hull_file.seek(0)

    # at the bottom of the image, draw the number of points and hull points
    fontsize = 30
    font = ImageFont.truetype("arial.ttf", fontsize)
    text = f"Points: {num_points}, Hull size: {len(hull_file.readlines())}"
    draw.text((10, IMG_HEIGHT - 50), text, font=font, fill='black')

      # save the image
    img.save('results/points.png')

print("Time taken: ", time.time() - start)


