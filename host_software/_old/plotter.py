import time
import alsaaudio
import BaseHTTPServer
import cStringIO
import numpy as np
import matplotlib
import usb.core

# list of backends can be found under
# matplotlib.rcsetup.interactive_bk
# matplotlib.rcsetup.non_interactive_bk
# matplotlib.rcsetup.all_backends
matplotlib.use('cairo.png')

import matplotlib.pyplot as plt
import matplotlib.cm as cm

from matplotlib import colors

import threading



fs = 8000. # [Hz]

# mutex lock
lock_waterfall_plot = threading.Lock()
lock_log_plot = threading.Lock()

K = 128
X = np.zeros((256,K))		# t, f
#X = np.random.rand(256,256)


#
#	waterfall image
#

fname_im_waterfall = '/doppler_radio.png'

fig_waterfall = plt.figure()
ax_waterfall = fig_waterfall.add_subplot(111)
im_waterfall = ax_waterfall.imshow(X, cmap=cm.jet, interpolation='nearest')

# create a memory-mapped image file
fimg_waterfall = cStringIO.StringIO()



### activity log

actlog = np.array([0. for _ in range(120)])

fname_im_log = '/doppler_log.png'

fig_actlog = plt.figure()
ax_actlog = fig_actlog.add_subplot(111)
ax_actlog.set_ylim(0, 65535)
#im_actlog = ax_actlog.plot(actlog)[0]
im_actlog = ax_actlog.fill_between(range(len(actlog)), actlog, 0., color='0.8')

# create a memory-mapped image file
fimg_actlog = cStringIO.StringIO()




class WelcomeHandler(BaseHTTPServer.BaseHTTPRequestHandler):
	def do_GET(self):
		print 'entering DO_GET'
		'''if self.path[0:len(fname_im_waterfall)] == fname_im_waterfall:	# ignore GET values
			lock_waterfall_plot.acquire()
			print("Web server: Now serving waterfall image to " + str(self.client_address))
			fimg_waterfall.seek(0)
			self.send_response(200)
			self.send_header('Content-type', 'image/png')
			self.send_header('Cache-control', 'no-store')
			self.end_headers()
			self.wfile.write(fimg_waterfall.read())
			lock_waterfall_plot.release()
			print("Done fetching image. ")
		elif self.path[0:len(fname_im_log)] == fname_im_log:	# ignore GET values
			lock_log_plot.acquire()
			print("Web server: Now serving log image to " + str(self.client_address))
			fimg_actlog.seek(0)
			self.send_response(200)
			self.send_header('Content-type', 'image/png')
			self.send_header('Cache-control', 'no-store')
			self.end_headers()
			self.wfile.write(fimg_actlog.read())
			lock_log_plot.release()
			print("Done fetching image. ")
		else:
			self.send_error(404, 'File not found')'''


		print 'leaving do_GET'


class MyWebServer(threading.Thread):
	def run(self):
		print("Now starting HTTP server.")
		httpserver = BaseHTTPServer.HTTPServer(('', 8080), WelcomeHandler)
		httpserver.serve_forever()


class MyLogger(threading.Thread):
	def run(self):

		global im_actlog

		print '* Opening logger connection...'
		dev = usb.core.find(idVendor=0x0483, idProduct=0x5726)

		if dev is None:
			raise Exception('Could not find device')

		while True:
			print '* Grabbing log value...'
			data = dev.ctrl_transfer(0xC0, 0x02, 0, 0, 2)
			data = data[0] + 256*data[1]
			print '  val = ' + str(data)
			
			actlog[:-1] = actlog[1:]
			actlog[-1] = data

			#plt.xticks(tick_locs, tick_labels, rotation=-30)
			plt.xlabel('time')
			plt.yticks([], [])
			# plot
			#for coll in (ax_actlog.collections):
			ax_actlog.collections.remove(im_actlog)
			im_actlog = ax_actlog.fill_between(range(len(actlog)), actlog, 0., color='0.8')
			#im_actlog.set_ydata(actlog)
			fig_actlog.canvas.draw()
			# save
			lock_log_plot.acquire()
			fimg_actlog.reset()
			fimg_actlog.seek(0)
			fig_actlog.savefig(fimg_actlog)
			lock_log_plot.release()

			time.sleep(1.)


