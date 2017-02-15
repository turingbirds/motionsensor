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

from matplotlib import colors
import matplotlib.backends.backend_agg
import threading




### waterfall image

audio_buf_len = 1024

K = 128		# number of points for fft
X = np.zeros((256,K))		# t, f

# create a memory-mapped image file
cstr_buf_waterfall = cStringIO.StringIO()

# file name for HTTP address
fname_im_waterfall = '/doppler_radio.png'

# matplotlib figure
fig_waterfall = plt.figure()
ax_waterfall = fig_waterfall.add_subplot(111)#fig_waterfall.add_axes([.1, .1, .8, .8])
im_waterfall = ax_waterfall.imshow(X, cmap=cm.jet, interpolation='nearest')





### activity log

actlog_buf_len = 120

# file name for HTTP address
fname_im_log = '/doppler_log.png'

fig_actlog = plt.figure(figsize=(10., 3.))
ax_actlog = fig_actlog.add_subplot(111)
ax_actlog.set_ylim(0, 65535)
ax_actlog.set_yticklabels([])
#im_actlog = ax_actlog.plot(actlog)[0]
im_actlog = ax_actlog.fill_between(range(actlog_buf_len), actlog_buf_len * [0], 0., color='0.8')

# create a memory-mapped image file
cstr_buf_actlog = cStringIO.StringIO()


def _twos_comp(val, bits):
	if (val & (1 << (bits - 1))) != 0: # if sign bit is set e.g., 8bit: 128-255
		val = val - (1 << bits)        # compute negative value
	return val    

def twos_comp(val, bits):
	"""compute the 2's complement of int value val"""
	if type(val) is np.ndarray:
		return [_twos_comp(val[i], bits) for i in xrange(len(val))]
	else:
		return _twos_comp(val, bits)

class RequestHandler(BaseHTTPServer.BaseHTTPRequestHandler):

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
		global fig_actlog, ax_actlog, im_actlog

		ax_actlog.collections.remove(im_actlog)
		im_actlog = ax_actlog.fill_between(np.linspace(actlog_data.size, 0., actlog_data.size), actlog_data, 0., color='0.8')
		#im_actlog.set_ydata(actlog)
		fig_actlog.canvas.draw()

	def _save_actlog_data(self):
		global fig_actlog, cstr_buf_actlog

		cstr_buf_actlog.reset()
		cstr_buf_actlog.seek(0)
		fig_actlog.savefig(cstr_buf_actlog, format='png')
		cstr_buf_actlog.seek(0)


	def _fetch_audio_data(self):
		# XXX grab audio data from the circular (FIFO?) buffer
		global sound_recorder_thread
		x = np.array(sound_recorder_thread.audio_buf)
		x = x.astype(np.int16)
		np.savetxt("/tmp/x.txt", x)
		return x, x.size
		#return np.random.rand(1024), 1024

	def _render_audio_data(self, x, xlen):
		global fig_waterfall, X

		if True:
			if True:
				x = x / float(2**16-1)	# normalise to [0..1]
				fig, ax = plt.subplots()
				ax.plot(x)
				fig.savefig("/tmp/foo.png")
				print 'max(x(t)) = ' + str(np.amax(x))
				# Fourier transform
				wind = np.hanning(xlen)
				x = x * wind
				x = np.sqrt(np.abs(np.fft.fft(x)))
				freqs = np.fft.fftfreq(xlen, d=1/8000.)
				print 'max(fft(x)) = ' + str(np.amax(x))
				x = x[2:K+2]
				freqs = freqs[0:K]
				#print('FFT{x} = '); print(str(x))
				#print('Freqs = ' + str(freqs))
				#if np.amax(x) < 0.5:
				#x = x / np.amax(x)
				x = x / 10.
				x[x > 1.] = 1.

				#x = x**3
				#x[x < 0.1*np.amax(x)] = 0.

				X[0:X.shape[0]-1,:] = X[1:,:]
				X[X.shape[0]-1,:] = x[:]

				# plot
				tick_locs = np.array(np.linspace(0,K-1,8), dtype=np.int)
				tick_labels = np.array(np.round(freqs[tick_locs], 1))	#, dtype=np.int
				tick_labels /= 44.	# frequency to km/h
				#tick_labels /= 158.4	# frequency to m/s
				#ax_waterfall.set_xticks(tick_locs, tick_labels), rotation=-30)
				ax_waterfall.set_xlabel('km/h')
				ax_waterfall.set_ylabel('time')
				ax_waterfall.set_yticks([], [])
				# plot
				norm = colors.Normalize(vmin=0., vmax=1.)
				im_waterfall.set_norm(norm)
				im_waterfall.set_data(X)
				
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
			'''elif self.path[0:len(fname_im_log)] == fname_im_log:	# ignore GET values
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
			self.wfile.flush()'''
		else:
			self.send_error(404, 'File not found')

		t1 = time.time() * 1000. # [ms]
		print('RequestHandler.do_GET(): time elapsed = ' + str(t1 - t0) + ' ms')


