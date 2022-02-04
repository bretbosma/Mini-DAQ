% script to receive data from target

% clear; clc; close all

%% === parameters =========================================================
tgName = 'baseline2';
buildDir = fullfile('C:','SimulinkBuild');
mdlName = 'MiniDAQ';
dateDir = datestr(now,'yyyymmdd');
timeDir = datestr(now,'HHMMss');
% sharename = 'Z:';
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
%             signalNames = fieldnames(data1.data{i}.Values.(signalCategories{j}).(signalName));
            output.(signalCategory).(signalName) = data1.data{i}.Values.(signalCategory).(signalName).Data;
            output.(signalCategory).time = data1.data{i}.Values.(signalCategory).(signalName).Time;
        end
    end
end
%% === convert timestamp to datetime format================================
% output.Timestamp.UTCtime = datetime(output.Timestamp.timestamp,'ConvertFrom','epochtime','TicksPerSecond',1e9,'Format','eee yyyy/MM/dd HH:mm:ss.SSSSSSSSS');


% projectName = app.ProjectEditField.Value;
% expname = app.ExperimentEditField.Value;
% trialnumber = app.TrialSpinner.Value;
% trialname = ['\Trial',num2str(trialnumber,'%02d')];

currentDir = pwd;
projectName = evalin('base','projectName');
expname = evalin('base','expname');
trialname = evalin('base','trialname');
dataexpname = [currentDir,'\',projectName,'\',expname];
datadirname = fullfile(dataexpname,trialname);
if ~exist(datadirname,'dir')
    mkdir(datadirname);
end
formatOut = 'yyyymmdd_HHMMSS';
fname = ['d',dateDir,'_',timeDir];


save([datadirname,'\',fname,'.mat'],'output');

disp(['Data saved to ',datadirname,'\',fname,'.mat'])

% if strcmp(app.HWRLShareButton.Text,'Push to Share')
%     % push data to share
%     % build the directory structure. assumes HWRL project share conventions
%     if ~exist(sharename,'dir') % give up if no HWRL share
%         disp([projname,': cannot find ',sharename])
%     elseif ~exist(fullfile(sharename,'projects'),'dir') % give up if no projects folder
%         disp([projname,': cannot find ',fullfile(sharename,'projects')])
%     elseif ~exist(fullfile(sharename,'projects',year),'dir') % give up if no projects/year folder
%         disp([projname,': cannot find ',fullfile(sharename,'projects',year)])
%     elseif ~exist(fullfile(sharename,'projects',year,projname),'dir') % give up if no project folder
%         disp([projname,': cannot find ',fullfile(sharename,'projects',year,projname)])
%     elseif ~exist(fullfile(sharename,'projects',year,projname,'data','raw'),'dir') % give up if no raw folder
%         disp([projname,': cannot find ',fullfile(projname,'data','raw')])
%     else % we found the raw folder and can proceed
%         expdirname = fullfile(sharename,'projects',year,projname,'data','raw',expname);
%         if ~exist(expdirname,'dir') % create a experiment data directory if it doesn't exist yet
%             mkdir(expdirname);
%         end
%         trialdirname = fullfile(expdirname,trialname);
%         if ~exist(trialdirname,'dir') % create a trial data directory if it doesn't exist yet
%             mkdir(trialdirname);
%         end
%         copyfile([datadirname,'\',fname,'.mat'],trialdirname)
%         disp(['Data saved to ',trialdirname,'\',fname,'.mat'])
%     end
%
% else
%     % don't push data to share
%     disp(['Data not saved to HWRL share'])
% end

addpath(genpath(dataexpname))
% load([fname,'.mat'])