class MySampler(threading.Thread):

	def run(self):
		print '* Opening streaming audio connection...'
		print '  Available cards: ' + str(alsaaudio.cards())
		card = 'hw:1'
		print '  Using: \'' + str(card) + '\''
		#print 'Available mixers: ' + str(alsaaudio.mixers(alsaaudio.cards().index(card)))

		# Open the device in blocking capture mode. The last argument could
		# just as well have been zero for blocking mode. Then we could have
		inp = alsaaudio.PCM(alsaaudio.PCM_CAPTURE, alsaaudio.PCM_NORMAL, card)

		# Set attributes: Mono, 8000 Hz, 16 bit little endian samples
		inp.setchannels(1)
		inp.setrate(8000L)
		inp.setformat(alsaaudio.PCM_FORMAT_S16_LE)
	
		# The period size controls the internal number of frames per period.
		# The significance of this parameter is documented in the ALSA api.
		# For our purposes, it is suficcient to know that reads from the device
		# will return this many frames. Each frame being 2 bytes long.
		# This means that the reads below will return either 320 bytes of data
		# or 0 bytes of data. The latter is possible because we are in nonblocking
		# mode.
		periodsize = 2048L
		inp.setperiodsize(periodsize)

		while True:
			print("Fetching new image (" + str(time.strftime("%H:%M:%S")) + ")")
			#try:
			if True:
				# Read data from device
				xlen, x = inp.read()
				if xlen <= 0:
					raise Exception('Did not receive any data from device!')

				# convert to S16
				_x = np.zeros(xlen)
				for i in range(xlen):
					_x[i] = (ord(x[(2*i)+1])<<8) + ord(x[2*i])
				x = _x

				# Fourier transform
				wind = np.hanning(xlen)
				x = x * wind
				x = np.sqrt(np.abs(np.fft.fft(x)))
				freqs = np.fft.fftfreq(xlen, d=1/fs)
				print 'xmax = ' + str(np.amax(x))
				print 'Xmax = ' + str(np.amax(X))
				x = x[2:K+2]
				freqs = freqs[0:K]
				#print('FFT{x} = '); print(str(x))
				#print('Freqs = ' + str(freqs))
				x = x / np.amax(x)	# normalize to [0..1]
				norm = colors.Normalize(vmin=0., vmax=1.)#2**16-1)
				X[0:X.shape[0]-1,:] = X[1:,:]
				X[X.shape[0]-1,:] = x[:]

				# plot
				tick_locs = np.array(np.linspace(0,K-1,8), dtype=np.int)
				tick_labels = np.array(np.round(freqs[tick_locs], 1))	#, dtype=np.int
				#tick_labels /= 44.	# frequency to km/h
				#tick_labels /= 158.4	# frequency to m/s
				#ax_waterfall.set_xticks(tick_locs, tick_labels, rotation=-30)
				ax_waterfall.set_xlabel('km/h')
				ax_waterfall.set_ylabel('time')
				ax_waterfall.set_yticks([], [])
				# plot
				im_waterfall.set_norm(norm)
				im_waterfall.set_data(X)
				fig_waterfall.canvas.draw()
				# save
				'''print 'MySampler - saving image (waiting for lock...)'
				lock_waterfall_plot.acquire()
				print 'MySampler - saving image (lock acquired)'
				fimg_waterfall.reset()
				fimg_waterfall.seek(0)
				fig_waterfall.savefig(fimg_waterfall)
				lock_waterfall_plot.release()
				print 'MySampler - saving image (lock released)' '''
			#except:
			#	print 'Exception during image rendering!'

			time.sleep(0.1)

		inp.close()

# launch threads

s = MySampler()
s.start()

s = MyLogger()
s.start()

w = MyWebServer()
w.setDaemon(True)
w.start()

w.join()	# wait until thread terminates (i.e. never)
