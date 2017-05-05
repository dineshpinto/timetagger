/*
 * DiagramData.cpp
 *
 *  Created on: 16.06.2013
 *      Author: mag
 */

#include "DiagramData.h"

#include <math.h>
#include <stdlib.h>
#include <iostream>
#include <string.h>

#ifndef DIAGRAMGDATA_IMPL_
#define DIAGRAMGDATA_IMPL_

template<typename DTYPE>
void DiagramData::downsample(std::vector< std::vector<DTYPE> > &data, int threshold, DiagramData::DOWNSAMPLING_ALGO alghorithm) {
	for (size_t n=0; n<data.size(); ++n) {
		downsample(data[n], threshold, alghorithm);
	}
}

template<typename DTYPE>
void DiagramData::downsample(std::vector<DTYPE> &data, int threshold, DiagramData::DOWNSAMPLING_ALGO alghorithm) {
	int sz=data.size();
	if (sz>threshold) {
		DTYPE *new_buf=downsample(data.data(), sz, threshold, alghorithm);
		data.resize(sz);
		memcpy(data.data(), new_buf, sz * sizeof(DTYPE));
	}
}


template<typename DTYPE>
DTYPE *DiagramData::downsample(DTYPE *data, int &data_length, int threshold, DiagramData::DOWNSAMPLING_ALGO alghorithm) {
	if (data_length>threshold) {
		DTYPE *new_data = new DTYPE[threshold*2];
		switch(alghorithm) {
		case DS_LOWPASS:
			lowpassDownsampling(data, data_length, new_data, threshold);
		    break;
		case DS_THREEBUCKET:
			largestTriangleDownsampling(data, data_length, new_data, threshold);
			break;
		}
		delete [] data;
		data_length=threshold;
		return new_data;
	} else {
		return data;
	}
}

template<typename DTYPE>
DTYPE *DiagramData::downsample2(DTYPE *data, int sequence_length, int &data_length, int threshold, DiagramData::DOWNSAMPLING_ALGO alghorithm) {
	if (data_length>threshold) {
		DTYPE *new_data=new DTYPE[sequence_length * threshold * 2];

		for (int n=0; n<sequence_length; ++n) {
			int off=n * threshold * 2;
			switch(alghorithm) {
			case DS_LOWPASS:
				lowpassDownsampling( data+off, data_length, new_data+off, threshold);
				break;
			case DS_THREEBUCKET:
				largestTriangleDownsampling( data+off, data_length, new_data + off, threshold);
				break;
			}
		}
		data_length=threshold;
		delete [] data;
		return new_data;

	} else {
		return data;
	}
}



/**
 * downsampling dataset.
 * shameless stolen from Sveinn Steinarsson
 * see https://github.com/sveinn-steinarsson/flot-downsample
 */
template<typename DTYPE>
void DiagramData::largestTriangleDownsampling(DTYPE *data, int data_length, DTYPE *new_data, int threshold) {
	DTYPE *sampled = new_data;
	int sampled_index = 0;

	// Bucket size. Leave room for start and end data points
	double every = double(data_length - 2) / double(threshold - 2);

	int a = 0,  // Initially a is the first point in the triangle
	next_a;

	// Always add the first point
    sampled[ sampled_index++ ] = data[ a*2 ];
    sampled[ sampled_index++ ] = data[ a*2 + 1 ];

    for (int i=0; i<threshold - 2; i++) {

        // Calculate point average for next bucket (containing c)
        double avg_x = 0,
               avg_y = 0;
        int avg_range_start  = floor( double(i+1) * every ) + 1,
            avg_range_end    = floor( double(i+2) * every ) + 1;

        if (avg_range_end > data_length)
        	avg_range_end=data_length;

        int avg_range_length = avg_range_end - avg_range_start;

        for ( ; avg_range_start<avg_range_end; avg_range_start++ ) {
			avg_x += data[ avg_range_start*2 + 0 ];
			avg_y += data[ avg_range_start*2 + 1 ];
        }
        avg_x /= double(avg_range_length);
        avg_y /= double(avg_range_length);

        // Get the range for this bucket
        int range_offs = floor( double(i) * every ) + 1,
            range_to   = floor( double(i+1) * every ) + 1;

        // Point a
        double point_a_x = data[ a*2 ],
               point_a_y = data[ a*2 + 1];

        double max_area = -1;
        double area = -1;

        int max_area_point;
        for ( ; range_offs < range_to; range_offs++ ) {
            // Calculate triangle area over three buckets
            area = abs( ( point_a_x - avg_x ) * ( data[ range_offs*2 + 1 ] - point_a_y ) -
                        ( point_a_x - data[ range_offs*2 + 0 ] ) * ( avg_y - point_a_y )
                      ) * 0.5;
            if ( area > max_area ) {
                max_area = area;
                max_area_point = range_offs;
                next_a = range_offs; // Next a is this b
            }
        }

        // Pick this point from the bucket
        sampled[ sampled_index++ ] = data[ max_area_point*2 ];
        sampled[ sampled_index++ ] = data[ max_area_point*2 + 1 ];

        // This a is the next a (chosen b)
        a = next_a;
    }

    sampled[ sampled_index++ ] = data[ data_length - 2 ]; // Always add last
    sampled[ sampled_index++ ] = data[ data_length - 1 ];
}

/**
 * downsampling dataset;
 * we use a simple lowpass over the incoming series
 */
template<typename DTYPE>
void DiagramData::lowpassDownsampling(DTYPE *data, int data_length, DTYPE *new_data, int threshold) {
	const double e=0.5;	// lowpass coeff

	int counter=0;

	double lp=data[1];	// init lowpass with first value in incoming set
	double step=(double) data_length / (double) threshold;
	DTYPE *p=new_data;
	// set first bin to incoming series
	*p++=data[0];
	*p++=data[1];
	double idx=1;
	for (int n=1; n<data_length-1; ++n) {
		// simple lowpass
		double val=(1-e)*lp+data[n*2+1]*e;
		idx += 1.0;;
		if (idx>step) {
			idx -= step;
			*p++ = data[n*2];
			*p++ = val;
			++counter;
		}
		lp=val;
	}

	// set last bin
	new_data[(threshold-1)*2 + 0] = data[(data_length-1)*2];
	new_data[(threshold-1)*2 + 1] = lp;
}



#if 0
/**
 * flatten dataset by moving averedge
 */
void DiagramData::movingAverage(double *data, int data_length, int winsize) {
	double avg=0;
	double *window=new double[winsize];

	int n,m;

	// fill first half of window
	for (int m=0; m<winsize/2; ++m) {
		window[m]=data[m];
		avg+=data[m];
	}

	m=winsize/2;
	// move thru dataset,
	for (n=0; n<data_length; ++n) {
		window[m % winsize] = data[n*2];

		avg+=data[n*2];							// add this datapoint

		if (m < winsize)						// didn't got enough points yet
			data[m] = avg/m;
		else if (data_length-n < winsize)		// not enough datapoints left
			data[m] = avg / (data_length-n);
		else
			data[n] = avg / winsize;

		if (m>winsize)
			avg -= window[(m+1) % winsize];		// substract first datapoint in window
	}

	delete window;
}

/**
 * flatten dataset by first order lowpass
 */
void DiagramData::lowpass(double *data, int data_length, double coeff) {
	double lp=0;
	double *p=data+1;
	for (int n=0; n<data_length; ++n) {
		double val=(1-coeff)*lp + coeff* (*p);
		*p=val;
		lp=val;
		++p;
	}
}

#endif

#endif // DIAGRAMGDATA_IMPL_
