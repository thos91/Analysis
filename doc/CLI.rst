=============
CLI and C API
=============
.. note::
   
   In the following the asterisk * refers to any program contained in the
   "process" folder. For example * can be ``Decode``, ``AnaHist``, etc...

All the programs in the ``process`` folder are compiled as a main object
``wg*.cc`` plus a library ``libwg*.cc``. The former is just a wrapper around the
latter, where all the fun stuff happens. So in the ``wg*.cc`` file you will find
nothing else but argument parsing. Its only purpose it to make the function
``wg*`` callable from the shell (CLI).

Usually there is only one principal function in the ``libwg*.cc`` source file,
called just ``wg*``. There may be other little helper functions defined in the
``libwg*.cc`` file but you can safely ignore them if you are just interested in
the API.

Python API
==========
I put a lot of care into making the API not only C++ but also C compatible, so
that it can be called from Python using ctypes, too. The Python wrapper for the
Analysis functions is in the ``wagasci_analysis.py`` file.
