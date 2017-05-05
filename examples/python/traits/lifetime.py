import numpy as np
import threading
import time
import weakref

from traits.api                         import HasTraits, Range, Float, Array, Instance, Enum, on_trait_change, Bool, Button, Int
from traitsui.api                       import View, Item, HGroup, VGroup, EnumEditor
from enable.api                         import ComponentEditor 
from chaco.api                          import ArrayPlotData

from chaco_addons import SavePlot as Plot, SaveTool

import TimeTagger

from utility import StoppableThread

class Lifetime( HasTraits ):
    
    window_size = Range(low=10, high=10**9, value=100, desc='window size [ns]', label='range [ns]', mode='text', auto_set=False, enter_set=True)
    bin_width = Range(low=0.01, high=1000, value=1, desc='bin width [ns]', label='bin width [ns]', mode='text', auto_set=False, enter_set=True)
    channel_apd = Enum(0,1,2,3,4,5,6,7, desc="The APD channel", label="APD channel")
    channel_sync = Enum(0,1,2,3,4,5,6,7, desc="The sync channel", label="sync channel")
    
    counts = Array()
    bins = Array()
    
    start_time = Float(value=0.0)
    run_time = Float(value=0.0, label='run time [s]')
    
    plot = Instance( Plot )
    plot_data = Instance( ArrayPlotData )
    
    refresh_interval = Range(low=0.01, high=1, value=0.1, desc='Refresh interval [s]', label='Refresh interval [s]')

    def __init__(self, time_tagger, *args, **kwargs):
        self.time_tagger = time_tagger
        self._create_pulsed()
        super(Lifetime, self).__init__(*args, **kwargs)
        self._create_plot()
        self._stoppable_thread = StoppableThread(target=self.__class__._run, args=(weakref.proxy(self),))
        self._stoppable_thread.start()        
                
    @on_trait_change('channel_apd,channel_sync,window_size,bin_width')
    def _reset(self):
        self._create_pulsed()
        self.bins = self._bins_default()
        self.counts = self._counts_default()
        self.start_time = 0.0
        self.run_time = 0.0    
        
    def _create_pulsed(self):
        n = int(np.round(self.window_size/self.bin_width))
        self.p1 = TimeTagger.Correlation(self.time_tagger, n, int(np.round(self.bin_width*1000)),1,self.channel_sync,self.channel_apd)
        
    def _refresh_data(self):
        self.counts = self.p1.getData()[0][::-1]
        self.run_time = time.time() - self.start_time
    
    def _run(self):
        while not threading.current_thread().stop_request.isSet():
            self._refresh_data()
            threading.current_thread().stop_request.wait(self.refresh_interval)
        
    def _counts_default(self):
        n = int(np.round(self.window_size/self.bin_width))
        return np.zeros(n)        
        
    def _bins_default(self):
        n = int(np.round(self.window_size/self.bin_width))
        return self.bin_width * np.arange(n)
        
    def _create_plot(self):
        plot_data = ArrayPlotData(t=self.bins, y=self.counts)
        plot = Plot(plot_data, padding=8, padding_left=96, padding_bottom=32)
        #plot = Plot(self.plot_data, width=500, height=500, resizable='hv')
        plot.plot(('t','y'), style='line', color='blue')
        plot.index_axis.title = 'time [ns]'
        plot.value_axis.title = 'counts'
        plot.tools.append(SaveTool(plot))
        self.plot_data = plot_data
        self.plot = plot

    def _bins_changed(self):
        self.plot_data.set_data('t', self.bins)

    def _counts_changed(self):
        self.plot_data.set_data('y', self.counts)

    def __del__(self):
        self._stoppable_thread.stop()
        
    traits_view = View(VGroup(Item('plot', editor=ComponentEditor(size=(100,100)), show_label=False
                                   ),
                              HGroup(Item('window_size'),
                                     Item('bin_width'),
                                     Item('channel_apd'),
                                     Item('channel_sync'),
                                     ),
                              ),
                       title='Lifetime', width=720, height=520, buttons=[], resizable=True
                       )

if __name__ == '__main__':
    time_tagger=TimeTagger.TimeTagger();
    lifetime = Lifetime(time_tagger)
    lifetime.configure_traits()
    