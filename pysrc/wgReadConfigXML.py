import argparse
import os,os.path,csv
import xml.etree.cElementTree as ET

#######################################
def get_bitstream_from_xml(inputxml,ichip):
  if inputxml:
    if ichip > -1 :
      ichip = int(ichip)
      tree = ET.parse(inputxml)
      root = tree.getroot()
      bitstream = root.findtext("./ecal/domain/acqpc/gdcc/dif/asu[@name='asu_1_1_1_%d']/spiroc2d/param[@name='spiroc2d_bitstream']"%(ichip+1))
      return bitstream
    else:
      print "Error!! undefined chip vakue!"
      quit()
  else:
    print "Error!! inputxml is NULL!"
    quit()
#######################################
def get_subadd_value(bitstream,target_subadd,target_ch): 
  SEMIOFF_DIR=os.environ['WAGASCI_MAINDIR']
  DIRECTORY = [""]
  CommandDir = ["%s/pysrc/"%(SEMIOFF_DIR)]
  CHIP = ["spiroc2d"]
  ConfigFile = [""]
  SubAddress = int(target_subadd)
  Num_chan = int(target_ch)

  name=[]
  bits=[]
  remark=[]
  subadd=[]
  default=[]
  value=[]
  NumConfigHex = 300
  TotalBits = 1186
  Margin = 6 
  NumRow = 85
  NumConfigBin = TotalBits + Margin + 2
  
  if(SubAddress<-1 or SubAddress>TotalBits-1):
    print "Error: SubbAddress is out of the range: {0} - {1}".format(0, TotalBits[0]-1)
    quit()
  if(Num_chan<-1 or Num_chan>36):
    print "Error: channel is out of the range : 0 - 36"
    quit()
  
  #bitstream in xml file is 299 bits, so add 1 bit for suiting 
  config = str(bitstream) + '0'

  #check the bitstream structure
  if(config[0:2]!="0x"):
    print "Error: the config file is not hex."
    quit()
  if(len(config.strip())!=NumConfigHex):
    print "Error: the length of config text is not correct."
    quit()

  #convert bitstream hex to bi
  config = "0b{1:0{0}b}".format((NumConfigBin-2),int(config,16))
  
  if(len(config)!=NumConfigBin):
      print "Error: the length of binary config is not correct."
      quit()
  
  #open csv file 
  ScFileName = CommandDir[0] + CHIP[0] + ".csv"
  ScFile = open(ScFileName,'rb')
  ScTable = csv.reader(ScFile)

  #convert bitstream from binary to readable string
  nLine = 0
  msb = -Margin-1
  lsb = -Margin-1
  target_value = ""
  target_name = ""
  search_subbit = False
  if target_ch > -1:
    search_subbit = True  
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
          default.append(row[4])
          msb=lsb
          lsb=msb-int(row[1])
          value.append(config[msb:lsb:-1])
          if(int(row[3])==int(SubAddress)):
            target_name = row[0]
            subBits = int(row[1])/36
            value_bits = config[msb:lsb:-1]
            if(search_subbit == True and int(row[1])%36==0 and int(subBits)>1):
              ch = -1
              for x in value_bits:
                ch += 1 
                if ch == target_ch:
                  start_bits= int(ch) * int(subBits);
                  end_bits= int(ch+1) * int(subBits);
                  target_value += "{0}".format(value_bits[start_bits:end_bits])
            else:
              target_value += "{0}".format(value_bits)
  return  target_value
################################################

def get_config_value(inputfile,chip_num,subadd,channel):
  true_inputfile=False
  true_chip_num=False
  true_subadd=False
  true_channel=False
  if inputfile:
    filename, ext = os.path.splitext(inputfile)
    if ext != ".xml":
      print "Error! wrong extension! file must be .xml"
    else:
      if os.path.exists(inputfile):
        true_inputfile=True
      else:  
        print "Error! inputfile doesn't exist!"
  else:
    print "Error! please enter inputfile"      
   
  if chip_num > -1:
    true_chip_num=True
  else:  
    print "Error! please enter chip_number"      

  if subadd > -1:
    true_subadd=True
  else:
    print "Error! please enter sub-address"

  if channel>-2 and channel<36:
    true_channel=True
    
  if(true_inputfile and true_chip_num and true_subadd and true_channel):
    bitstream = get_bitstream_from_xml(inputfile,chip_num)
    return get_subadd_value(bitstream,subadd,channel)
  else:
    quit()
 
################################################################
   
def fill_value(inputfile,outputfile,ichip,ichan):
  tree = ET.parse(outputfile)
  data = tree.getroot()
  for config in data.findall("config"):
    if(int(config.find("chipid").text)!=int(ichip)):
      print "Error!! read different chip config!!!"
      quit()
    if(int(config.find("chanid").text)!=int(ichan)): 
      print "Error!! read different channel config!!!"
      quit()
    for subadd in [931,941,37,367,1006]:
      if subadd == 931:
        config.find("trigth").text = str(int(get_config_value(inputfile,ichip,subadd,-1),2))
      elif subadd==941:
        config.find("gainth").text = str(int(get_config_value(inputfile,ichip,subadd,-1),2))
      elif subadd==37:
        i_value = int(get_config_value(inputfile,ichip,subadd,int(ichan)),2)
        t_value = i_value/2
        config.find("inputDAC").text = str(t_value);
      elif subadd==367:
        i_value = int(get_config_value(inputfile,ichip,subadd,int(ichan)),2)/8
        HG_value = i_value/64
        LG_value = i_value%64
        config.find("HG").text = str(HG_value)
        config.find("LG").text = str(LG_value)
      elif subadd==1006:
        config.find("trig_adj").text = str(int(get_config_value(inputfile,ichip,subadd,int(ichan)),2))
      else:
        print "Warning! wrong subaddress!"
        quit()
  ET.ElementTree(data).write(outputfile,encoding="utf-8", xml_declaration=True)

#######################################
parser = argparse.ArgumentParser(description="add configure parameter for xml.")
parser.add_argument("-f","--inputfile", help="input xml file.")
parser.add_argument("-o","--outputfile", help="output xml file.")
parser.add_argument("-i","--chip_num",help="chip number.")
parser.add_argument("-j","--chan_num",help="chan number.")
parser.add_argument("-r","--overwrite",action='store_const',default="no",const="yes", help="overwrite.")
args = parser.parse_args()

if(args.inputfile and args.outputfile and args.chip_num and args.chan_num):
  fill_value(args.inputfile,args.outputfile,args.chip_num,args.chan_num)
else:
  print "Error! Bad option!"
  quit()
  
       

