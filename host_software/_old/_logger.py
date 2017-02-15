#XXX: two separate *processes* for actlog vs audio; each with their own webserver

import matplotlib

# list of backends can be found under
# matplotlib.rcsetup.interactive_bk
# matplotlib.rcsetup.non_interactive_bk
# matplotlib.rcsetup.all_backends
#matplotlib.use('cairo.png')
matplotlib.use('Agg')

import collections
import time
import alsaaudio
import BaseHTTPServer
import cStringIO
import numpy as np
import usb.core

import matplotlib.pyplot as plt
import matplotlib.cm as cm

import matplotlib as mpl
from matplotlib import colors
import matplotlib.backends.backend_agg
import threading


### activity log

actlog_buf_len = 720
actlog_max_time = 12. * 60 * 60 	# [s]
actlog_time = np.linspace(0., actlog_max_time, actlog_buf_len)
max_log_value = 12000

fig_actlog = plt.figure(figsize=(10., 3.))
ax_actlog = fig_actlog.add_axes([.05, .15, .9, .7])
#im_actlog = ax_actlog.fill_between(range(actlog_buf_len), actlog_buf_len * [0], 0., color='0.8')


def smooth(x,window_len=11,window='hanning'):
    """smooth the data using a window with requested size.

    This method is based on the convolution of a scaled window with the signal.
    The signal is prepared by introducing reflected copies of the signal
    (with the window size) in both ends so that transient parts are minimized
    in the begining and end part of the output signal.

    input:
        x: the input signal
        window_len: the dimension of the smoothing window; should be an odd integer
        window: the type of window from 'flat', 'hanning', 'hamming', 'bartlett', 'blackman'
            flat window will produce a moving average smoothing.

    output:
        the smoothed signal

    example:

    t=linspace(-2,2,0.1)
    x=sin(t)+randn(len(t))*0.1
    y=smooth(x)

    see also:

    numpy.hanning, numpy.hamming, numpy.bartlett, numpy.blackman, numpy.convolve
    scipy.signal.lfilter

    TODO: the window parameter could be the window itself if an array instead of a string
    NOTE: length(output) != length(input), to correct this: return y[(window_len/2-1):-(window_len/2)] instead of just y.
    """

    if x.ndim != 1:
        raise ValueError, "smooth only accepts 1 dimension arrays."

    if x.size < window_len:
        raise ValueError, "Input vector needs to be bigger than window size."


    if window_len<3:
        return x


    if not window in ['flat', 'hanning', 'hamming', 'bartlett', 'blackman']:
        raise ValueError, "Window is on of 'flat', 'hanning', 'hamming', 'bartlett', 'blackman'"


    s=np.r_[x[window_len-1:0:-1],x,x[-1:-window_len:-1]]
    #print(len(s))
    if window == 'flat': #moving average
        w=np.ones(window_len,'d')
    else:
        w=eval('np.'+window+'(window_len)')

    y=np.convolve(w/w.sum(),s,mode='valid')
    return y



def rainbow_fill(ax, X, Y, cmap=mpl.colors.LinearSegmentedColormap.from_list( "non_annoying_rainbow", [[0,1,0],[0,1,0]], N=2), interpolate=1000):
	ax.plot(X, Y, lw=1., color=(0, .1, 1.))	# dummy plot for auto axis scaling
	polys = []
	N = len(X)

	if not interpolate is None:
		_X = np.linspace(np.amin(X), np.amax(X), interpolate)
		Y = np.interp(_X, X, Y)
		X = _X

	for n, (x,y) in enumerate(zip(X, Y)):
		c = cmap(y / float(max_log_value))
		if n > 0:
			x1 = (X[n-1] + X[n]) / 2.
			y1 = (Y[n-1] + Y[n]) / 2.
			poly = plt.Polygon([(x1, y1), (X[n], Y[n]), (X[n], 0.), (x1, 0)], color=c)
			ax.add_patch(poly)
			polys.append(poly)
		if n < N - 1:
			x2 = (X[n] + X[n+1]) / 2.
			y2 = (Y[n] + Y[n+1]) / 2.
			poly = plt.Polygon([(X[n], Y[n]), (x2, y2), (x2, 0.), (X[n], 0)], color=c)
			ax.add_patch(poly)
			polys.append(poly)

	ax.set_xlim(np.amin(X), np.amax(X))
	ax.set_ylim(np.amin(Y), np.amax(Y))

	return polys


polys = []
def render(actlog_data):
	global fig_actlog, ax_actlog, im_actlog, polys

	# [ax_actlog.collections.remove(x) for x in polys]
	[poly.remove() for poly in polys]
	polys = rainbow_fill(ax_actlog, actlog_time, actlog_data)
	#im_actlog = ax_actlog.fill_between(np.linspace(actlog_data.size, 0., actlog_data.size), actlog_data, 0., color='0.8')
	#im_actlog.set_ydata(actlog)

	fgcolor = "black"
	bgcolor = "white"

	ax_actlog.set_yticklabels([])
	# ax_actlog.set_axis_bgcolor("black")
	ax_actlog.set_xlabel("Time [h]")
	ax_actlog.grid(True)
	ax_actlog.spines['bottom'].set_color(fgcolor)
	# ax_actlog.spines['top'].set_color(fgcolor)
	ax_actlog.spines['left'].set_color(fgcolor)
	# ax_actlog.spines['right'].set_color(fgcolor)
	ax_actlog.xaxis.label.set_color(fgcolor)
	ax_actlog.tick_params(axis='x', colors=fgcolor)

	ax_actlog.set_xticks(np.linspace(0., actlog_max_time, 12))
	ax_actlog.set_xticklabels(np.array(np.rint(np.linspace(-12., 0., int(np.ceil(actlog_max_time / 60. / 60.)))), dtype=np.int))

	ax_actlog.set_ylim([0., max_log_value])

	fig_actlog.canvas.draw()
	# import pdb;pdb.set_trace()
	fig_actlog.savefig("/tmp/foo.png", facecolor=bgcolor)


actlog = max_log_value * np.abs(np.random.rand(actlog_buf_len))
actlog = smooth(actlog)[:actlog_buf_len]
render(actlog)





