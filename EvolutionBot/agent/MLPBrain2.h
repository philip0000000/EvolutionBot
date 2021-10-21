#include "config.h"
#ifdef HAVE_MY_VECTORS


#ifndef MLPBRAIN2_H
#define MLPBRAIN2_H


#include <vector>
#include <stdio.h>


#include "settings.h"
#include "helpers.h"
#include "MyVectors2.h"
#include "VectorMLPBox.h"


class MLPBox
{
public:
	MLPBox();
	void RandomiseMLPBox();

	MyVector<float> w; //weight of each connecting box
	MyVector<int> id; //id in boxes[] of the connecting box
	MyVector<int> type; //0: regular synapse. 1: change-sensitive synapse. 2: memory trigger synapse
	float kp; //damper
	float gw; //global w
	float bias;

	//state variables
	float target; //target value this node is going toward
	float out; //current output
	float oldout; //output a tick ago
};


/**
 * Damped Weighted Recurrent AND/OR Network
 */
class MLPBrain
{
public:
	MyVector< MLPBox> boxes;

	MLPBrain();
	void RandomiseMLPBrain();
	MLPBrain(const MLPBrain& other);
	virtual MLPBrain& operator=(const MLPBrain& other);

	void tick(MyVector<float>& in, MyVector<float>& out);
	float getActivity();
	void initMutate(float MR, float MR2);
	void liveMutate(float MR, float MR2, MyVector<float>& out);
	MLPBrain crossover(const MLPBrain& other);
};


#endif /* MLPBRAIN2_H */


#endif /* HAVE_MY_VECTORS */
