# image tiling and filtering using openmp
splits an image into filters and applies a filter to each tile in parallel using openmp. 
- grayscale, threshold, sharpen, edge, blur and none
- input image is divided into a grid of 64x64 pixel tiles.


Build and run
```bash
g++ -std=c++17 -fopenmp -O2 -o tilefilter main.cpp -lm

./tilefilter <input.png> <filter> [output_dir]
```

