import sys
filename = sys.argv[1]

import pprint
p = pprint.PrettyPrinter(indent=4).pprint

import xml.etree.ElementTree
map = xml.etree.ElementTree.parse(filename).getroot()
data = map.find('layer/data')

import base64
data_u32 = base64.b64decode(data.text).strip()

import struct
values = struct.unpack("960I", data_u32)
values = [e - 1 for e in values] + [0]*64
data_u8 = struct.pack("1024B", *values)

sys.stdout.write(data_u8)
