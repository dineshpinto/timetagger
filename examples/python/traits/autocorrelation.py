import numpy as np
import weakref
import threading

import logging
import time

from enthought.traits.api             import HasTraits, Range, Array, Instance, Enum, Bool, Button, Float
#from enthought.traits.api             import on_trait_change
from enthought.traits.ui.api           import View, Item, HGroup #, EnumEditor
from enthought.enable.api             import ComponentEditor
from enthought.chaco.api              import ArrayPlotData

from chaco_addons import SavePlot as Plot, SaveTool

import TimeTagger

from utility import StoppableThread

class Autocorrelation( HasTraits ):
    
    window_size = Range(low=1., high=1e9, value=2000., desc='window size [ns]', label='range [ns]', mode='text', auto_set=False, enter_set=True)
    bin_width   = Range(low=0.001, high=1000., value=1., desc='bin width [ns]', label='bin width [ns]', mode='text', auto_set=False, enter_set=True)
    chan1 = Enum(0,1,2,3,4,5,6,7, desc="the trigger channel", label="Channel 1")
    chan2 = Enum(0,1,2,3,4,5,6,7, desc="the signal channel", label="Channel 2")

    counts = Array()
    bins = Array()
    
    start_time = Float(value=0.0)
    run_time = Float(value=0.0, label='run time [s]')
    
    plot = Instance( Plot )
    plot_data = Instance( ArrayPlotData )
    
    refresh_interval = Range(low=0.01, high=1, value=0.1, desc='Refresh interval [s]', label='Refresh interval [s]')
    
    def __init__(self, time_tagger, **kwargs):
        self.time_tagger = time_tagger
        self._create_pulsed()
        super(Autocorrelation, self).__init__(**kwargs)
        self._create_plot()
        self._stoppable_thread = StoppableThread(target=Autocorrelation._run, args=(weakref.proxy(self),))
        self._stoppable_thread.start()
        
    # @on_trait_change('chan1,chan2,window_size,bin_width')
    def _reset(self):
        self._create_pulsed()
        self.bins = self._bins_default()
        self.counts = self._counts_default()
        self.start_time = 0.0
        self.run_time = 0.0
        
    def _create_pulsed(self):
        n_bins = int(np.round(self.window_size/self.bin_width))
        bin_width = int(np.round(self.bin_width*1000))
        self.p1 = TimeTagger.Correlation(self.time_tagger, n_bins, bin_width, 1, self.chan1, self.chan2)
        self.p2 = TimeTagger.Correlation(self.time_tagger, n_bins, bin_width, 1, self.chan2, self.chan1)
    
    def _refresh_data(self):
        data1 = self.p1.getData()
        data2 = self.p2.getData()
        self.run_time = time.time() - self.start_time
        self.counts = np.append(np.append(data1[0][-1:0:-1], max(data1[0][0],data2[0][0])),data2[0][1:])
    
    def _run(self):
        while not threading.current_thread().stop_request.isSet():
            self._refresh_data()
            threading.current_thread().stop_request.wait(self.refresh_interval)
        
    def _counts_default(self):
        n = int(np.round(self.window_size/self.bin_width))
        return np.zeros(n*2-1)
    
    def _bins_default(self):
        n = int(np.round(self.window_size/self.bin_width))
        return self.bin_width*np.arange(-n+1,n)
    
    def _bins_changed(self):
        self.plot_data.set_data('t', self.bins)

    def _counts_changed(self):
        self.plot_data.set_data('y', self.counts)
    
    def _chan1_default(self):
        return 0
    
    def _chan2_default(self):
        return 7
        
    def _window_size_changed(self):
        self.counts = self._counts_default()
        self.bins = self._bins_default()
        
    def _create_plot(self):
        data = ArrayPlotData(t=self.bins, y=self.counts)
        plot = Plot(data, width=500, height=500, resizable='hv', padding_left=96, padding_bottom=32)
        plot.plot(('t','y'), type='line', color='blue')
        plot.tools.append(SaveTool(plot))
        plot.index_axis.title = 'time [ns]'
        plot.value_axis.title = 'counts'
        self.plot_data=data
        self.plot=plot
    
    def __del__(self):
        self._stoppable_thread.stop()
        
    traits_view = View(
        Item('plot', editor=ComponentEditor(size=(100,100)), show_label=False),
        HGroup(
            Item('window_size'),
            Item('bin_width'),
            Item('chan1'),
            Item('chan2'),
            ),
        title='Autocorrelation', width=720, height=520, buttons=[], resizable=True)

if __name__ == '__main__':
        
    time_tagger=TimeTagger.TimeTagger();
    autocorrelation = Autocorrelation(time_tagger)
    autocorrelation.configure_traits()
    
