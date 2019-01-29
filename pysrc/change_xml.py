import argparse
import xml.etree.ElementTree as ET

parser = argparse.ArgumentParser('change parameter of xml cofig file.')
parser.add_argument('xml', help='input xml file.')
parser.add_argument('param', help='parameter you want to change.')
parser.add_argument('integer', help='new int value for parameter.',type=int)
parser.add_argument('-o', '--output', help='output new file. enter new filename.')
args = parser.parse_args()

print 'target xml : %s ' % args.xml
inputxml = args.xml

print 'target parameter : %s ' % args.param
par = args.param
	
tree = ET.parse(args.xml)
root = tree.getroot()

for asu in root.iter('asu'):
	print(asu.attrib)
	for param in asu.iter('param'):
		par_name = param.get('name')
	        if par_name == par:
	 		new_int = int(args.integer)
	 		param.text = str(new_int)
			print(par_name,param.text)
if args.output:
	tree.write(args.output, encoding="UTF-8" , xml_declaration=True)
else:
	tree.write(args.xml, encoding="utf-8" , xml_declaration=True)
