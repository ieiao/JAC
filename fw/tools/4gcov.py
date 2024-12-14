#!/bin/python3

import sys
from PIL import Image

if len(sys.argv) != 2:
	print('Usage: %s img.xpm' %(sys.argv[0]))
	sys.exit()

img = Image.open(sys.argv[1])
o1 = Image.new("1", img.size, 1)

def get_rgb_by_index(img, index):
	palette = img.getpalette()
	start = index * 3
	end = start + 3
	r, g, b = palette[start:end]
	return (r << 16) | (g << 8) | b

colors = list()
for c in img.getcolors():
	colors.append(get_rgb_by_index(img, c[1]))
colors.sort()

print('processing...')
for i in range(0, len(img.getcolors()) - 1):
	for w in range(0, img.width):
		for h in range(0, img.height):
			c = get_rgb_by_index(img, img.getpixel((w, h)))
			if c <= colors[i]:
				o1.putpixel((w, h), 1)
			else:
				o1.putpixel((w, h), 0)
	o1.save(sys.argv[1] + 'o' + str(i) + '.xbm')
	s = o1.tobytes("xbm").decode()
	s = s.replace("\n", "")
	s = s.replace("0x", "")
	s = s.replace(",", "")
	f = open(sys.argv[1] + 'o' + str(i) + '.bin', 'wb+')
	f.write(bytes.fromhex(s))
	f.close()

o1.close()
img.close()
print('done')
