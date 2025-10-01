function [desc] = comdesc_get_desc(com_name)
% Using tool get_com_descriptor.exe obtain user friendly COM port description.
% 
% usage:
%  [desc] = comdesc_get_desc(com_name)
%
% where:
%  com_name - is name of COM port such as 'COM4' 
%  desc - is obtained COM port descriptor if COM found
%
% (c) Stanislav Maslan, 2025, s.maslan@seznam.cz
% part of project: https://github.com/smaslan/get-com-descriptor
% The script is distributed under MIT license, https://opensource.org/licenses/MIT.

    mfld = fileparts(mfilename('fullpath'));
    tool_path = fullfile(mfld, 'get_com_descriptor.exe');        
    if ~exist(tool_path,'file')
        error('%s tool not found!',tool_path);
    end
    
    [~,list] = system(sprintf('%s -name %s', tool_path, com_name));     
    list = deblank(list);
    
    if isempty(list)
        error('COM port ''%s'' not found in system devices!', com_name);
    end
    
    desc = list;
    
end