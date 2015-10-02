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
for file_idx = 1:length(inputfilenames)
   
   load(inputfilenames{file_idx});
   
   figure(1)
   plot(offeredload_vs_thrpt(:,1), offeredload_vs_thrpt(:,2), '-d');
   
   figure(2)
   plot(offeredload_vs_pkloss(:,1), offeredload_vs_pkloss(:,2), '-d');
   
end

figure(1)
title('Throughput')
xlabel('Offered load (pk/s)'), ylabel('Throughput (pk/s)')
grid on, box on
legend('SF=1', 'SF=2', 'SF=4', 'SF=8')

figure(2)
title('Packet loss rate')
xlabel('Offered load (pk/s)'), ylabel('Packet loss rate')
grid on, box on
legend('SF=1', 'SF=2', 'SF=4', 'SF=8')
