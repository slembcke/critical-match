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
# TODO is the padding weird?
attribs = [(e - 1) >> 8 for e in values] + [0]*64
values = [(e - 1) & 0xFF for e in values]

def attr_byte(i):
	i0 = 4*(i % 8) + 128*(i/8)
	return (attribs[i0] << 0) | (attribs[i0 + 2] << 2) | (attribs[i0 + 64] << 4) | (attribs[i0 + 66] << 6)

if len(values) == 960:
	values += [attr_byte(i) for i in range(64)]

format = "{0}B".format(len(values))
data_u8 = struct.pack(format, *values)

sys.stdout.write(data_u8)
