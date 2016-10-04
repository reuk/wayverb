function [sm] = mps(s) 
% [sm] = mps(s) 
% create minimum-phase spectrum sm from complex spectrum s 
   sm = exp( fft( fold( ifft( log( clipdb(s,-100) )))));
