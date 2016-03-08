function [stable] = stabilitycheck(A);

N = length(A)-1; % Order of A(z)
stable = 1;      % stable unless shown otherwise
A = A(:);        % make sure it's a column vector
for i=N:-1:1
    rci=A(i+1);
    if abs(rci) >= 1
        stable=0;
        return;
    end
    A = (A(1:i) - rci * A(i+1:-1:2))/(1-rci^2);
end

