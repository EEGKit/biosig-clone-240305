p=pwd
cd ..
install
cd(p)

if exist('OCTAVE_VERSION','builtin')
	pkg load mexbiosig
	pkg load optim
else
	addpath /fs3/home/schloegl/src/biosig-code/biosig4matlab/t330_StimFit
	addpath /fs3/home/schloegl/src/biosig-code/biosig4c++/mex
end

simul001; 

% load data 
[data,HDR]=mexSLOAD('test01.gdf', 1);

% run microstimfit
default.t1=round(-Fs*10e-3);
default.t2=round(+Fs*100e-3);
default.baseBegin=round(-Fs*7e-3);
default.baseEnd=round(-Fs*2e-3);
default.peakBegin=round(-Fs*1e-3);
default.peakEnd=round(+Fs*10e-3);
default.fitEnd=round(+Fs*50e-3);
default.meanN=1;
default.dir=0;
default.plotFlag=0;
default.baseFlag=0;
default.fitFlag=1;
default.thres=0.03;
default.fitFlag=1;
default.thresFlag=0;

[results, opt] = microstimfit(data, HDR.SampleRate, [0:5249]*2201+201, default);


