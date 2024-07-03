pkg load signal

output_precision(8)

function build_elliptic_filter(n,Rp,Rs,Wp,fs,t)
  disp("")
  disp("-------------")
  disp(t)
  disp("-------------")
  if strcmp(t, "high")
    [b,a] = ellip(n, Rp, Rs, Wp*2/fs, t);
  else
    [b,a] = ellip(n, Rp, Rs, Wp*2/fs);
  end
  freqz(b, a, [], fs)
  sos = tf2sos(b, a)
  [bh, ah] = sos2tf(sos);
  errB = norm(bh-b)/norm(b)
  errA = norm(ah-a)/norm(a)
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
