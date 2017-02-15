#!/usr/bin/env python
# -*- coding: utf-8 -*-

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

from matplotlib import colors
import matplotlib.backends.backend_agg
import threading


#
#	web server settings
#

http_server_port = 8232
fname_im_waterfall = '/doppler_radio.png'		# file name for HTTP address


#
#	digitization parameters
#

fs = 8000. 		# sample rate
f_plot = 1. 		# update plot every this often


#
#	 waterfall image
#

audio_buf_len = 2048
waterfall_height = 80

K = 512		# number of points for fft
K_plot = 128	# number of points of fft to plot
assert K_plot <= K
X = np.zeros((waterfall_height, K_plot))		# t, f


#
#	create waterfall image
#

fig_waterfall = plt.figure(figsize=(5., 4.))
ax_waterfall = fig_waterfall.add_subplot(111)
im_waterfall = ax_waterfall.imshow(X, cmap=cm.jet, interpolation='nearest')

cstr_buf_waterfall = cStringIO.StringIO()



class RequestHandler(BaseHTTPServer.BaseHTTPRequestHandler):

	def setup(self):
		BaseHTTPServer.BaseHTTPRequestHandler.setup(self)
		self.request.settimeout(60)


	def _fetch_audio_data(self):
		# XXX grab audio data from the circular (FIFO?) buffer
		global sound_recorder_thread
		x = np.array(sound_recorder_thread.audio_buf)
		return x, x.size
		#return np.random.rand(1024), 1024


	def _render_audio_data(self, x, xlen):
		global fig_waterfall, X

		# x = x / float(2**16-1)	# normalise to [0..1]
		# print 'max(x(t)) = ' + str(np.amax(x))
		# Fourier transform
		wind = np.hanning(xlen)
		x = x * wind

		fig2 = plt.figure()
		ax2 = fig2.add_subplot(111)
		ax2.plot(x)
		fig2.savefig("/tmp/foo_" + str(np.random.rand()) + ".png")
		# plt.close(fig2)



		x = np.sqrt(np.abs(np.fft.fft(x)))
		freqs = np.fft.fftfreq(xlen, d=1/fs)
		# print 'max(fft(x)) = ' + str(np.amax(x))
		x = x[2:K+2]
		freqs = freqs[0:K]
		#print('FFT{x} = '); print(str(x))
		#print('Freqs = ' + str(freqs))
		#if np.amax(x) < 0.5:
		#x = x / np.amax(x)
		# x = x / 10.
		x[x > 1.] = 1.

		X[0:X.shape[0]-1,:] = X[1:,:]
		X[X.shape[0]-1,:] = x[:K_plot]


		#
		#	plot
		#

		tick_locs = np.array(np.linspace(0, K_plot - 1, 8), dtype=np.int)
		tick_labels = np.array(np.round(freqs[tick_locs], 1))	#, dtype=np.int
		tick_labels /= 44.	# frequency to km/h
		#tick_labels /= 158.4	# frequency to m/s
		tick_labels /= K_plot / float(K)
		print("tick_locs = " + str(tick_locs))
		print("tick_labels = " + str(tick_labels))
		ax_waterfall.set_xticks(tick_locs)
		ax_waterfall.set_xticklabels(["{0:.0f}".format(tick_label) for tick_label in tick_labels])
		ax_waterfall.set_xlabel('Speed [km/h]')
		ax_waterfall.set_ylabel('Time [s]')
		# ax_waterfall.set_yticks([], [])

		ytick_locs = np.linspace(0., waterfall_height, 5)[::-1]
		ytick_labels = np.linspace(0., waterfall_height * f_plot, len(ytick_locs))
		ax_waterfall.set_yticks(ytick_locs)
		ax_waterfall.set_yticklabels(["{0:.0f}".format(ytick_label) for ytick_label in ytick_labels])

		# plot
		norm = colors.Normalize(vmin=0., vmax=1.)
		im_waterfall.set_norm(norm)
		im_waterfall.set_data(X)
		fig_waterfall.suptitle("Spectrogram")
		
		fig_waterfall.canvas.draw()

		#canvas = matplotlib.backends.backend_agg.FigureCanvasAgg(fig_waterfall)
		#import pdb;pdb.set_trace()
		#canvas.draw()


	def _save_audio_data(self):
		global fig_waterfall, cstr_buf_waterfall

		cstr_buf_waterfall.reset()
		cstr_buf_waterfall.seek(0)
		fig_waterfall.savefig(cstr_buf_waterfall, format='png')
		cstr_buf_waterfall.seek(0)

		#print 'MySampler - saving image (waiting for lock...)'
		#lock_waterfall_plot.acquire()
		#print 'RequestHandler._save_audio_data(): saving image'# (lock acquired)'
		#canvas = matplotlib.backends.backend_agg.FigureCanvasAgg(fig_waterfall)
		#canvas.print_png(cstr_buf_waterfall)
		
		##cstr_buf_waterfall.reset()
		##cstr_buf_waterfall.seek(0)
		##fig_waterfall.savefig(cstr_buf_waterfall)
		#lock_waterfall_plot.release()
		#print 'MySampler - saving image (lock released)' '''
		#except:
		#	print 'Exception during image rendering!'



	def do_GET(self):
		print 'RequestHandler.do_GET(): now serving HTTP request'
		t0 = time.time() * 1000. # [ms]
		if self.path[0:len(fname_im_waterfall)] == fname_im_waterfall:	# ignore GET values
			print("RequestHandler.do_GET(): request is HTTP GET of waterfall image. Requester = " + str(self.client_address))

			global cstr_buf_waterfall

			#time.sleep(0.1)
			x, xlen = self._fetch_audio_data()
			t1 = time.time() * 1000. # [ms]
			print('RequestHandler.do_GET(): time elapsed after _fetch_audio_data = ' + str(t1 - t0) + ' ms')
			self._render_audio_data(x, xlen)
			t1 = time.time() * 1000. # [ms]
			print('RequestHandler.do_GET(): time elapsed after _render_audio_data = ' + str(t1 - t0) + ' ms')
			self._save_audio_data()
			t1 = time.time() * 1000. # [ms]
			print('RequestHandler.do_GET(): time elapsed after _save_audio_data = ' + str(t1 - t0) + ' ms')

			data = cstr_buf_waterfall.getvalue()

			#lock_waterfall_plot.acquire()
			#cstr_buf_waterfall.seek(0)
			self.send_response(200)
			self.send_header('Content-type', 'image/png')
			self.send_header('Content-length', len(data))
			self.send_header('Cache-control', 'no-store')
			self.end_headers()
			#self.wfile.write(cstr_buf_waterfall.read())
			self.wfile.write(data)
			self.wfile.flush()
			#lock_waterfall_plot.release()
			#print("Done fetching image. ")
		else:
			self.send_error(404, 'File not found')

		t1 = time.time() * 1000. # [ms]
		print('RequestHandler.do_GET(): time elapsed = ' + str(t1 - t0) + ' ms')


