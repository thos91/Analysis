import ctypes
import json

lib = ctypes.cdll.LoadLibrary('../../lib/libwagasci.so')
lib.FreeTopologyCtypes.argtypes = [ctypes.c_void_p]
lib.FreeTopologyCtypes.restype = None
lib.GetDifTopologyCtypes.argtypes = [ctypes.c_char_p]
lib.GetDifTopologyCtypes.restype = ctypes.c_void_p

topology_cstring = lib.GetDifTopologyCtypes("/home/neo/Desktop/test/AcqConfig/wagasci_config_test.xml")
print hex(topology_cstring)
topology_string = ctypes.cast(topology_cstring, ctypes.c_char_p).value
print topology_string
lib.FreeTopologyCtypes(topology_cstring)
topology = json.loads(topology_string)
print topology
