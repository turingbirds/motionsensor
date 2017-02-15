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

X = np.zeros((256,256))
#X = np.random.rand(256,256)

fig = plt.figure()
ax = fig.add_subplot(111)
#im = ax.imshow(X, cmap=cm.jet, interpolation='nearest')
im = ax.plot(X, 'o-')

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
				p = subprocess.Popen(["/home/digiposter/doppler/set-led", "status"], stdout=subprocess.PIPE)
				s = p.stdout.readlines()[0]
				x = np.fromstring(s, sep=" ", dtype="int")
				print('x = ' + str(x))
				x = np.sqrt(np.abs(np.fft.fft(x)))
				X[0:X.shape[1]-1,:] = X[1:,:]
				X[X.shape[1]-1,:] = x[:]
				print('FFT{x} = '); print(str(X[X.shape[1]-1,:]))
				#norm = colors.Normalize(vmin=-128, vmax=127)
				#im.set_norm(norm)
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
