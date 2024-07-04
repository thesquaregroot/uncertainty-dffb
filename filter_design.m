pkg load signal

output_precision(8)

# taken from https://www.dsprelated.com/freebooks/filters/Stability_Revisited.html
# since isstable is not supported in Octave at this time
function [stable] = stabilitycheck(A)
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
  %disp(sprintf('A[%d]=',i)); A(1:i)'
end

function build_elliptic_filter(n,Rp,Rs,Wp,fs,t)
  disp("")
  disp("-------------")
  disp(t)
  disp("-------------")
  # performing both transfer function and zpk filter design
  # to enable use of freqz in Octave (which does not support sos at this time)
  if strcmp(t, "high")
    [b,a] = ellip(n, Rp, Rs, Wp*2/fs, t);
    [z,p,k] = ellip(n, Rp, Rs, Wp*2/fs, t);
  else
    [b,a] = ellip(n, Rp, Rs, Wp*2/fs);
    [z,p,k] = ellip(n, Rp, Rs, Wp*2/fs);
  end
  freqz(b, a, [], fs)
  sos = zp2sos(z,p,k)
  # checking sos factoring (though not really needed given the low order here)
  [bh, ah] = sos2tf(sos);
  errB = norm(bh-b)/norm(b)
  errA = norm(ah-a)/norm(a)
  # check filter stablity (again, mostly included for completeness)
  stable = stabilitycheck(a)
end

fs = 10000
build_elliptic_filter(2, 1, 40, 40, fs, "low")
build_elliptic_filter(2, 1, 40, [50, 100], fs, "band 1")
build_elliptic_filter(2, 1, 40, [100, 200], fs, "band 2")
build_elliptic_filter(2, 1, 40, [200, 400], fs, "band 3")
build_elliptic_filter(2, 1, 40, [400, 800], fs, "band 4")
build_elliptic_filter(2, 1, 40, [800, 1600], fs, "band 5")
build_elliptic_filter(2, 1, 40, [1600, 3200], fs, "band 6")
build_elliptic_filter(2, 1, 40, 3600, fs, "high")
