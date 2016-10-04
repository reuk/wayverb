function [rw] = fold(r) 
% [rw] = fold(r) 
% Fold left wing of vector in "FFT buffer format" 
% onto right wing 
% J.O. Smith, 1982-2002
  
[m,n] = size(r);
if m*n ~= m+n-1
 error('fold.m: input must be a vector'); 
end
flipped = 0;
if (m > n)
 n = m;
 r = r.';
 flipped = 1;
end
if n < 3, rw = r; return; 
elseif mod(n,2)==1, 
   nt = (n+1)/2; 
   rw = [ r(1), r(2:nt) + conj(r(n:-1:nt+1)), ...
		 0*ones(1,n-nt) ]; 
else 
   nt = n/2; 
   rf = [r(2:nt+1)]; 
   rf = rf + conj(r(n:-1:nt+1)); 
   rw = [ r(1) , rf , 0*zeros(1,nt-1) ]; 
end; 

if flipped
 rw = rw.';
end
