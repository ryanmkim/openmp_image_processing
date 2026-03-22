import png, glob, random

files = glob.glob("tiles_out/*.png")
picks = [random.choice(files) for _ in range(20)]

tiles = []
for f in picks:
    r = png.Reader(filename=f)
    w, h, rows, info = r.read()
    tiles.append(list(rows))

th = len(tiles[0])
tw = len(tiles[0][0])
cols, rows = 5, 4

out_rows = []
for r in range(rows):
    for y in range(th):
        row = bytearray()
        for c in range(cols):
            row.extend(tiles[r * cols + c][y])
        out_rows.append(row)

w = png.Writer(width=cols * (tw // 3), height=rows * th, greyscale=False)
with open("check.png", "wb") as f:
    w.write(f, out_rows)

print("saved check.png")
