% initialize Mini-DAQ compile build and send to speedgoat and launch app

clearvars; close all; clc;


mdlName = 'MiniDAQ';
appname = 'MiniDAQapp';
buildDir = fullfile('C:','SimulinkBuild');

Ts = 1/100;    % sample time for real time processes
stopTime = '3660';

tgName = 'baseline';    % primary speedgoat (control if using 2 SG)

% =========================================================================

%% === get model information ==============================================
mdlInfo = Simulink.MDLInfo(mdlName);
mdlVersion = mdlInfo.ModelVersion;
% =========================================================================

%% === Setting up the model parameters ====================================
load_system(mdlName)

MiniDAQActiveConfig = getActiveConfigSet(mdlName);


set_param(MiniDAQActiveConfig,'StopTime',stopTime);

set_param(MiniDAQActiveConfig,'SolverType','Fixed-step','FixedStep','Ts');

save_system(mdlName)

set_param(mdlName, 'RTWVerbose', 'off');
fprintf('*** Build Simulink RT code for Speedgoat) ...\n\n')
slbuild(mdlName)

%% === Open the App =======================================================
fprintf('*** Start user app ...\n\n')
run(appname)



