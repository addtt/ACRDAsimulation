clear, close all

fileList = dir('.');
i=1;
for i_dummy=1:length(fileList)
   if (~isempty(strfind(fileList(i_dummy).name, '.mat')))
      inputfilenames{i} = fileList(i_dummy).name;
      i=i+1;
   end
end

figure(1), hold on
figure(2), hold on
figure(3), hold on
figure(4), hold on
for file_idx = 1:length(inputfilenames)
   
   load(inputfilenames{file_idx});
   
   figure(1)
   plot(offeredload_vs_thrpt(:,1), offeredload_vs_thrpt(:,2), '-d');
   
   figure(2)
   plot(offeredload_vs_pkloss(:,1), offeredload_vs_pkloss(:,2), '-d');
    
   figure(3)
   plot(offeredload_vs_thrpt(:,1)/numHosts, offeredload_vs_thrpt(:,2)/numHosts, '-d');
   
   figure(4)
   plot(arrRate_vs_thrpt(:,1), arrRate_vs_thrpt(:,2), '-d'); 
   
end

figure(1)
title('Throughput')
xlabel('Offered load (pk/s)'), ylabel('Throughput (pk/s)')
grid on, box on
legend(inputfilenames)

figure(2)
title('Packet loss rate')
xlabel('Offered load (pk/s)'), ylabel('Packet loss rate')
grid on, box on
legend(inputfilenames)

figure(3)
title('Throughput per user')
xlabel('Offered load (pk/s/usr)'), ylabel('Throughput per user (pk/s/usr)')
grid on, box on
legend(inputfilenames)

figure(4)
title('Throughput')
xlabel('Offered load (pk/s)'), ylabel('Throughput (pk/s)')
grid on, box on
legend(inputfilenames)