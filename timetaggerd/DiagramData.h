/*
 * DiagramData.h
 *
 *  Created on: 16.06.2013
 *      Author: mag
 */

#ifndef DIAGRAMDATA_H_
#define DIAGRAMDATA_H_

#include <vector>

class DiagramData {

public:
	enum DOWNSAMPLING_ALGO {
		DS_THREEBUCKET=0,
		DS_LOWPASS=1
	};

	template<typename DTYPE> static void downsample(std::vector<DTYPE> &, int new_size, DOWNSAMPLING_ALGO alghorithm=DS_THREEBUCKET);
	template<typename DTYPE> static void downsample(std::vector< std::vector<DTYPE> > &, int new_size, DOWNSAMPLING_ALGO alghorithm=DS_THREEBUCKET);

	template<typename DTYPE> static DTYPE *downsample(DTYPE *, int &data_length, int threshold, DOWNSAMPLING_ALGO alghorithm=DS_THREEBUCKET);
	template<typename DTYPE> static DTYPE *downsample2(DTYPE *, int sequence_length, int &data_length, int threshold, DOWNSAMPLING_ALGO alghorithm=DS_THREEBUCKET);

	static void movingAverage(double *data, int data_length, int winsize);
	static void lowpass(double *data, int data_length, double coofficient);

protected:
	template<typename DTYPE> static void largestTriangleDownsampling(DTYPE *data, int data_length, DTYPE *new_data, int threshold);
	template<typename DTYPE> static void lowpassDownsampling(DTYPE *data, int data_length, DTYPE *new_data, int threshold);


};

#include "DiagramData.cpp"

#endif /* DIAGRAMDATA_H_ */
