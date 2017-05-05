from traits.api import Float, HasTraits, HasPrivateTraits, Str, Tuple, File, Button, SingletonHasPrivateTraits
from traitsui.api import Handler, View, Item;
from traitsui.menu import OKButton, CancelButton
#import logging

import threading

#import weakref

class StoppableThread( threading.Thread ):
    """
    A thread that can be stopped.
    
    Parameters:
        target:    callable that will be execute by the thread
        name:      string that will be used as a name for the thread
    
    Methods:
        stop():    stop the thread
        
    Use threading.currentThread().stop_request.isSet()
    or threading.currentThread().stop_request.wait([timeout])
    in your target callable to react to a stop request.
    """
    
    def __init__(self, *arguments, **kwargs):
        self.stop_request = threading.Event()
        threading.Thread.__init__(self, *arguments, **kwargs)
        
    def stop(self, timeout=10.):
        if threading.currentThread() is self:
            return
        elif not self.is_alive():
            return
        self.stop_request.set()
        self.join(timeout)
            
            
#class Job( HasTraits ):

    #"""
    #Defines a job.
    
    #Methods:
    
        #start():        starts the job
        #stop(timeout):  stops the job
        #_run():         actual function that is run in a thread        
    #"""

    #thread = Instance( StoppableThread, factory=StoppableThread )

    #def __init__(self, *args, **kwargs):
        #super(Job, self).__init__(*args, **kwargs)
        #self.start()
    
    #def start(self):
        #"""Start the thread."""
        #if self.thread.is_alive():
            #return
        #self.thread = StoppableThread(target=self.__class__._run, args=(weakref.proxy(self),), name=self.__class__.__name__+timestamp())
        #self.thread.start()
        
    #def stop(self, timeout=None):
        #"""Stop the thread."""
        #self.thread.stop(timeout=timeout)

    #def __del__(self):
        #self.stop()
        
    #def _run(self):
        #"""Method that is run in a thread."""
        #pass


class DialogBox( HasPrivateTraits ):
    """Dialog box for showing a message."""
    message = Str
    
class FileDialogBox( SingletonHasPrivateTraits ):
    """Dialog box for selection of a filename string."""
    filename = File
		
def warning( message='', buttons=[OKButton, CancelButton] ):
    """
    Displays 'message' in a dialog box and returns True or False
    if 'OK' respectively 'Cancel' button pressed.
    """    
    dialog_box = DialogBox( message=message )
    ui = dialog_box.edit_traits(view=View(Item('message', show_label=False, style='readonly'),
                                          buttons=buttons,
                                          width=400, height=150,
                                          title='warning',
                                          kind='modal'
                                          )
                                )
    return ui.result

		
def save_file(title=''):
    """
    Displays a dialog box that lets the user select a file name.
    Returns None if 'Cancel' button is pressed or overwriting
    an existing file is canceled.
    
    The title of the window is set to 'title'.
    """    
    dialog_box = FileDialogBox()

    ui = dialog_box.edit_traits(View(Item('filename'),
                                     buttons = [OKButton, CancelButton],
                                     width=400, height=150,
                                     kind='modal',
                                     title=title
                                     )
                                )
    if ui.result:
        if not os.access(dialog_box.filename, os.F_OK) or warning('File exists. Overwrite?'):
            return dialog_box.filename
        else:
            return

def open_file(title=''):
    """
    Displays a dialog box that lets the user select a file name.
    Returns None if 'Cancel' button is pressed or file
    cannot be opened.
    
    The title of the window is set to 'title'.
    """    
    dialog_box = FileDialogBox()

    ui = dialog_box.edit_traits(View(Item('filename'),
                                     buttons = [OKButton, CancelButton],
                                     width=400, height=150,
                                     kind='modal',
                                     title=title
                                     )
                                )
    if ui.result:
        if os.access(dialog_box.filename, os.F_OK):
            return dialog_box.filename
        else:
            warning('File does not exist.', buttons=[OKButton])
		
