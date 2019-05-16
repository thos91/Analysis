#!/bin/sh
# -*- coding: utf-8 -*-

import ctypes

def wrap_function(lib, funcname, restype, argtypes):
    """Simplify wrapping ctypes functions"""
    func = lib.__getattr__(funcname)
    func.restype = restype
    func.argtypes = argtypes
    return func

libwgDecoder = ctypes.CDLL('/home/neo/Code/WAGASCI/Analysis/bin/libwgDecoder.so')
check_ChipID = wrap_function(libwgDecoder, 'check_ChipID', ctypes.c_bool, [ctypes.c_int16, ctypes.c_uint16])

print check_ChipID(10,15)
