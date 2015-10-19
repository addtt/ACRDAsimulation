% This script parses the log files in order to save the results in a .mat file.
% It looks for all acrda logs in ../outputfiles/ and parses them, then puts things
% together doing statistics and clears useless stuff. Then you can save the whole
% workspace in the current folder and plotdata will plot data from all files together.

%#ok<*ST2NM>
%#ok<*SAGROW>

clear, close all

% BLUE_COLOR = [0 0.4470 0.7410];
% RED_COLOR  = [0.8500 0.3250 0.0980];
% YELL_COLOR = [0.9290 0.6940 0.1250];

fileList = dir('../outputfiles/');
i=1;
for i_dummy=1:length(fileList)
   if (~isempty(strfind(fileList(i_dummy).name, 'acrda')))
      filenames{i} = ['../outputfiles/', fileList(i_dummy).name];
      i=i+1;
   end
end

for file_idx = 1:length(filenames)
   
   % Open the file
   filename = filenames{file_idx};
   fid = fopen(filename);
   
   % Parse system parameters
   simtime              = str2double(getNextValue(fid));
   numHosts             = str2num(getNextValue(fid));
   numReplicas          = str2num(getNextValue(fid));
   numSlots             = str2num(getNextValue(fid));
   frameDurationAtServer= str2double(getNextValue(fid));
   maxIcIter            = str2num(getNextValue(fid));
   maxSf                = str2num(getNextValue(fid));
   wndLen               = str2double(getNextValue(fid));
   wndShift             = str2double(getNextValue(fid));
   snirThresh           = str2double(getNextValue(fid));
   sysParams(:, file_idx) = [simtime, numHosts, numReplicas, numSlots, frameDurationAtServer,...
      maxIcIter, maxSf, wndLen, wndShift, snirThresh].';
   
   % Skip lines
   for i = 1 : 2
      fgets(fid);
   end
   
   hostParams{file_idx} = textscan(fid, '%d%d%f%s%f%s%f', numHosts, 'delimiter',',');
   %csvread(filename, 12,   0, [12,   0, (12+numHosts-1), 5]);
   hostResults{file_idx} = csvread(filename, (14+numHosts),   0, [(14+numHosts),   0, (14+2*numHosts-1), 5]);
   systemResults(:,file_idx) = csvread(filename, (14+2*numHosts), 1, [(14+2*numHosts), 1, (14+2*numHosts), 5]);
   succRate  (file_idx) = systemResults(4,file_idx);
   throughput(file_idx) = systemResults(5,file_idx);
   offeredload(file_idx) = systemResults(2,file_idx) / simtime; % attempted packets divided by time
   arrivalrates{file_idx} = 1 ./ hostParams{file_idx}{5};
   systemarrivalrate(file_idx) = sum(arrivalrates{file_idx});
   maxSfVec(file_idx) = sysParams(7, file_idx);
   
end

% Compute throughput average with fixed number of hosts
% numHosts = unique(sysParameters(2,:));
% for i=1:length(numHosts)
%    indices = find(sysParameters(2,:) == numHosts(i));
%    throughputByNumHosts{i} = throughput(indices).';
%    throughputByNumHosts_avg(i) = mean(throughputByNumHosts{i});
% end
% figure, scatter(sysParameters(2,:), throughput)
% hold on
% plot(numHosts, throughputByNumHosts_avg, 'Color', BLUE_COLOR)
% title('Throughput')
% xlabel('Number of hosts'), ylabel('Throughput (pk/s)')
% grid on, box on

offeredload_vs_thrpt = sortrows([offeredload', throughput']);

figure, plot(offeredload_vs_thrpt(:,1), offeredload_vs_thrpt(:,2), '-d');
title('Throughput')
xlabel('Offered load (pk/s)'), ylabel('Throughput (pk/s)')
grid on, box on

arrRate_vs_thrpt = sortrows([systemarrivalrate', throughput']);

figure, plot(arrRate_vs_thrpt(:,1), arrRate_vs_thrpt(:,2), '-d');
title('System throughput')
xlabel('System arrival rate (pk/s)'), ylabel('Throughput (pk/s)')
grid on, box on

figure, plot(arrRate_vs_thrpt(:,1)/numHosts, arrRate_vs_thrpt(:,2)/numHosts, '-d');
title('Throughput per user')
xlabel('Average arrival rate (pk/s/usr)'), ylabel('Throughput per user (pk/s/usr)')
grid on, box on

offeredload_vs_pkloss = sortrows([offeredload', (1-succRate)']);

figure, plot(offeredload_vs_pkloss(:,1), offeredload_vs_pkloss(:,2), '-d');
title('Packet loss rate')
xlabel('Offered load (pk/s)'), ylabel('Packet loss rate')
grid on, box on

% nhosts_vs_thrpt = sortrows([sysParameters(2,:)', throughput']);
% 
% figure, plot(nhosts_vs_thrpt(:,1), nhosts_vs_thrpt(:,2), '-d');
% title('Throughput')
% xlabel('Number of hosts'), ylabel('Throughput (pk/s)')
% grid on, box on

% nhosts_vs_pkloss = sortrows([sysParameters(2,:)', (1-succRate)']);
% 
% figure, plot(nhosts_vs_pkloss(:,1), nhosts_vs_pkloss(:,2), '-d');
% title('Packet loss rate')
% xlabel('Number of hosts'), ylabel('Packet loss rate')
% grid on, box on

sf_vs_thrpt = sortrows([maxSfVec', throughput']);

figure, plot(sf_vs_thrpt(:,1), sf_vs_thrpt(:,2)/numHosts, '-d');
title('Throughput varying SF')
xlabel('Spreading Factor'), ylabel('Throughput per user (pk/s/usr)')
grid on, box on

nslots_vs_thrput = sortrows([sysParams(4, :)', throughput']);

figure, plot(nslots_vs_thrput(:,1), nslots_vs_thrput(:,2)/numHosts, '-d');
title('Throughput varying numSlots')
xlabel('Number of slots'), ylabel('Throughput per user (pk/s/usr)')
grid on, box on


fclose('all');
clear ans BLUE_COLOR RED_COLOR YELL_COLOR filename i i_dummy file_idx fid indices fileList

