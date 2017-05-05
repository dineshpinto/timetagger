from TimeTagger import *

from control import TimeTaggerControl

time_tagger = TimeTagger(serial='12520004J3')

control = TimeTaggerControl(time_tagger)

control.configure_traits()

