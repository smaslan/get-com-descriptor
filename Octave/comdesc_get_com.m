function [name] = comdesc_get_com(descriptor)
% Using tool get_com_descriptor.exe obtain COM port name using 
% COM port descriptor string.
% 
% usage:
%  [name] = comdesc_get_com(descriptor)
%
% where:
%  descriptor - is COM port descriptor string
%  name - is name of COM port such as 'COM4'
%         if more than one COM matches, returns cell array! 
%
% (c) Stanislav Maslan, 2025, s.maslan@seznam.cz
% part of project: https://github.com/smaslan/get-com-descriptor
% The script is distributed under MIT license, https://opensource.org/licenses/MIT.

    mfld = fileparts(mfilename('fullpath'));
    tool_path = fullfile(mfld, 'get_com_descriptor.exe');        
    if ~exist(tool_path,'file')
        error('%s tool not found!',tool_path);
    end
    
    [~,list] = system(sprintf('%s -desc \"%s\"', tool_path, descriptor));     
    list = deblank(list);
    
    if isempty(list)
        error('COM port having descriptor ''%s'' not found in system devices!', descriptor);
    end
    
    csv = strsplit(list,'\r\n');    
    if numel(csv) > 1
        name = csv;
        return;
    end
    
    name = list;      
    
end