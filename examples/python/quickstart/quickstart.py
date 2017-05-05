from TimeTagger import createTimeTagger, Counter, Correlation, TimeDifferences, CHANNEL_INVALID
from time import sleep
from pylab import *

# create a backend instance
tagger = createTimeTagger()

# run the autocalibration (this is done silently automatically at startup)
tagger.autoCalibration()

# apply the built in test signal (~0.8 to 0.9 MHz, jitter >> 20 ps) to channels 0 and 1
tagger.setTestsignal(0, True)
tagger.setTestsignal(1, True)

# simple counter on channels 0 and 1 with 1 ms binwidth (1e9 ps) and 2000 successive values
count = Counter(tagger, [0,1], int(1e9), 1000)

# wait until the 1000 values should be filled
# we should see the calibration signal turning on in the time trace
sleep(1.0)

# retrieve the current buffer
data = count.getData()

# plot the result
figure()
plot(count.getIndex()/1e12, data[0]*1e-3, count.getIndex()/1e12, data[1]*1e-3)
xlabel('Time [s]')
ylabel('Countrate [MHz]')
ylim(0.,4.)
show()

# cross correlation between channels 0 and 1
# binwidth=1000 ps, n_bins=3000, thus we sample 3000 ns
# we should see the correlation delta peaks at a bit more than 1000 ns distance
corr = Correlation(tagger, 0, 1, 1000, 3000)
sleep(1)
figure()
plot(corr.getIndex()/1e3, corr.getData())
xlabel('Time [ns]')
show()

# cross correlation between channels 0 and 1
# binwidth=10 ps, n_bins=300, thus we sample 3 ns
# The standard deviation of the peak
# is the root mean square sum of the
# input jitters of channels 0 and 1
corr = Correlation(tagger, 0, 1, 10, 300)
sleep(1)
figure()
plot(corr.getIndex(), corr.getData())
xlabel('Time [ps]')
show()

# disable the internal test signal on chnnels 0 1nd 1
tagger.setTestsignal(0, False)
tagger.setTestsignal(1, False)

# set the trigger levels on channel 0 and 1 to 0.5 V
# (this is the default value at startup)
tagger.setTriggerLevel(0, 0.5)
tagger.setTriggerLevel(1, 0.5)

# A general time difference measurement with 100 histograms.
# Each histogram has 2000 bins and 1 ns binwidth.
# We use the start channel at the same time for stepping
# through the histograms and we omit the sync channel
#
# doc snippet pasted below
#
#  click channel = channel that increments the count in a bin
#  start channel = channel that sets start times relative to which clicks on the click channel are measured
#  next channel = channel that increments the histogram index
#  sync channel = channel that resets the histogram index to zero
#  binwidth = binwidth in ps
#  n bins = number of bins in each histogram
#  n histograms = number of histograms

histogram = TimeDifferences(tagger, 0, 1, 1, CHANNEL_INVALID, 1000, 2000, 100)

