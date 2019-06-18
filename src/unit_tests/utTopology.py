import ctypes
import json

lib = ctypes.cdll.LoadLibrary('../../lib/libwagasci.so')
lib.FreeTopologyCtypes.argtypes = ctypes.c_void_p,
lib.FreeTopologyCtypes.restype = None
lib.GetTopologyCtypes.argtypes = [ctypes.c_char_p]
lib.GetTopologyCtypes.restype = ctypes.c_void_p

topology_cstring = lib.GetTopologyCtypes("/home/neo/Code/WAGASCI/Configs/wagasci_config_6asu.xml")
print hex(topology_cstring)
topology_string = ctypes.cast(topology_cstring, ctypes.c_char_p).value
print topology_string
lib.FreeTopologyCtypes(topology_cstring)
topology = json.loads(topology_string)
print topology
