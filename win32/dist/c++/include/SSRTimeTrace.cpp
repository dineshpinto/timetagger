#include <fstream>
#include <iostream>
#include <list>
#include <thread>

#include "TimeTagger.h"


class ODMR: public _Iterator {
public:
    /**
    *   \brief constructor of ODMR
    *
    *   @param tagger                  reference to a TimeTagger
    *   @param _click_channel_apd1     channel that increments counts in a memory
    *   @param _click_channel_apd2     channel that increments counts in a memory
    *   @param _freqswap_channel       channel that switches between memories to account for different MW pulses (for example at laser pulse)
    *   @param _n_odmr_samples         number of freqency points which are sampled
    *
    **/
    ODMR(TimeTagger* tagger, channel_t _click_channel_apd1, channel_t _click_channel_apd2, channel_t _odmr_trigger,channel_t _freqswap_trigger, int _n_odmr_samples) : _Iterator(tagger) {

        click_channel_apd1 = _click_channel_apd1;
        click_channel_apd2 = _click_channel_apd2;
        odmr_trigger = _odmr_trigger;
        freqswap_trigger = _freqswap_trigger;

        n_odmr_samples = _n_odmr_samples;

        current_sample = -1;
        current_odmr_run = 0;
        odmr_enable = -1;

        registerChannel(click_channel_apd1);
        registerChannel(click_channel_apd2);
        registerChannel(odmr_trigger);
        registerChannel(freqswap_trigger);

        data = new int[n_odmr_samples];

        clear();

        start();
    }

    virtual ~ODMR() {
        stop();
        delete [] data;
    }

    //TimeDifferences method to get data
    GET_DATA_2D(getData, int, array_out) {
        if(!data) return;
        int *arr = array_out(1, n_odmr_samples);
        if(!arr)  return;
        lock();

        for ( int i=0; i<n_odmr_samples; i++ ) {
            arr[i] = data[i];
        }

        unlock();
    }

    GET_DATA_1D(getIndex, timestamp_t, array_out) {
        timestamp_t* arr = array_out(n_odmr_samples);

        for ( int i=0; i<n_odmr_samples; i++ ) {
            arr[i] = current_sample;
        }
    }

    //Clear the data array that is stored, as well as the defined variables in the class
    virtual void clear() {
        lock();

        for( int i; i < n_odmr_samples; i++ ) {
            data[i] = 0;
        }

        current_sample = -1;
        current_odmr_run = 0;
        odmr_enable = -1;

        unlock();
    }

    int getSample() {
        return current_sample;
    }

    int getNSamples() {
        return n_odmr_samples;
    }

    int getODMRRun() {
        return current_odmr_run;
    }

protected:
    virtual void next(Tag* list, int count, timestamp_t time) {
        //if(!data) return;
        for( int i = 0; i < count; i++ ) {
            Tag t = list[i];
            if(t.overflow) {
                signal_overflow();
            } else {
		if(t.chan == odmr_trigger) {
                    current_sample = -1;
		}
                if(t.chan == freqswap_trigger) {
                    current_sample += 1;
                    if(current_sample == n_odmr_samples) {
                        current_sample = 0;
                        current_odmr_run += 1;
                    }
                }
                if(t.chan == click_channel_apd1 || t.chan == click_channel_apd2) {
                    if(current_sample == -1) {
                        continue;
                    } else {
                        data[current_sample]++;
                    }
                }
            }
        }
    }

private:

    channel_t click_channel_apd1;
    channel_t click_channel_apd2;
    channel_t odmr_trigger;
    channel_t freqswap_trigger;

    int n_odmr_samples;
    int odmr_enable;

    int current_odmr_run;
    int current_sample;

    int *data;

};

class SSRTimeTrace: public _Iterator {
public:
    /**
    *	\brief constructor of SSRTimeTrace
    *
    *   @param tagger                  reference to a TimeTagger
    *   @param _click_channel_apd1     channel that increments counts in a memory
    *   @param _click_channel_apd2     channel that increments counts in a memory
    *   @param _ssr_trigger            channel that starts a SSR measurement
    *   @param _freqswap_channel       channel that switches between memories to account for different MW pulses (for example at laser pulse)
    *   @param _n_ssr                  number of SSR measurements
    *   @param _n_memories             number of memories, to account for different MW pulses, i.e.
    *
    **/
    SSRTimeTrace(TimeTagger* tagger, channel_t _click_channel_apd1, channel_t _click_channel_apd2, channel_t _ssr_trigger, channel_t _freqswap_trigger, int _n_ssr, int _n_memories) : _Iterator(tagger) {

        click_channel_apd1 = _click_channel_apd1;
        click_channel_apd2 = _click_channel_apd2;
        ssr_trigger = _ssr_trigger;
        freqswap_trigger = _freqswap_trigger;

        n_ssr = _n_ssr;
        n_memories = _n_memories;

        current_memory = -1;
        current_ssr = -1;

        registerChannel(click_channel_apd1);
        registerChannel(click_channel_apd2);
        registerChannel(ssr_trigger);
        registerChannel(freqswap_trigger);

        data = new int[n_ssr*n_memories];

        clear();

        start();

    }

    virtual ~SSRTimeTrace() {
        stop();
		if(data)
			delete [] data;
    }

    /*
    //Necessary? Used in TimeDifferences.
    virtual void start() {
		lock();
		for( int i; i < n_ssr*n_memories; i++ ) {
            data[i] = 0;
        }
		current_memory = -1;
		current_ssr = -1;
		unlock();
		_Iterator::start();
	}
    */

    //TimeDifferences method to get data
    GET_DATA_2D(getData, int, array_out) {
		if(!data) return;
		int *arr = array_out(n_ssr, n_memories);
		if(!arr)
			return;

		lock();

		for ( int i=0; i<n_ssr*n_memories; i++ ) {
			arr[i] = data[i];
		}

		unlock();
	}

