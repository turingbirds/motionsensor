#!/usr/bin/env python
# -*- coding: utf-8 -*-

import matplotlib

# list of backends can be found under
# 	matplotlib.rcsetup.interactive_bk
# 	matplotlib.rcsetup.non_interactive_bk
# 	matplotlib.rcsetup.all_backends
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

from matplotlib import colors
import matplotlib.backends.backend_agg
import threading




#
#	web server settings
#

http_server_port = 8080

fname_im_log = '/doppler_log.png'		# file name for HTTP address


#
#	custom USB commands
#

COMMAND_GET_INTEGRATION_WINDOW = 0x01
COMMAND_GET_SIGNAL_LEVEL = 0x02
COMMAND_GET_SEQ_NUM = 0x03
COMMAND_SET_INTEGRATION_WINDOW = 0x04


#
#	activity log buffer size
#

max_log_value = 65536		 # max amplitude of a log entry (2^16)

actlog_buf_len = 50		# 1 log sample per minute

global actlog_wind_size_samples
actlog_wind_size_samples = 8000		# in 
actlog_wind_size_t = actlog_buf_len * 1. 		# update interval


#
#	begin plotting
#

line_plot = None

fig_actlog = plt.figure(figsize=(10., 4.))
ax_actlog = fig_actlog.add_subplot(111)
ax_actlog.set_ylim(0, max_log_value)
ax_actlog.set_yticklabels([])
ax_actlog.set_ylabel("")
im_actlog = ax_actlog.fill_between(range(actlog_buf_len), actlog_buf_len * [0], 0., color='0.8')
polys = []
cstr_buf_actlog = cStringIO.StringIO()		# create a memory-mapped image file


def _send_usb(dev, command, data):
	w = data
	w &= 0x3FFFFFFF
	w |= command << 29
	print("w = " + str(w))

	buf = 4 * [0]
	for i in range(4):
		buf[i] = (w >> (8 * i)) & 0xFF

	print("buf = " + str(buf))

	return dev.ctrl_transfer(0x40, 1, 0, 0, buf)


def _get_usb(dev):
	data = dev.ctrl_transfer(0xC0, 2, 0, 0, 4)
	if not len(data) == 4:
		raise Exception("Error receiving data")
	w = 0
	for i in range(4):
		w |= data[i] << (8 * i)

	return w


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


def rainbow_fill(ax, X, Y, cmap=plt.get_cmap("Greys_r"), interpolate=1000, render_line=True):
	Y = np.flipud(Y)
	if render_line:
		_plot, = ax.plot(X, Y, lw=1.5, color=(.6, .6, .7))	# dummy plot for auto axis scaling
	else:
		_plot = None
	polys = []
	N = len(X)

	if not interpolate is None:
		_X = np.linspace(np.amin(X), np.amax(X), interpolate)
		Y = np.interp(_X, X, Y)
		X = _X

	for n, (x,y) in enumerate(zip(X, Y)):
		c = cmap(.2 + y / float(2.5 * max_log_value))
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

	ax.set_xlim(np.amax(X), np.amin(X))
	ax.set_ylim(np.amin(Y), np.amax(Y))


	return polys, _plot


