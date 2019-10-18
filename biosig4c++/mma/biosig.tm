
:Begin:
:Function:      sload
:Pattern:       Sload[fn_String, i_List]
:Arguments:     {fn, i}
:ArgumentTypes: {String, IntegerList}
:ReturnType:    Manual
:End:

:Evaluate: Sload::usage = "Sload[filename, {ne,ng,ns}] load data sweeps into mathematica workspace.
 ne, ng, and ns are the number of the experiment, the number of the series from this experiment and
 the number of the sweep from this series sweep, respectivly. 0 can be used as wildcard to select all
 sweeps.\nExamples: data = Sload(\"abc.dat\",{1,5,0}) selects all sweeps from 5th series of first experiment; {0,0,0} selects
 all sweeps from file \"abc.dat\".\n
 The output ist a list of three elements, data[[1]] contains the 2-dim array of data samples,
 data[[2]] contains the time axis in seconds, and data[[3]] contains the header information in serialized JSON string.
 It can be converted with ImportString[data[[3]], \"JSON\"]. \n
 \nNOTE: If sweeps were sampled with different sampling rates, all data is converted to the
 least common multiple of the various sampling rates. (e.g. loading a 20kHz and a 25kHz sweep simultaneously, both sweeps are converted to 100kHz).
 \n\nCompiled on __DATE__"

:Evaluate: Sload::failed="Failed to load file"


:Begin:
:Function:      uload
:Pattern:       Uload[fn_String, i_List]
:Arguments:     {fn, i}
:ArgumentTypes: {String, IntegerList}
:ReturnType:    Manual
:End:

:Evaluate: Uload::usage = "Uload[filename, {ne,ng,ns}] load data sweeps into mathematica workspace.
 uload is very similar to sload except that it returns the raw digital values without scaling
 it's the \"un-calibrated\" load function.\n
 ne, ng, and ns are the number of the experiment, the number of the series from this experiment and
 the number of the sweep from this series sweep, respectivly. 0 can be used as wildcard to select all
 sweeps.\nExamples: data = Uload(\"abc.dat\",{1,5,0}) selects all sweeps from 5th series of first experiment; {0,0,0} selects
 all sweeps from file \"abc.dat\".\n
 The output ist a list of three elements, data[[1]] contains the 2-dim array of data samples,
 data[[2]] contains the time axis in seconds, and data[[3]] contains the header information in serialized JSON string.
 It can be converted with ImportString[data[[3]], \"JSON\"]. \n
 \nNOTE: If sweeps were sampled with different sampling rates, all data is converted to the
 least common multiple of the various sampling rates. (e.g. loading a 20kHz and a 25kHz sweep simultaneously, both sweeps are converted to 100kHz).
 \n\nCompiled on __DATE__"

:Evaluate: Uload::failed="Failed to load file"

