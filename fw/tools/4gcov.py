#!/bin/python3

import sys
from PIL import Image

if len(sys.argv) != 2:
	print('Usage: %s img.xpm' %(sys.argv[0]))
	sys.exit()

img = Image.open(sys.argv[1])
o1 = Image.new("1", img.size, 1)

print('processing...')
for i in range(0, len(img.getcolors()) - 1):
	for w in range(0, img.width):
		for h in range(0, img.height):
			if img.getpixel((w, h)) <= img.getcolors()[i][1]:
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
