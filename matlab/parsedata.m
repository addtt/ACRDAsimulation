% This script parses the log files and saves the results in .mat files

%#ok<*ST2NM>
%#ok<*SAGROW>

clear, close all

fileList = dir('../outputfiles/');
i=1;
for i_dummy=1:length(fileList)
   if (~isempty(strfind(fileList(i_dummy).name, 'acrda')))
      filenames{i} = ['../outputfiles/', fileList(i_dummy).name];
      i=i+1;
   end
end
%filenames = {'../outputfiles/acrda-20151001-221350.log'};

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
   sysParameters(:, file_idx) = [simtime, numHosts, numReplicas, numSlots, frameDurationAtServer,...
      maxIcIter, maxSf, wndLen, wndShift, snirThresh].';
   
   for i = 1 : (2 + numHosts + 2)
      fgets(fid);
   end
   
   hostData(:,:,file_idx) = csvread(filename, (14+numHosts),   0, [(14+numHosts),   0, (14+2*numHosts-1), 5]);
   systemData(:,file_idx) = csvread(filename, (14+2*numHosts), 1, [(14+2*numHosts), 1, (14+2*numHosts), 5]);
   succRate  (file_idx) = systemData(4,file_idx);
   throughput(file_idx) = systemData(5,file_idx);
   
%    i2 = 1;
%    line = '';
%    match = 'Irradiances (units of W/m^2 nm), Mean Cosines (Mubars), and Irradiance Reflectance';
%    while isempty(strfind(line, match)) % loop over the following until the end of the file is reached.
%       line = fgets(fid); % read in one line
%       if (feof(fid))
%          return;
%       end
%    end
%    
%    for i=1:5
%       fgets(fid);
%    end
%    
%    i = 1;
%    line = fgets(fid);
%    while ~isequal(double(line), [13, 10])
%       data(i, :) = str2num(line);
%       line = fgets(fid);
%       i = i+1;
%    end
%    
%    fclose(fid);
%    
%    data = data(:, [3 6]);
%    save(strcat(filename, '_LUT.mat'), 'data')
%    clear data
   
end