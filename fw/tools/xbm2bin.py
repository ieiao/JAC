#!/bin/python3

import sys
from PIL import Image

if len(sys.argv) != 2:
	print('Usage: %s img.xbm' %(sys.argv[0]))
	sys.exit()

img = Image.open(sys.argv[1])
str = img.tobytes("xbm").decode()
str = str.replace("\n", "")
str = str.replace("0x", "")
str = str.replace(",", "")
f = open(sys.argv[1] + '.bin', 'wb+')
f.write(bytes.fromhex(str))
f.close()

print(sys.argv[1])
print('width: %d' % img.width)
print('height: %d' % img.height)
img.close()
print('done')
