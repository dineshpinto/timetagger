import datetime
import numpy
import TimeTagger

class TimeTaggerWrapper:
    def __init__(self, serial=''):
        TimeTagger._Tagger.setSerial(serial)
        self.tagger = None
        self.x = 0
        self.y = 0
        self.bin_width = 0
        self.started = 0
    
    def Configure(self, laser, bin_width, sequence_length):
        self.x = laser
        self.y = sequence_length
        self.bin_width = bin_width
        self.started = datetime.datetime.now()
        
    def SetCycles(self, blub):
        pass
    def SetTime(self, blub):
        pass
    def SetDelay(self, blub):
        pass
    
    def GetData(self):
        arraysmall = self.tagger.getData()
        if self.bin_width == 1.25:
            return arraysmall        
        arraybig = numpy.zeros((int(self.y),int(self.x/self.bin_width)))
        for y in range(0,int(self.y)):
            for x in range(0,int(self.x/self.bin_width)):
                arraybig[y][x] = arraysmall[y][int(x*self.bin_width/1.25)]
        return arraybig
        
    def Start(self):
        #(int _length, int _binwidth, int _sequence_length, int _channel, int _shot_trigger=-1, int _sequence_trigger=-1) 
        self.tagger = TimeTagger.Pulsed(int(self.x/1.25), 1, int(self.y), 0, 2, 3)
        print("Serial: ", TimeTagger._Tagger.getSerial())
        self.started = datetime.datetime.now()
    
    def GetSweeps(self):
        return 1.0
    
    def GetState(self):
        timediff = datetime.datetime.now() - self.started
        timediff = timediff.seconds + timediff.days * 86400 + timediff.microseconds / 1000000.0
        return timediff,self.tagger.getCounts()
    
    def Halt(self):
        del self.tagger
    
    def Erase(self):
        self.tagger.clear()
        self.started = datetime.datetime.now()