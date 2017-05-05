"""
Counter Time Trace Demo

(c) Helmut Fedder 2013-04-02
"""

import numpy as np

import threading

import weakref

from traits.api import HasTraits, Instance, Float, Range,\
                       Bool, Array, on_trait_change
from traitsui.api import Handler, View, Item, Group, HGroup, VGroup, Tabbed, EnumEditor

from enable.api import ComponentEditor
from chaco.api import ArrayPlotData

from chaco_addons import SavePlot as Plot, SaveTool

import TimeTagger

from utility import StoppableThread

class CountTrace( HasTraits ):

    number_of_points = Range(low=10, high=10000, value=200, desc='Length of Count Trace', label='Number of points')
    seconds_per_point = Range(low=0.001, high=1, value=0.1, desc='Seconds per point [s]', label='Seconds per point [s]')
    refresh_interval = Range(low=0.01, high=1, value=0.1, desc='Refresh interval [s]', label='Refresh interval [s]')

    # trace data
    counts_0 = Array()
    counts_1 = Array()
    counts_2 = Array()
    counts_3 = Array()
    counts_4 = Array()
    counts_5 = Array()
    counts_6 = Array()
    counts_7 = Array()
    time_bins = Array()
    
    enable_0 = Bool(True, label='channel 0', desc='enable channel 0')
    enable_1 = Bool(False, label='channel 1', desc='enable channel 1')
    enable_2 = Bool(False, label='channel 2', desc='enable channel 2')
    enable_3 = Bool(False, label='channel 3', desc='enable channel 3')
    enable_4 = Bool(False, label='channel 4', desc='enable channel 4')
    enable_5 = Bool(False, label='channel 5', desc='enable channel 5')
    enable_6 = Bool(False, label='channel 6', desc='enable channel 6')
    enable_7 = Bool(False, label='channel 7', desc='enable channel 7')
    
    plot = Instance( Plot )
    plot_data = Instance( ArrayPlotData )

    def __init__(self, time_tagger, *args, **kwargs):
        self.time_tagger = time_tagger
        self._create_counter()
        self._create_plot()
        super(CountTrace, self).__init__(*args, **kwargs)
        self.on_trait_change(self._update_time_bins, 'time_bins', dispatch='ui')
        self.on_trait_change(self._update_counts_0, 'counts_0', dispatch='ui')
        self.on_trait_change(self._update_counts_1, 'counts_1', dispatch='ui')
        self.on_trait_change(self._update_counts_2, 'counts_2', dispatch='ui')
        self.on_trait_change(self._update_counts_3, 'counts_3', dispatch='ui')
        self.on_trait_change(self._update_counts_4, 'counts_4', dispatch='ui')
        self.on_trait_change(self._update_counts_5, 'counts_5', dispatch='ui')
        self.on_trait_change(self._update_counts_6, 'counts_6', dispatch='ui')
        self.on_trait_change(self._update_counts_7, 'counts_7', dispatch='ui')
        self._stoppable_thread = StoppableThread(target=CountTrace._run, args=(weakref.proxy(self),))
        self._stoppable_thread.start()

    def _create_counter(self):
        self._counter_0 = TimeTagger.Counter(self.time_tagger, 0, int(self.seconds_per_point*1e12), self.number_of_points) 
        self._counter_1 = TimeTagger.Counter(self.time_tagger, 1, int(self.seconds_per_point*1e12), self.number_of_points) 
        self._counter_2 = TimeTagger.Counter(self.time_tagger, 2, int(self.seconds_per_point*1e12), self.number_of_points) 
        self._counter_3 = TimeTagger.Counter(self.time_tagger, 3, int(self.seconds_per_point*1e12), self.number_of_points) 
        self._counter_4 = TimeTagger.Counter(self.time_tagger, 4, int(self.seconds_per_point*1e12), self.number_of_points) 
        self._counter_5 = TimeTagger.Counter(self.time_tagger, 5, int(self.seconds_per_point*1e12), self.number_of_points) 
        self._counter_6 = TimeTagger.Counter(self.time_tagger, 6, int(self.seconds_per_point*1e12), self.number_of_points) 
        self._counter_7 = TimeTagger.Counter(self.time_tagger, 7, int(self.seconds_per_point*1e12), self.number_of_points)
        
    def _counts_0_default(self):
        return np.zeros((self.number_of_points,))
    def _counts_1_default(self):
        return np.zeros((self.number_of_points,))
    def _counts_2_default(self):
        return np.zeros((self.number_of_points,))
    def _counts_3_default(self):
        return np.zeros((self.number_of_points,))
    def _counts_4_default(self):
        return np.zeros((self.number_of_points,))
    def _counts_5_default(self):
        return np.zeros((self.number_of_points,))
    def _counts_6_default(self):
        return np.zeros((self.number_of_points,))
    def _counts_7_default(self):
        return np.zeros((self.number_of_points,))
    
    def _time_bins_default(self):
        return self.seconds_per_point*np.arange(self.number_of_points)

    def _update_time_bins(self):
        self.plot_data.set_data('t', self.time_bins)

    def _update_counts_0(self):
        self.plot_data.set_data('y0', self.counts_0)
    def _update_counts_1(self):
        self.plot_data.set_data('y1', self.counts_1)
    def _update_counts_2(self):
        self.plot_data.set_data('y2', self.counts_2)
    def _update_counts_3(self):
        self.plot_data.set_data('y3', self.counts_3)
    def _update_counts_4(self):
        self.plot_data.set_data('y4', self.counts_4)
    def _update_counts_5(self):
        self.plot_data.set_data('y5', self.counts_5)
    def _update_counts_6(self):
        self.plot_data.set_data('y6', self.counts_6)
    def _update_counts_7(self):
        self.plot_data.set_data('y7', self.counts_7)
    
    def _number_of_points_changed(self):
        self.counts_0 = self._counts_0_default()
        self.counts_1 = self._counts_1_default()
        self.counts_2 = self._counts_2_default()
        self.counts_3 = self._counts_3_default()
        self.counts_4 = self._counts_4_default()
        self.counts_5 = self._counts_5_default()
        self.counts_6 = self._counts_6_default()
        self.counts_7 = self._counts_7_default()
        self.time_bins = self._time_bins_default()
        self._create_counter()
        
    def _seconds_per_point_changed(self):
        self.time_bins = self._time_bins_default()
        self._create_counter()

    def _plot_data_default(self):
        return ArrayPlotData(t=self.time_bins, y0=self.counts_0, y1=self.counts_1, y2=self.counts_2, y3=self.counts_3, y4=self.counts_4, y5=self.counts_5, y6=self.counts_6, y7=self.counts_7)

    @on_trait_change('enable_0,enable_1,enable_2,enable_3,enable_4,enable_5,enable_6,enable_7')
    def _create_plot(self):
        plot = Plot(self.plot_data, width=100, height=100, resizable='hv', padding_left=96, padding_bottom=32)
        plot.tools.append(SaveTool(plot))        
        plot.index_axis.title = 'time [s]'
        plot.value_axis.title = 'count rate [counts/s]'

        n = 0
        if self.enable_0:
            plot.plot(('t','y0'), type='line', color='blue',  name='channel 0')
            n+=1
        if self.enable_1:
            plot.plot(('t','y1'), type='line', color='red',   name='channel 1')
            n+=1
        if self.enable_2:
            plot.plot(('t','y2'), type='line', color='green', name='channel 2')
            n+=1
        if self.enable_3:
            plot.plot(('t','y3'), type='line', color='black', name='channel 3')
            n+=1
        if self.enable_4:
            plot.plot(('t','y4'), type='line', color='brown',  name='channel 4')
            n+=1
        if self.enable_5:
            plot.plot(('t','y5'), type='line', color='red',   name='channel 5')
            n+=1
        if self.enable_6:
            plot.plot(('t','y6'), type='line', color='green', name='channel 6')
            n+=1
        if self.enable_7:
            plot.plot(('t','y7'), type='line', color='black', name='channel 7')

        plot.legend.align = 'll'
        if n > 1:
            plot.legend.visible = True
        else:
            plot.legend.visible = False
            
        self.plot = plot
    
    def _refresh_data(self):
        self.counts_0 = self._counter_0.getData() / self.seconds_per_point
        self.counts_1 = self._counter_1.getData() / self.seconds_per_point
        self.counts_2 = self._counter_2.getData() / self.seconds_per_point
        self.counts_3 = self._counter_3.getData() / self.seconds_per_point
        self.counts_4 = self._counter_4.getData() / self.seconds_per_point
        self.counts_5 = self._counter_5.getData() / self.seconds_per_point
        self.counts_6 = self._counter_6.getData() / self.seconds_per_point
        self.counts_7 = self._counter_7.getData() / self.seconds_per_point
            
    def _run(self):
        while not threading.current_thread().stop_request.isSet():
            self._refresh_data()
            threading.current_thread().stop_request.wait(self.refresh_interval)

    def __del__(self):
        self._stoppable_thread.stop()

    traits_view = View(HGroup(Item('plot', editor=ComponentEditor(size=(100,100)), show_label=False),
                              VGroup(Item('enable_0'),
                                     Item('enable_1'),
                                     Item('enable_2'),
                                     Item('enable_3'),
                                     Item('enable_4'),
                                     Item('enable_5'),
                                     Item('enable_6'),
                                     Item('enable_7'),
                                     ),
                              ),
                       Item('number_of_points'),
                       Item ('seconds_per_point'),
                       Item ('refresh_interval'),
                       title='Count Trace',
                       width=780,
                       height=520,
                       buttons=[],
                       resizable=True,
                       )


if __name__=='__main__':

    time_tagger=TimeTagger.TimeTagger();
    count_trace = CountTrace(time_tagger)
    count_trace.configure_traits()
    