function [a,b] = eqnerror(M,N,w,D,W,iter);
% [a,b] = eqnerror(M,N,w,D,W,iter);
%
% IIR filter design using equation error method
%
% if the input argument 'iter' is specified and if iter > 1, then
% the equation error method is applied iteratively trying to
% determine the true L2 solution of the nonlinear IIR design problem
%
% M     numerator order
% N     denominator order
% a     denominator coefficients (length N+1), a(1) = 1
% b     numerator coefficients (length M+1)
% w     frequency vector in [0,pi], where pi is Nyquist
% D     desired complex frequency response at frequencies w
% W     weight vector defined at frequencies w
% iter  optional; number of iterations for non-linear solver
%
% author: Mathias C. Lang, mattsdspblog@gmail.com

if nargin == 5,
    iter = 1;
end

if ( isempty(iter) || iter <= 0 ),
    iter = 1;
end

if ( max(w) > pi || min(w) < 0 ),
    error('w must be in [0,pi]');
end

L = length(w);

if ( length(D) != L ),
    error('D and w must have the same lengths.');
end

if ( length(W) != L ),
    error('W and w must have the same lengths.');
end

D0 = D(:);
w = w(:); 
W0 = W(:);

A0 = [-D0(:,ones(N,1)).*exp(-1i*w*(1:N)), exp(-1i*w*(0:M))];

den = ones(L,1);

for k = 1:iter
    W = W0./abs(den);
    A = A0.*W(:,ones(M+N+1,1));
    D = D0.*W;
    x = [real(A);imag(A)] \ [real(D);imag(D)];
    a = [1;x(1:N)]; b = x(N+1:M+N+1);
    den = freqz(a,1,w);
end
