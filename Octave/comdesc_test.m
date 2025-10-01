%----------------------------------------------------------------------------------------------------
% Simple demo for get_com_descriptor.exe tool (see attached executable help).
% It is a simple tool to obtain BusReportedDeviceDesc descriptor string of 
% COM port devices under in Windows. It can be used to identify USB-COM port
% devices by user defined string stored in the USB-COM chip EEPROM.
% Very handy when there are many of such devices connected to PC.
%
% This demo obtains list of COM ports with their descriptors, then gets COM port
% descriptor string, then gets COM port name by descriptor string, 
% and finally opens COM by descriptor string.
%                    
% (c) Stanislav Maslan, 2025, s.maslan@seznam.cz
% part of project: https://github.com/smaslan/get-com-descriptor
% The script is distributed under MIT license, https://opensource.org/licenses/MIT.
%----------------------------------------------------------------------------------------------------
clc;
clear all;
close all;

% RS232 communication package (for GNU Octave)
pkg load instrument-control;

% current path
rpth = fileparts(mfilename('fullpath'));
cd(rpth);


% list available COM ports and their descriptors
[coms, desc, list] = comdesc_get_list()


if isempty(list)
    % no ports found cannot do anything
    warning('No COM ports found!');
    return;
end

% try get descriptor for some COM port
descriptor = comdesc_get_desc(list(1).com)


% try get COM port name for given descriptor string
port_name = comdesc_get_com(list(1).desc)


% try open COM port by descriptor string
com = comdesc_open_com(list(1).desc)

% do stuff...

% close port
clear com;







