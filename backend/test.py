import TimeTagger
import time
import matplotlib.pyplot as plt
import math
import numpy
import pickle
import sys
#o = TimeTagger.OverFlow()
#set print thread-events 0
"""
d = [TimeTagger.Distribution(i) for i in range(16)]
h = [TimeTagger.Histogram(i) for i in range(16)]

time.sleep(1)


for i in range(16):
	d[i].clear()
	h[i].clear()
	
	
time.sleep(5)

for i in range(16):
	data = d[i].getData()
	if sum(data):
		data = filter(lambda x: x>0, data)
		data = map(lambda x:6000*float(x)/sum(data), data)

		y = sum(data) / len(data)
		o = math.sqrt(sum(map(lambda x: (x-y)**2, data)) / len(data))
		data = numpy.array(map(int, data))

		print "Channel: %d" % (i,)
		print d[i].getData()
		print data
		print "Mittelwert: %s ps" % (y,)
		print "Standartabweichung: %s ps" % (o, )
		print "Maximale Fenstergroesse: %s ps" % (max(data),)

		print h[i].getData()
"""

sleeptime = 10
directory = 'Test-4'
print('which channel?')
channel = int(sys.stdin.readline())


c = TimeTagger.Counter(channel,1000**4)
o = TimeTagger.OverFlow()

while True:
	print('waiting, please press enter')
	try:
		sleeptime = int(sys.stdin.readline())
		if sleeptime == 0:
			break
		print('sleeptime set to %s s' % (sleeptime, ))
	except:
		pass
	print('starting')
	
	h = TimeTagger.Histogram(channel,channel^8,10)
	time.sleep(2)
	h.clear()
	time.sleep(sleeptime)
	d = h.getData()
	
	d2 = {}
	for t,n in d:
		d2[t] = n
	d = d2

	for i in range(list(d.keys())[0], list(d.keys())[-1], 10):
		if not i in d:
			d[i] = 0
			
	times = list(d.keys())
	times.sort()
	
	counts = [d[i] for i in times]

	s = sum(counts)
	print('Counts: %d' % (s,))

	m = sum([i[0]*i[1] for i in d.items()])/float(s)
	print("Mittelwert: %s ps" % (m,))

	freq = 1e12 / m
	print("Frequenz: %s Hz" % (freq,))

	o = math.sqrt(sum([x[1]*(x[0]-m)**2 for x in iter(d.items())]) / s)
	print("Standartabweichung: %s ps" % (o, ))	

	print("effektive Abtastrate: %s ps" % (o*math.sqrt(6),))
	
	verteilung1 = TimeTagger._Tagger.getDistributionCount(channel)
	verteilung2 = TimeTagger._Tagger.getDistributionCount(channel^8)
	o_erwartet = math.sqrt(6000.0**2/12 * sum([i**3 for i in list(verteilung1) + list(verteilung2)]))
	
	print("erwartete Standartabweichung: %s ps" % (o_erwartet,))
	

	
	f = file('%s/jitter-%d.txt' % (directory,freq),'a')
	s = {'freq':   freq,
		'jitter': o,
		'nativ':  d,
		'd1' : verteilung1,
		'd2' : verteilung2}
	s = pickle.dumps(s)
	f.write(s)
	f.close()

	plt.plot(times, [i/10.0/sum(counts) for i in counts])
	
	index1 = [6000*max(verteilung1)*i/500.0/sum(verteilung1) + m for i in range(-1000,1000)]
	value1 = [sum([i*3000>abs(k-m) for i in verteilung1])/6000.0 for k in index1]
	#plt.plot(index1, value1)
	
	#index2 = [6000*max(verteilung1)*i/500.0/sum(verteilung1) + m for i in range(-1000,1000)]
	value2 = [sum([i*3000>abs(k-m) for i in verteilung2])/6000.0 for k in index1]
	#plt.plot(index1, value2)
	
	index3 = [6000*max(verteilung1)*i/500.0/sum(verteilung1) + m for i in range(-2000+1,2000)]
	plt.plot(index3, numpy.convolve(value1, value2)*(index1[1]-index1[0]))
	
	plt.legend(('gemessen','Abtastung'))
	plt.ylabel('Verteilung')
	plt.xlabel('Zeit [ps]')
	plt.yscale("log")
	plt.savefig('%s/jitter-%d.svg' % (directory,freq))
	plt.close()
