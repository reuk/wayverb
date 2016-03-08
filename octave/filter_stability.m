%A = [1; rand(N,1)-0.5]';
% see https://ccrma.stanford.edu/~jos/filters/Testing_Filter_Stability_Matlab.html
%A = [  1.          -5.76757344  13.85884998 -17.75869355  12.79874003 -4.91893183   0.78760881];
A = [ 1. 0. 0. 0. ];
stable = stabilitycheck(A);

poles = roots(A);
pr = abs(poles);
unst = (pr >= 1);
nunst = sum(unst);
if (stable & nunst>0) | (~stable & nunst==0)
    error('*** stabilitycheck() and poles DISAGREE ***');
end