class RequestHandler(BaseHTTPServer.BaseHTTPRequestHandler):

	polys = []

	def setup(self):
		BaseHTTPServer.BaseHTTPRequestHandler.setup(self)
		self.request.settimeout(60)



		'''	#plt.xticks(tick_locs, tick_labels, rotation=-30)
			plt.xlabel('time')
			plt.yticks([], [])
			# plot
			#for coll in (ax_actlog.collections):
			# save
			#lock_log_plot.acquire()
			cstr_buf_actlog.reset()
			cstr_buf_actlog.seek(0)
			fig_actlog.savefig(cstr_buf_actlog)
			#lock_log_plot.release()

			time.sleep(1.)'''

	def _fetch_actlog_data(self):
		# XXX grab audio data from the circular (FIFO?) buffer
		global logger_thread
		x = np.array(logger_thread.actlog_buf)
		print 'RequestHandler._fetch_actlog_data(): received ' + str(x.size) + ' samples'
		return x


	def _render_actlog_data(self, actlog_data):
		global fig_actlog, ax_actlog, im_actlog, polys, line_plot, actlog_wind_size_t

		# data = self._fetch_actlog_data()
		# print ax_actlog.collections
		[poly.remove() for poly in polys]
		# if not line_plot is None:
			# line_plot.remove()
		polys = []

		if actlog_buf_len > 3660:
			X = np.linspace(0., actlog_wind_size_t / 3600., len(actlog_data))
			unit = "h"
		if actlog_buf_len > 60:
			X = np.linspace(0., actlog_wind_size_t / 60., len(actlog_data))
			unit = "m"
		else:
			X = np.linspace(0., actlog_wind_size_t, len(actlog_data))
			unit = "s"


		polys, _line_plot = rainbow_fill(ax_actlog, X, actlog_data, render_line=line_plot is None)
		if line_plot is None:
			line_plot = _line_plot

		line_plot.set_ydata(np.flipud(actlog_data))




		fgcolor = "black"
		bgcolor = "white"

		ax_actlog.set_yticklabels([])
		# ax_actlog.set_axis_bgcolor("black")
		ax_actlog.set_xlabel("Time [" + unit + "]")
		ax_actlog.grid(True)
		ax_actlog.spines['bottom'].set_color(fgcolor)
		# ax_actlog.spines['top'].set_color(fgcolor)
		ax_actlog.spines['left'].set_color(fgcolor)
		# ax_actlog.spines['right'].set_color(fgcolor)
		ax_actlog.xaxis.label.set_color(fgcolor)
		ax_actlog.tick_params(axis='x', colors=fgcolor)

		# ax_actlog.set_xticks(np.linspace(0., actlog_max_time, 12))
		# ax_actlog.set_xticklabels(np.array(np.rint(np.linspace(-12., 0., int(np.ceil(actlog_max_time / 60. / 60.)))), dtype=np.int))

		ax_actlog.set_ylim([0., max_log_value])

		fig_actlog.canvas.draw()
		fig_actlog.savefig("/tmp/foo.png")

		# print ax_actlog.collections
		# for fuck in ax_actlog.collections:
		# 	print (" fuck =  " + str(fuck) + str(dir(fuck)))
		# 	fuck.set_visible(False)
		# while len(ax_actlog.collections) > 0:
		# 	ax_actlog.collections.remove(ax_actlog.collections[0])
		# for fuck in ax_actlog.collections:
		# 	print (" fuck =  " + str(fuck) + str(dir(fuck)))
		# [ax_actlog.collections.remove(x) for x in polys]
		
		#im_actlog = ax_actlog.fill_between(np.linspace(actlog_data.size, 0., actlog_data.size), actlog_data, 0., color='0.8')
		#im_actlog.set_ydata(actlog)
		fig_actlog.canvas.draw()


	def _save_actlog_data(self):
		global fig_actlog, cstr_buf_actlog

		cstr_buf_actlog.reset()
		cstr_buf_actlog.seek(0)
		fig_actlog.savefig(cstr_buf_actlog, format='png')
		cstr_buf_actlog.seek(0)



	def do_GET(self):
		print 'RequestHandler.do_GET(): now serving HTTP request'
		t0 = time.time() * 1000. # [ms]
		if self.path[0:len(fname_im_log)] == fname_im_log:	# ignore GET values
			print("RequestHandler.do_GET(): request is HTTP GET of actlog image. Requester = " + str(self.client_address))

			global cstr_buf_actlog

			#time.sleep(0.1)
			x = self._fetch_actlog_data()
			t1 = time.time() * 1000. # [ms]
			print('RequestHandler.do_GET(): time elapsed after _fetch_actlog_data = ' + str(t1 - t0) + ' ms')
			self._render_actlog_data(x)
			t1 = time.time() * 1000. # [ms]
			print('RequestHandler.do_GET(): time elapsed after _render_actlog_data = ' + str(t1 - t0) + ' ms')
			self._save_actlog_data()
			t1 = time.time() * 1000. # [ms]
			print('RequestHandler.do_GET(): time elapsed after _save_actlog_data = ' + str(t1 - t0) + ' ms')

			data = cstr_buf_actlog.getvalue()

			#lock_actlog_plot.acquire()
			#cstr_buf_actlog.seek(0)
			self.send_response(200)
			self.send_header('Content-type', 'image/png')
			self.send_header('Content-length', len(data))
			self.send_header('Cache-control', 'no-store')
			self.end_headers()
			#self.wfile.write(cstr_buf_actlog.read())
			self.wfile.write(data)
			self.wfile.flush()
		else:
			self.send_error(404, 'File not found')

		t1 = time.time() * 1000. # [ms]
		print('RequestHandler.do_GET(): time elapsed = ' + str(t1 - t0) + ' ms')


class MyWebServer(threading.Thread):
	def run(self):
		print("MyWebServer.run(): Now starting HTTP server.")
		httpserver = BaseHTTPServer.HTTPServer(('', http_server_port), RequestHandler)
		httpserver.serve_forever()

	#def terminate(self):
	#	inp.close()


class MyLogger(threading.Thread):

	dev = None		# USB device
	actlog_buf = None


	def __init__(self, actlog_buf_len=2048, integration_window_size=actlog_wind_size_samples):
		print('MyLogger.__init__(): initialising')

		threading.Thread.__init__(self)

		print '* Initialising buffer...'
		self.actlog_buf = collections.deque(maxlen=actlog_buf_len)
		self.actlog_buf.extend(actlog_buf_len * [0])

		self._open_device()

		_send_usb(self.dev, COMMAND_SET_INTEGRATION_WINDOW, integration_window_size)
		_send_usb(self.dev, COMMAND_GET_INTEGRATION_WINDOW, 0)
		print '  integration window size = ' + str(_get_usb(self.dev))


	def _open_device(self):
		print '* Opening logger connection...'
		self.dev = usb.core.find(idVendor=0x0483, idProduct=0x5726)

		if self.dev is None:
			raise Exception('Could not find device')


	def run(self):
		while True:
			# try:
			print 'MyLogger.run(): Grabbing log value...'

			# self.dev.ctrl_transfer(0x40, COMMAND_GET_SEQ_NUM, 0, 0, None)
			# data = self.dev.ctrl_transfer(0xC0, 0, 0, 0, 4)
			# print '  seq = ' + str(data)
			# # data = data[0] + 256*data[1]
			_send_usb(self.dev, COMMAND_GET_SEQ_NUM, 0)
			data = _get_usb(self.dev)
			print "  seq = " + str(data)

			_send_usb(self.dev, COMMAND_GET_SIGNAL_LEVEL, 0)
			data = _get_usb(self.dev)

			print '  val = ' + str(data)
			self.actlog_buf.append(data)
			print(self.actlog_buf)

			time.sleep(.9)

#
# launch threads
#

global logger_thread
logger_thread = MyLogger(actlog_buf_len=actlog_buf_len)
logger_thread.start()

global webserver_thread
webserver_thread = MyWebServer()
#webserver_thread.run()
webserver_thread.setDaemon(True)
webserver_thread.start()

webserver_thread.join()	# wait until thread terminates (i.e. never)
