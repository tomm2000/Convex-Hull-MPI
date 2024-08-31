# Format for the input file (points.bin)
- 4 bytes: #bytes for the number of points (n)
- 4 bytes: #bytes for each point (d)
- n bytes (size_t): number of points
- n*d bytes (long+long): points

# Some "small" examples of convex hulls with different point distributions

## Circle distribution (used for testing):
![Circle distribution](results/circle_250k.png)

## Circumference distribution:
![Circumference distribution](results/circumference_250k.png)

## Gaussian distribution:
![Gaussian distribution](results/gaussian_250k.png)

## Toroid distribution:
![Toroid distribution](results/toroid_250k.png)