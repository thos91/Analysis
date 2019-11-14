add_subdirectory(wgDecoder)
# Sometimes the compile of wgRawEmulator fails for MacOS. No other
# component depends on it so it does not need to be compiled.
# add_subdirectory(wgRawEmulator)
add_subdirectory(wgMakeHist)
add_subdirectory(wgAnaHist)
add_subdirectory(wgAnaHistSummary)
add_subdirectory(wgPedestalCalib)
add_subdirectory(wgChangeConfig)
add_subdirectory(wgOptimize)
add_subdirectory(wgGainCalib)
add_subdirectory(wgGainCalibUgly)
add_subdirectory(wgScurve)
add_subdirectory(wgOnlineMonitor)