class MyWebServer(threading.Thread):
	def run(self):
		print("MyWebServer.run(): Now starting HTTP server.")
		httpserver = BaseHTTPServer.HTTPServer(('', http_server_port), RequestHandler)
		httpserver.serve_forever()

	# def terminate(self):
	# 	inp.close()



class SoundRecorder(threading.Thread):

	inp = None
	audio_buf = None

	def __init__(self, audio_buf_len):
		print('SoundRecorded.__init__(): initialising')

		threading.Thread.__init__(self)

		# init buffer
		self.audio_buf = collections.deque(maxlen=audio_buf_len)
		self.audio_buf.extend(audio_buf_len * [0])

		print '* Opening streaming audio connection...'
		print '  Available cards: ' + str(alsaaudio.cards())
		card = 'hw:2'
		print '  Using: \'' + str(card) + '\''
		#print 'Available mixers: ' + str(alsaaudio.mixers(alsaaudio.cards().index(card)))

		# Open the device in blocking capture mode.
		self.inp = alsaaudio.PCM(alsaaudio.PCM_CAPTURE, alsaaudio.PCM_NONBLOCK, card)

		# Set attributes: Mono, 8000 Hz, 16 bit little endian samples
		self.inp.setchannels(1)
		self.inp.setrate(8000L)
		self.inp.setformat(alsaaudio.PCM_FORMAT_S16_LE)
		
		# The period size controls the internal number of frames per period.
		# The significance of this parameter is documented in the ALSA api.
		periodsize = 2048L
		self.inp.setperiodsize(periodsize)

	def run(self):
		while True:	# infinite loop
			# Read data from device
			xlen, x = self.inp.read()
			if xlen > 0:

				# x = chr(n & 0xFF) + chr((n & 0xFF00) >> 8)

				# convert to S16
				_x = np.zeros(xlen, dtype=np.int)
				for i in range(xlen):
					_x[i] = (ord(x[(2*i)+1])<<8) + ord(x[2*i])
				_x = _x.astype(np.int16) / float(0x7FFF)

				self.audio_buf.extend(_x)

			time.sleep(0.)	# yield to scheduler


#
#		launch threads
#

global sound_recorder_thread
sound_recorder_thread = SoundRecorder(audio_buf_len=audio_buf_len)
sound_recorder_thread.start()

global webserver_thread
webserver_thread = MyWebServer()
#webserver_thread.run()
webserver_thread.setDaemon(True)
webserver_thread.start()

webserver_thread.join()	# wait until thread terminates (i.e. never)

