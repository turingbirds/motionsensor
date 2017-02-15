import subprocess

import time

import BaseHTTPServer

import cStringIO

import numpy as np

import matplotlib

# list of backends can be found under
# matplotlib.rcsetup.interactive_bk
# matplotlib.rcsetup.non_interactive_bk
# matplotlib.rcsetup.all_backends
matplotlib.use('cairo.png')

import matplotlib.pyplot as plt
import matplotlib.cm as cm

from matplotlib import colors

import threading


# create a memory-mapped image file
fimg = cStringIO.StringIO()

# mutex lock
lock = threading.Lock()

X = np.zeros((256,128))		# t, f
#X = np.random.rand(256,256)

fig = plt.figure()
ax = fig.add_subplot(111)
im = ax.imshow(X, cmap=cm.jet, interpolation='nearest')

plt.show()



class WelcomeHandler(BaseHTTPServer.BaseHTTPRequestHandler):
	def do_GET(self):
		fname = '/doppler_radio.png'
		if self.path[0:len(fname)] == fname:	# ignore GET values
			lock.acquire()
			print("Web server: Now serving image to " + str(self.client_address))
			fimg.seek(0)
			self.send_response(200)
			self.send_header('Content-type', 'image/png')
			self.send_header('Cache-control', 'no-store')
			self.end_headers()
			self.wfile.write(fimg.read())
			lock.release()
			print("Done fetching image. ")
		else:
			self.send_error(404, 'File not found')


class MyWebServer(threading.Thread):
	def run(self):
		print("Now starting HTTP server.")
		httpserver = BaseHTTPServer.HTTPServer(('', 8080), WelcomeHandler)
		httpserver.serve_forever()



class MySampler(threading.Thread):
	def run(self):
		while True:
			print("Fetching new image (" + str(time.strftime("%H:%M:%S")) + ")")
			try:
				#X = np.random.rand(256,256)
				p = subprocess.Popen(["/home/digiposter/doppler/set-led", "status"],
				                     stdout=subprocess.PIPE)
				s = p.stdout.readlines()[0]
				x = np.fromstring(s, sep=" ", dtype="int")
				x_index = x[256]
				if x_index < 0:
					x_index = 255 - ~x_index	# two's complement 
				x = x[0:256]
				print('x = ' + str(x) + ', index = ' + str(x_index))
				# Hanning window
				wind = np.hanning(x.shape[0])
				x = x * wind
				x = np.sqrt(np.abs(np.fft.fft(x)))
				x = x[0:128]
				print('FFT{x} = '); print(str(x))
				norm = colors.Normalize(vmin=0, vmax=127)
				X[0:X.shape[0]-1,:] = X[1:,:]
				X[X.shape[0]-1,:] = x[:]
				# calculate frequencies
				freqs = np.fft.fftfreq(256, d=1/366.2109375)
				tick_locs = np.array(np.linspace(0,127,8), dtype=np.int)
				tick_labels = np.array(np.round(freqs[tick_locs]), dtype=np.int)
				tick_labels /= 44.	# frequency to km/h
				#tick_labels /= 158.4	# frequency to m/s
				plt.xticks(tick_locs, tick_labels, rotation=-30)
				plt.xlabel('km/h')
				plt.ylabel('time')
				plt.yticks([], [])
				# plot
				im.set_norm(norm)
				im.set_data(X)
				fig.canvas.draw()
				# save
				lock.acquire()
				fimg.reset()
				fimg.seek(0)
				plt.savefig(fimg)
				lock.release()
			except:
				pass

			time.sleep(0.5)


# launch threads

s = MySampler()
s.start()

w = MyWebServer()
w.setDaemon(True)
w.start()

w.join()	# wait until thread terminates (i.e. never)