	GET_DATA_1D(getIndex, timestamp_t, array_out) {
		timestamp_t* arr = array_out(n_ssr);

		for ( int i=0; i<n_ssr; i++ ) {
			arr[i] = n_ssr;
		}
	}

    //Clear the data array that is stored, as well as the defined variables in the class
    virtual void clear() {
        lock();

        for( int i; i < n_ssr*n_memories; i++ ) {
            data[i] = 0;
        }

        current_memory = -1;
        current_ssr = -1;

        unlock();
    }

    int getMemory() {
		lock();
        return current_memory;
		unlock();
    }

    int getNMemory() {
		lock();
        return n_memories;
		unlock();
    }

    int getNSSR() {
		lock();
        return n_ssr;
		unlock();
    }

    int getSSR() {
		lock();
        return current_ssr;
		unlock();
    }

protected:
    virtual void next(Tag* list, int count, timestamp_t time) {
        //if(!data) return;
        for( int i = 0; i < count; i++ ) {
    	    Tag t = list[i];
			if (current_ssr < n_ssr) {
    		    if(t.chan == ssr_trigger) {
                    current_ssr += 1;
                    current_memory = -1;
                }
                if(t.chan == freqswap_trigger) {
                    if(current_ssr==-1) {
                        continue;
                    }
                    else {
           	       	    current_memory += 1;
           	       	    if(current_memory == n_memories) {
           	        	   current_memory = 0;
           	            }
                    }
            	}
                if(t.chan == click_channel_apd1 || t.chan == click_channel_apd2) {
                    if(current_ssr==-1) {
                        continue;
                    } else {
                        data[n_memories*current_ssr + current_memory]++;
                    }
           	    }
    	    }
    	    else {
    		    continue;
    	    }
        }
    }

private:

    channel_t click_channel_apd1;
    channel_t click_channel_apd2;
    channel_t ssr_trigger;
    channel_t freqswap_trigger;

    int n_ssr;
    int n_memories;

    int current_ssr;
    int current_memory;

    int *data;
};


// class PhotonRate : public _Iterator {
// public:
//     /**
//     *   \brief      construct a countrate for the photons
//     *   
//     *   \param      tagger          reference to TimeTagger
//     *   \param      _channels       channels on which the rate should be monitored
//     *   \param      _binwidth       time until the rate is updated
//     *
//     **/ 
//     PhotonRate( TimeTagger* tagger, std::vector<channel_t> _channels, timestamp_t _binwidth ) : _Iterator(tagger), channels(_channels), tags(_channels.size()) {
//         binwidth = _binwidth;

//         for(channel_t c=0; c<channels.size(); c++)
//             registerChannel(channels[c]);

//         clear();
//         start();

//     }

//     /*virtual ~PhotonRate() {
//         stop();
//     }
//     */

//     GET_DATA_1D(getData, double, array_out) {
//         double *arr = array_out(channels.size());

//         lock();
//         for (size_t c=0; c<channels.size(); c++)
//         if(now==start) continue;
//             else arr[c] = (1.0e12)*double(binwidth)/double(now-start);
//         unlock();
//     }

//     virtual void clear() {
//         lock();
//         for (size_t c=0; c<channels.size(); c++)
//             tags[c] = 0;
//         unlock();
//     }

// protected:
//     virtual void next(Tag* list, int count, timestamp_t time) {
//         for(int i=0; i<count; i++) {
//             Tag t = list[i];
//             if(t.overflow) {
//                 start = -1;
//                 for (size_t c=0; c<channels.size(); c++)
//                     tags[c] = 0;
//                 signal_overflow();
//             } else {
//                 for(size_t c=0; c<channels.size(); c++) {
//                     if(t.chan == channels[c]) {
//                         if(start==-1) start = time;
//                         else {
//                 tags[c]++;
//                             if(tags[c]==binwidth) {
//                     now = time;
//                     zero();
//                 }
//             }
//                     }
//                 }
//             }
//         }
//     }

// private:
//     void zero() {
//         start = -1;
//         for(size_t c=0; c<channels.size(); c++)
//                 tags[c] = 0;
//     }

//     const std::vector<channel_t> channels;
    
//     std::vector<timestamp_t> tags;
//     timestamp_t increment;
//     timestamp_t time;
//     timestamp_t start;
//     timestamp_t binwidth;
// };

/*
MODIFIED CODE BEGINS HERE

#include <iostream>
#include <algorithm>
#include <vector>


int main(void){
	const int n_ssr = 10;
	std::vector<int> rpt_unit = {1, 2, 3, 2};


	int rpt_unit_size = *std::max_element(rpt_unit.begin(), rpt_unit.end());
	std::cout << rpt_unit_size << std::endl; 
	std::vector<std::vector<int>> arr(n_ssr, std::vector<int>(rpt_unit_size, -1)); 

	for(int i=0;i<n_ssr;i++){
		for(int j=0;j<rpt_unit_size;j++){
			std::cout << arr[i][j] << ' '; 
		}
		std::cout << std::endl;
	}

	int counter = 0;

	// std::cout << rpt_unit.back() << std::endl;

	for(int i=0;i<n_ssr;i++){
		for(int j=0;j<rpt_unit[counter];j++){
			arr[i][j] = 1;
		}
		counter += 1;
		if (counter == rpt_unit.back()){
			counter = 0;
		}
	}

	for(int i=0;i<n_ssr;i++){
		for(int j=0;j<rpt_unit_size;j++){
			std::cout << arr[i][j] << ' '; 
		}
		std::cout << std::endl;
	}

}
*/