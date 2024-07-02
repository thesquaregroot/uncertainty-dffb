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

# filter cutoffs determined so that the -20db point of the frequency response
# correlates (roughly) to the nearest cutoff frequency of the adjancent filter(s)
# and also spanning one octave
fs = 40000
build_elliptic_filter(4, 1, 50, 60, fs, "low")
build_elliptic_filter(3, 1, 60, [80, 160], fs, "band 1")
build_elliptic_filter(4, 1, 60, [200, 400], fs, "band 2")
build_elliptic_filter(3, 1, 60, [450, 900], fs, "band 3")
build_elliptic_filter(4, 1, 60, [1000, 2000], fs, "band 4")
build_elliptic_filter(4, 1, 60, [2300, 4600], fs, "band 5")
build_elliptic_filter(4, 1, 60, [5000, 10000], fs, "band 6")
build_elliptic_filter(5, 1, 60, 11000, fs, "high")
