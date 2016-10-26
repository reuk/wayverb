from scipy import signal
import matplotlib.pyplot as plt
import numpy as np

b=[
    0.492826,
    -0.103056,
    -0.453453,
    0.516234,
    0.238287,
    -0.351316,
    -0.0630573]
a=[
    1,
    0.269101,
    -0.203543,
    1.1341,
    0.733831,
    -0.175853,
    0.0917495]

w, h = signal.freqz(b, a)

fig = plt.figure()
plt.title('response')
ax1 = fig.add_subplot(111)

plt.plot(w, 20 * np.log10(abs(h)), 'b')
plt.ylabel('Amplitude [dB]', color='b')
plt.xlabel('Frequency [rad/sample]')

ax2 = ax1.twinx()
angles = np.unwrap(np.angle(h))
plt.plot(w, angles, 'g')
plt.ylabel('Angle (radians)', color='g')
plt.grid()
plt.axis('tight')
plt.show()
