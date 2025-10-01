function [coms, desc, list] = comdesc_get_list()
% Using tool get_com_descriptor.exe obtains list of available COM ports
% and their user friendly descriptors.
% 
% usage:
%  [coms, desc, list] = comdesc_get_list()
%
% where:
%  coms - cell array of COM port string such as 'COM1', 'COM5', ...
%  desc - cell array of COM port descriptor strings
%  list - array of structures containting .com and .desc items
%
% (c) Stanislav Maslan, 2025, s.maslan@seznam.cz
% part of project: https://github.com/smaslan/get-com-descriptor
% The script is distributed under MIT license, https://opensource.org/licenses/MIT.

    mfld = fileparts(mfilename('fullpath'));
    tool_path = fullfile(mfld, 'get_com_descriptor.exe');        
    if ~exist(tool_path,'file')
        error('%s tool not found!',tool_path);
    end
    
    [~,list] = system(sprintf('%s -list', tool_path));   
    list = deblank(list);
        
    csv = strsplit(list,{'\t', '\r\n'});
    if isempty(csv)
        list = [];
        coms = {};
        desc = {};
        return;
    end
    csv = reshape(csv,[2 numel(csv)/2]).'; 
    
    coms = csv(:,1);
    desc = csv(:,2);
    list = [];
    for k = 1:numel(coms)
        list(k).com = coms{k};
        list(k).desc = desc{k};
    end
    
end