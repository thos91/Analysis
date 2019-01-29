#!/usr/bin/python2
# -*- coding: utf-8 -*-

import sys, time, os, commands
import csv
import argparse

import argparse
import xml.etree.ElementTree as ET

SEMIOFF_DIR=os.environ['WAGASCI_SEMIOFFDIR']
DIRECTORY = [""]
CommandDir = ["%s/RunCommand/python/"%(SEMIOFF_DIR)]
CHIP = [""]
ConfigFile = [""]
SubAddr = [-1]
NewValue = [-1]
NewValueCh = [-1]
NewConfig = [""]
FindWord = [""]

OutputFile = [""]

NumConfigHex = [0]
NumConfigBin = [0]
TotalBits = [0]
Margin = [0]
NumRow = [0]

name = []
bits = []
remark = []
subadd = []
defalut = []
value = []

###########################################

def check():
 
  if(CHIP[0]!="spiroc2b" and CHIP[0]!="spiroc2d"):
      print "Error: Chip type must be spiroc2b/spiroc2d"
      quit()
  elif(CHIP[0]=="spiroc2b"):
      NumConfigHex[0] = 234+2
      TotalBits[0] = 929
      Margin[0] = 7
      NumRow[0] = 69
  elif(CHIP[0]=="spiroc2d"):
      NumConfigHex[0] = 298+2
      TotalBits[0] = 1186
      Margin[0] = 6 
      NumRow[0] = 85
      DIRECTORY[0] += ""
  NumConfigBin[0] = TotalBits[0] + Margin[0] + 2
  
  if(SubAddr[0]<-1 or SubAddr[0]>TotalBits[0]-1):
      print "Error: The SubbAddr is out of the range: {0} - {1}".format(0, TotalBits[0]-1)
      quit()
  
  config = ""
  temp, ext = os.path.splitext(ConfigFile[0])
  if(ext!=".txt"):
      print "Error: Config file must be txt file."
      quit()
  else:
      ConfigFile[0] = DIRECTORY[0] + ConfigFile[0]
      if(os.path.isfile(ConfigFile[0])):
          nLine = 0
          for line in open(ConfigFile[0],'r'):
              if(nLine>0):
                  print "Error: more than 1 line for config file."
                  quit()
              else:           
                  config = line 
              if(config[0:2]!="0x"):
                  print "Error: the config file is not hex."
                  quit()
              elif(len(config.strip())!=NumConfigHex[0]):
                  print "Error: the length of config text is not correct."
                  quit()
              else:
                  nLine += 1
      else:
          print "Error: There is no such file {0}".format(ConfigFile[0])
          quit()
  
  
  #print "{0}".format(config)
  config = "0b{1:0{0}b}".format((NumConfigBin[0]-2),int(config,16))
  #print "{0}".format(config)
  
  if(len(config)!=NumConfigBin[0]):
      print "Error: the length of binary config is not correct."
      quit()
  
  
  ScFileName = CommandDir[0] + CHIP[0] + ".csv"
  ScFile = open(ScFileName,'rb')
  ScTable = csv.reader(ScFile)
  

  
  nLine = 0
  msb = -Margin[0]-1
  lsb = -Margin[0]-1
  for row in ScTable:
      nLine += 1
      if(len(row) != 5):
          print "The number of array at line {0} in {1} is not correct: {2}".format(nLine,ScFileName,len(row))
          quit()
      else:
          name.append(row[0])
          bits.append(row[1])
          remark.append(row[2])
          subadd.append(row[3])
          defalut.append(row[4])
  
          msb=lsb
          lsb=msb-int(row[1])
          value.append(config[msb:lsb:-1])
  
  isSubAdd = False
  for row in range(0,NumRow[0]):
      opt_subadd = False
      opt_find = False
      if name[row].find(FindWord[0])!=-1:
	opt_find = True

      if SubAddr[0]==-1:
	   opt_subadd = True
	   isSubAdd = True
      else:
	   if SubAddr[0]==int(subadd[row]):
		opt_subadd = True
		isSubAdd = True

      if opt_subadd==True and opt_find==True:	
        subBits = int(bits[row])/36
        if(int(bits[row])%36==0 and subBits>1):
            value[row] = [ value[row][i:i+subBits] for i in range(0,len(value[row]),subBits)]
        print ">> {0:40}, Bits:{1:>4} , SubAdd {2:>4} , {3}".format(name[row],bits[row],subadd[row],value[row])
	opt_subadd=False
	opt_find=False
      
  if isSubAdd==False:
	print "No corresponding sub-address."
	quit()

  return


parser = argparse.ArgumentParser(description="get configure from xml file.")
parser.add_argument("-f","--config_file", help="configure file (.txt).")
parser.add_argument("-o","--output",default="new_config.txt",help="Name of output file")
parser.add_argument("-r","--overwrite",action='store_const',default="no",const="yes", help="overwrite.")
parser.add_argument("-s","--subadd",type=int, help="select the sub-addrees of bits")
parser.add_argument("-b","--subbit",default=36,type=int,help="select one channel you want to change form 0-35 ,or 36(all)")

args = parser.parse_args()

print 'target xml : %s ' % args.config_file
inputxml = args.config_file
	
tree = ET.parse(args.config_file)
root = tree.getroot()

if args.chip_type and args.config_file and args.command:
	print "Arguments are OK \n"
	CHIP[0] += args.chip_type
	ConfigFile[0] = args.config_file

	if args.subadd!=None:
 	  SubAddr[0] = args.subadd
	if args.index!=None:
	  FindWord[0] = args.index
		
	if args.overwrite=="yes":
	  OutputFile[0] += ConfigFile[0]
	  #print "#### The file will be overwritten!!! #### \n "
	elif args.overwrite=="no":
	  OutputFile[0] = "{0}_{1}".format(CHIP[0],args.output)

	check()

	if args.command == "edit":
		if args.subadd==None:
		  print "Select sub-addres."
		  quit()
		else:
		  print "what is new value?"
		  NewValue[0] = bin(int(args.value))
		  NewValueCh[0] = int(args.subbit)
		  edit()

else:
	print "Not good arguments. Chip type, command, and configuration file are required at least.\n\n"
	parser.print_help()

	quit()



