% script to receive data from target

% clear; clc; close all

%% === parameters =========================================================
tgName = 'baseline2';
buildDir = fullfile('C:','SimulinkBuild');
mdlName = 'MiniDAQ';
timeStamp = datestr(now,'yyyy-mm-dd HH-MM-SS');
output.timeStamp = timeStamp;
dateDir = datestr(now,'yyyymmdd');
timeDir = datestr(now,'HHMMss');
year = datestr(now,'yyyy');
matFileName = 'simdata.mat';

%% === test speedgoat connection ==========================================
tg = slrealtime(tgName);

try
    tg.connect
catch ME
    fprintf('\n*** Target %s not connected. Stopping program.  Check connection.\n',tgName)
    fprintf('\n*** Matlab error \n %s \n\n',ME.getReport)
    return
end

if tg.isConnected
    fprintf('\n*** Target %s is connected at IP address %s. \n\n',tg.TargetSettings.name,tg.TargetSettings.address)
end
ipAddress = tg.TargetSettings.address;

%% === copy data from target ==============================================
newFolder = fullfile('C:','dataMD',dateDir,timeDir);
mkdir(newFolder);

system(['pscp -pw slrt -r slrt@', ipAddress, ':applications/',mdlName,' ',newFolder])

%% === Import Logged Data into MATLAB and view in Simulation Data Inspector
slrealtime.fileLogList('Directory',fullfile('C:\dataMD',[dateDir,'\',timeDir]))
slrealtime.fileLogImport(mdlName,'Directory',fullfile('C:\dataMD',[dateDir,'\',timeDir]))
% Simulink.sdi.view;

%% === Export to .mat file from Data Inspector ============================
runIDs = Simulink.sdi.getAllRunIDs;
runID = runIDs(end);

rtRun = Simulink.sdi.getRun(runID); % get data for last run

SignalData = rtRun.export;

Simulink.sdi.exportRun(runID,'to','file','filename',matFileName); % export to .mat

%% === convert to structure format ========================================

data1 = load(matFileName);
numdatasets = numElements(data1.data);

for i = 1:numdatasets
    signalCategories = fieldnames(data1.data{i}.Values);
    for j = 1:length(signalCategories)
        blockCategoryTot = data1.data{i}.BlockPath.getBlock(1);
        level = wildcardPattern + "/";
        pat = asManyOfPattern(level);
        blockCategory = extractAfter(blockCategoryTot,pat);
        signalCategory = char(signalCategories(j));
        signalNames = fieldnames(data1.data{i}.Values.(signalCategories{j}));
        for k = 1:length(signalNames)
            blockNameTot = data1.data{i}.BlockPath.getBlock(1);
            level = wildcardPattern + "/";
            pat = asManyOfPattern(level);
            blockName = extractAfter(blockNameTot,pat);
            signalName = char(signalNames(k));
            output.(signalCategory).(signalName) = data1.data{i}.Values.(signalCategory).(signalName).Data;
            output.(signalCategory).time = data1.data{i}.Values.(signalCategory).(signalName).Time;
        end
    end
end


currentDir = pwd;
projectName = evalin('base','projectName');
expname = evalin('base','expname');
trialname = evalin('base','trialname');
dataexpname = [currentDir,'\Data\',projectName,'\',expname];
datadirname = fullfile(dataexpname,trialname);
if ~exist(datadirname,'dir')
    mkdir(datadirname);
end

fname = ['d',dateDir,'_',timeDir];


save([datadirname,'\',fname,'.mat'],'output');

datafilename = [fname,'.mat'];
disp(['Data saved to ',datadirname,'\',datafilename])


addpath(genpath(dataexpname))
