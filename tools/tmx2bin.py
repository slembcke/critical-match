import sys
import struct
import base64
import xml.etree.ElementTree

filename = sys.argv[1]

map = xml.etree.ElementTree.parse(filename).getroot()
data = map.find('layer/data')
data_u32 = base64.b64decode(data.text).strip()

format = "{0}I".format(len(data_u32)/4)
values = struct.unpack(format, data_u32)
# TODO generate attribute table here.
values = [(e - 1) & 0xFF for e in values]

# Append a blank attribute table if it's the size of a full name table.
if len(values) == 960: values += [0]*64

format = "{0}B".format(len(values))
data_u8 = struct.pack(format, *values)

sys.stdout.write(data_u8)
