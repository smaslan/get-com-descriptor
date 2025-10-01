function [com] = comdesc_open_com(descriptor)
% Using tool get_com_descriptor.exe obtain COM port name using 
% COM port descriptor string and open COM port.
% 
% usage:
%  [com] = comdesc_open_com(descriptor)
%
% where:
%  descriptor - is COM port descriptor string
%  com - COM port object opened by serialport() 
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
        descriptor
        error('COM port descriptor ''%s'' matches more than one COM port! Cannot open COM.', descriptor);
        desc = csv;
        return;
    end
    
    com = serialport(list);       
    
end