class MyWebServer(threading.Thread):
	def run(self):
		print("MyWebServer.run(): Now starting HTTP server.")
		httpserver = BaseHTTPServer.HTTPServer(('', 8092), RequestHandler)
		httpserver.serve_forever()

	#def terminate(self):
	#	inp.close()


class MyLogger(threading.Thread):

	dev = None		# USB device
	actlog_buf = None

	def __init__(self, actlog_buf_len=2048):
		print('MyLogger.__init__(): initialising')

		threading.Thread.__init__(self)

		print '* Initialising buffer...'
		self.actlog_buf = collections.deque(maxlen=actlog_buf_len)
		self.actlog_buf.extend(actlog_buf_len * [0])

		print '* Opening logger connection...'
		self.dev = usb.core.find(idVendor=0x0483, idProduct=0x5726)

		if self.dev is None:
			raise Exception('Could not find device')


	def run(self):

		while True:
			print 'MyLogger.run(): Grabbing log value...'
			data = self.dev.ctrl_transfer(0xC0, 0x02, 0, 0, 2)
			data = data[0] + 256*data[1]
			print '  val = ' + str(data)
			
			self.actlog_buf.append(data)
			#actlog[:-1] = actlog[1:]
			#actlog[-1] = data
			
			time.sleep(1.)



class SoundRecorder(threading.Thread):

	inp = None
	audio_buf = None

	def __init__(self, audio_buf_len=2048):
		print('SoundRecorded.__init__(): initialising')

		threading.Thread.__init__(self)

		# init buffer
		self.audio_buf = collections.deque(maxlen=audio_buf_len)
		self.audio_buf.extend(audio_buf_len * [0])

		print '* Opening streaming audio connection...'
		print '  Available cards: ' + str(alsaaudio.cards())
		card = 'hw:3'
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
				#print 'SoundRecorder.run(): received ' + str(xlen) + ' samples'

				# convert to S16
				_x = np.zeros(xlen)
				for i in range(xlen):
					_x[i] = (ord(x[(2*i)+1])<<8) + ord(x[2*i])
				x = _x

				self.audio_buf.extend(x)

				# buffer.write(x)
				'''l = audio_buf_idx
				r = audio_buf_idx + xlen
				_wrap = False
				if r > audio_buf_len:
					_wrap = True
					r = audio_buf_len
				audio_buf[l:r] = x[:r - l]
				if _wrap:
					audio_buf[0:r - audio_buf_len] = 

				audio_buf_idx = (audio_buf_idx + xlen) % audio_buf_len'''

			time.sleep(0)	# yield to scheduler


# launch threads

global sound_recorder_thread
sound_recorder_thread = SoundRecorder(audio_buf_len=audio_buf_len)
sound_recorder_thread.start()

global logger_thread
logger_thread = MyLogger(actlog_buf_len=actlog_buf_len)
#logger_thread.start()

global webserver_thread
webserver_thread = MyWebServer()
#webserver_thread.run()
webserver_thread.setDaemon(True)
webserver_thread.start()

webserver_thread.join()	# wait until thread terminates (i.e. never)
