% Look for all .mat files in the current folder and make the desired plots.

clear, close all

fileList = dir('.');
i=1;
for i_dummy=1:length(fileList)
   if (~isempty(strfind(fileList(i_dummy).name, '.mat')))
      inputfilenames{i} = fileList(i_dummy).name;
      i=i+1;
   end
end

for i=1:length(inputfilenames)
   legendEntries{i} = strrep(inputfilenames{i}, '_', ', ');
   legendEntries{i} = strrep(legendEntries{i}, '.mat', '');
end

figure(1), hold on
figure(2), hold on
figure(3), hold on
figure(4), hold on
figure(5), hold on
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
   else
      plot(arrRate_vs_thrpt(:,1)/numHosts, arrRate_vs_thrpt(:,2)/numHosts);
   end
      
end

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

close(1,2,3)