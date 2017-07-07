from glob import glob
import pickle
import matplotlib.pyplot as plt

colors = ['red','blue','green']

dirs = glob('Test-4')
for d in dirs:
    data = {}
    files = glob('%s/jitter-*.txt' % (d,))
    for f in files:
        p = pickle.load(file(f))
        data[p['freq']] = p
        
    freqs = list(data.keys())
    freqs.sort()
    
    jitter = [data[freq]['jitter'] for freq in freqs]
    
    plt.plot(freqs,jitter,color=colors.pop(0))
    
plt.ylabel('Jitter [ps]')
plt.xlabel('Frequenz [Hz]')
plt.xscale('log')
plt.yscale('log')
plt.title('Jitter')
#plt.legend(('korrigiert','unkorrigiert'))
#plt.yscale("log")
#plt.show()
#plt.savefig('jitter-%s.png' % (d,))
plt.savefig('jitter.svg')
plt.close()
