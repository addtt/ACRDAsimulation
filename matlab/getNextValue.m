function [ output ] = getNextValue( fid )
%GETNEXTVALUE Summary of this function goes here
%   Detailed explanation goes here

cellarray = strsplit(fgets(fid),',');
output = cellarray{2};

end

