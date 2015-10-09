% Look for all .mat files in the current folder and make the desired plots.

clear, close all

resultsDirectory = './';
fileList = dir(resultsDirectory);
i=1;
for i_dummy=1:length(fileList)
   if (~isempty(strfind(fileList(i_dummy).name, '.mat')))
      inputfilenames{i} = [resultsDirectory, fileList(i_dummy).name];
      i=i+1;
   end
end

for i=1:length(inputfilenames)
   legendEntries{i} = strrep(inputfilenames{i}, '_', ', ');
   legendEntries{i} = strrep(legendEntries{i}, '.mat', '');
   legendEntries{i} = strrep(legendEntries{i}, resultsDirectory, '');
end

figure(1), hold on
figure(2), hold on
figure(3), hold on
figure(4), hold on
figure(5), hold on
figure(6), hold on
figure(7), hold on
figure(8), hold on
figure(9), hold on
for file_idx = 1:length(inputfilenames)
   
   load(inputfilenames{file_idx});
   
   figure(1)
   plot(offeredload_vs_thrpt(:,1), offeredload_vs_thrpt(:,2), '-d');
   
   figure(2)
   plot(offeredload_vs_pkloss(:,1), offeredload_vs_pkloss(:,2), '-d');
   
   figure(3)
   plot(offeredload_vs_thrpt(:,1)/numHosts, offeredload_vs_thrpt(:,2)/numHosts, '-d');
   
   figure(4)
   plot(arrRate_vs_thrpt(:,1), arrRate_vs_thrpt(:,2)); 
   
   figure(5)
   if (strfind(legendEntries{file_idx}, ' s1,'))
      plot(arrRate_vs_thrpt(:,1)/numHosts, arrRate_vs_thrpt(:,2)/numHosts, '-.');
   elseif (strfind(legendEntries{file_idx}, ' s6,'))
      plot(arrRate_vs_thrpt(:,1)/numHosts, arrRate_vs_thrpt(:,2)/numHosts, '--');
   else
      plot(arrRate_vs_thrpt(:,1)/numHosts, arrRate_vs_thrpt(:,2)/numHosts);
   end
   
   figure(9)
   arrRate_vs_pkloss = sortrows([systemarrivalrate', (1-succRate)']);
   if (strfind(legendEntries{file_idx}, ' s1,'))
      plot(arrRate_vs_pkloss(:,1)/numHosts, arrRate_vs_pkloss(:,2), '-.');
   elseif (strfind(legendEntries{file_idx}, ' s6,'))
      plot(arrRate_vs_pkloss(:,1)/numHosts, arrRate_vs_pkloss(:,2), '--');
   else
      plot(arrRate_vs_pkloss(:,1)/numHosts, arrRate_vs_pkloss(:,2));
   end
   
   if exist('sf_vs_thrpt', 'var')
      figure(6)
      plot(sf_vs_thrpt(:,1), sf_vs_thrpt(:,2)/numHosts);
   end
   
   if exist('maxSfVec', 'var')
      energyConsPerUser = offeredload .* maxSfVec * numReplicas / numHosts;
      sf_vs_energyConsPerUser = sortrows([maxSfVec', energyConsPerUser']);
      figure(7)
      plot(sf_vs_energyConsPerUser(:,1), sf_vs_energyConsPerUser(:,2))
      
      sf_vs_pkloss = sortrows([maxSfVec', (1-succRate)']);
      figure(8)
      plot(sf_vs_pkloss(:,1), sf_vs_pkloss(:,2));
   end
end

warning off

figure(1)
title('System throughput')
xlabel('Offered load (pk/s)'), ylabel('System throughput (pk/s)')
grid on, box on
legend(legendEntries)

figure(2)
title('Packet loss rate')
xlabel('Offered load (pk/s)'), ylabel('Packet loss rate')
grid on, box on
legend(legendEntries)

figure(3)
title('Throughput per user')
xlabel('Offered load (pk/s/usr)'), ylabel('Throughput per user (pk/s/usr)')
grid on, box on
legend(legendEntries)

figure(4)
title('System throughput')
xlabel('System arrival rate (pk/s)'), ylabel('System throughput (pk/s)')
grid on, box on
legend(legendEntries)

figure(5)
title('Throughput per user')
xlabel('Average arrival rate (pk/s/usr)'), ylabel('Throughput per user (pk/s/usr)')
grid on, box on
legend(legendEntries)

figure(9)
title('Packet loss rate')
xlabel('Average arrival rate (pk/s/usr)'), ylabel('Packet loss rate')
grid on, box on
legend(legendEntries)
set(gca, 'yscale', 'log');

figure(6)
title('Throughput per user varying SF')
xlabel('Spreading Factor'), ylabel('Throughput per user (pk/s/usr)')
grid on, box on
legend(legendEntries)

figure(7)
title('Energy consumption per user')
xlabel('Spreading Factor'), ylabel('Energy consumption per user')
grid on, box on
legend(legendEntries)

figure(8)
title('Packet loss rate')
xlabel('Spreading Factor'), ylabel('Packet loss rate')
grid on, box on
legend(legendEntries)

close(1,2,3)
close(4)
close(6,7,8)