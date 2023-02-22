#include "config.h"
#ifndef MY_VECTORS


#include "Agent.h"


#include "settings.h"
#include "helpers.h"
#include <stdio.h>
#include <iostream>
#include <string>
#include "MLPBrain.h"


Agent::Agent()
{
	// randomly spawned bots get the following attributes
	pos = Vector2f(randf(0, conf::WIDTH), randf(0, conf::HEIGHT));
	angle = randf(-M_PI, M_PI);
	health = 0.5 + randf(0, 0.5);
	age = 0;
	species = randi(-conf::NUMBOTS * 20, conf::NUMBOTS * 20); //related to numbots because it's a good relationship
	radius = randf(conf::MEANRADIUS * 0.2, conf::MEANRADIUS * 1.8);
	spikeLength = 0;
	red = 0;
	gre = 0;
	blu = 0;
	w1 = 0;
	w2 = 0;
	soundmul = 1;
	give = 0;
	clockf1 = randf(5, 100);
	clockf2 = randf(5, 100);
	boost = false;
	indicator = 0;
	gencount = 0;
	selectflag = false;
	ir = 0;
	ig = 0;
	ib = 0;
	lungs = randf(0, 1);
	hybrid = false;
	stomach[0] = randf(0, 1);
	stomach[1] = randf(0, 1);
	stomach[2] = randf(0, 1);
	repcounter = conf::REPRATE;
	metabolism = randf(0.2, conf::MAXMETABOLISM);

	genes[0] = NUMEYES;
	genes[1] = NUMEARS;

	id = 0;

	smell_mod = randf(0.1, 0.5);
	sound_mod = randf(0.5, 1.6);
	hear_mod = randf(0.5, 2);
	eye_see_agent_mod = randf(0.5, 3);
	eye_see_cell_mod = randf(0.1, 2);
	blood_mod = randf(0.5, 3);

	MUTRATE1 = 0.09; //randf(0.001, 0.005); //chance of mutations occuring
	MUTRATE2 = 0.005; //randf(0.01, 0.5); //severity of mutations

	spiked = false;

	in.resize(INPUTSIZE, 0);
	out.resize(OUTPUTSIZE, 0);

	brainActivity = 0;

	eardir.resize(NUMEARS, 0);
	for (int i = 0; i < NUMEARS; i++)
	{
		eardir[i] = randf(0, 2 * M_PI);
	}

	eyefov.resize(NUMEYES, 0);
	eyedir.resize(NUMEYES, 0);
	for (int i = 0; i < NUMEYES; i++)
	{
		eyefov[i] = randf(0.5, 2);
		eyedir[i] = randf(0, 2 * M_PI);
	}

	children = 0;
}
Agent Agent::reproduce(Agent father, float MR, float MR2)
{
	// create baby. Note that if the bot selects itself to mate with, this function acts also as asexual reproduction
	// NOTES: Agent "this" is mother, Agent "father" is father, Agent "child" is child
	// if a single parent's trait is required, use the mother's (this->)
	Agent child;

	//spawn the baby somewhere closeby behind agent
	//we want to spawn behind so that agents dont accidentally eat their young right away
	Vector2f fb(this->radius*3,0);
	fb.rotate(child.angle+M_PI);
	child.pos= this->pos + fb + Vector2f(randf(-this->radius*2,this->radius*2), randf(-this->radius*2,this->radius*2));
	if (child.pos.x<0) child.pos.x= conf::WIDTH+ child.pos.x;
	if (child.pos.x>conf::WIDTH) child.pos.x= child.pos.x-conf::WIDTH;
	if (child.pos.y<0) child.pos.y= conf::HEIGHT+ child.pos.y;
	if (child.pos.y>conf::HEIGHT) child.pos.y= child.pos.y-conf::HEIGHT;

	//basic trait inheritance
	// genes
	child.genes[0] = this->genes[0]; // eyes
	child.genes[1] = this->genes[1]; // ears

	child.gencount= std::max(this->gencount+1, father.gencount+1);
	child.metabolism= randf(0,1)<0.5 ? this->metabolism : father.metabolism;
	child.stomach[0]= randf(0,1)<0.5 ? this->stomach[0]: father.stomach[0];
	child.stomach[1]= randf(0,1)<0.5 ? this->stomach[1]: father.stomach[1];
	child.stomach[2]= randf(0,1)<0.5 ? this->stomach[2]: father.stomach[2];
	child.species= randf(0,1)<0.5 ? this->species : father.species;
	child.radius= randf(0,1)<0.5 ? this->radius : father.radius;

	child.MUTRATE1= randf(0,1)<0.5 ? this->MUTRATE1 : father.MUTRATE1;
	child.MUTRATE2= randf(0,1)<0.5 ? this->MUTRATE2 : father.MUTRATE2;
	child.clockf1= randf(0,1)<0.5 ? this->clockf1 : father.clockf1;
	child.clockf2= randf(0,1)<0.5 ? this->clockf2 : father.clockf2;

	child.smell_mod = randf(0,1)<0.5 ? this->smell_mod : father.smell_mod;
	child.sound_mod = randf(0,1)<0.5 ? this->sound_mod : father.sound_mod;
	child.hear_mod = randf(0,1)<0.5 ? this->hear_mod : father.hear_mod;
	child.eye_see_agent_mod = randf(0,1)<0.5 ? this->eye_see_agent_mod : father.eye_see_agent_mod;
	child.eye_see_cell_mod = randf(0,1)<0.5 ? this->eye_see_cell_mod : father.eye_see_cell_mod;
	child.blood_mod = randf(0,1)<0.5 ? this->blood_mod : father.blood_mod;

	child.lungs= randf(0,1)<0.5 ? this->lungs : father.lungs;
	
	child.eardir = randf(0,1)<0.5 ? this->eardir : father.eardir;

	child.eyefov = randf(0,1)<0.5 ? this->eyefov : father.eyefov;
	child.eyedir = randf(0,1)<0.5 ? this->eyedir : father.eyedir;

	// mutations
	// genes
	if (randf(0, 1) < MR / 3) child.genes[0] = AddOne(this->genes[0], 1, MAX_NUMEYES); // eyes
	if (randf(0, 1) < MR / 3) child.genes[1] = AddOne(this->genes[1], 1, MAX_NUMEARS); // ears

	if (randf(0,1)<MR/2) child.metabolism= randn(child.metabolism, MR2*5);
	if (child.metabolism<0) child.metabolism= 0;
	if (child.metabolism>conf::MAXMETABOLISM) child.metabolism= conf::MAXMETABOLISM;
	if (randf(0,1)<MR*5) child.stomach[0]= cap(randn(child.stomach[0], MR2*5));
	if (randf(0,1)<MR*5) child.stomach[1]= cap(randn(child.stomach[1], MR2*5));
	if (randf(0,1)<MR*5) child.stomach[2]= cap(randn(child.stomach[2], MR2*5));
	if (randf(0,1)<MR*100) child.species= (int) randn(child.species, MR2*100);
	if (randf(0,1)<MR*5) child.radius= randn(child.radius, MR2*3);
	if (child.radius<1) child.radius= 1;

	if (randf(0,1)<MR) child.MUTRATE1= abs(randn(child.MUTRATE1, conf::METAMUTRATE1));
	if (randf(0,1)<MR) child.MUTRATE2= abs(randn(child.MUTRATE2, conf::METAMUTRATE2));
	//we dont really want mutrates to get to zero; thats too stable. so take absolute randn instead.

	if (randf(0,1)<MR*5) child.clockf1= randn(child.clockf1, MR2/4);
	if (child.clockf1<2) child.clockf1= 2;
	if (randf(0,1)<MR*5) child.clockf2= randn(child.clockf2, MR2/4);
	if (child.clockf2<2) child.clockf2= 2;

	if (randf(0,1)<MR) child.smell_mod= randn(child.smell_mod, MR2);
	if (randf(0,1)<MR) child.sound_mod= randn(child.sound_mod, MR2);
	if (randf(0,1)<MR) child.hear_mod= randn(child.hear_mod, MR2);
	if (randf(0,1)<MR) child.eye_see_agent_mod= randn(child.eye_see_agent_mod, MR2);
	if (randf(0,1)<MR) child.eye_see_cell_mod= randn(child.eye_see_cell_mod, MR2);
	if (randf(0,1)<MR) child.blood_mod= randn(child.blood_mod, MR2);

	if (randf(0,1)<MR*5) child.lungs= cap(randn(child.lungs, MR2*3));

	for(int i=0;i<NUMEARS;i++){
		if(randf(0,1)<MR*2) child.eardir[i] = randn(child.eardir[i], MR2*5);
		if(child.eardir[i]<0) child.eardir[i] = 0;
		if(child.eardir[i]>2*M_PI) child.eardir[i] = 2*M_PI;
	}

	for(int i=0;i<NUMEYES;i++){
		if(randf(0,1)<MR) child.eyefov[i] = randn(child.eyefov[i], MR2/2);
		if(child.eyefov[i]<0) child.eyefov[i] = 0;
		if(child.eyefov[i]>M_PI) child.eyefov[i] = M_PI; //eyes cannot wrap around bot
		
		if(randf(0,1)<MR*2) child.eyedir[i] = randn(child.eyedir[i], MR2*5);
		if(child.eyedir[i]<0) child.eyedir[i] = 0;
		if(child.eyedir[i]>2*M_PI) child.eyedir[i] = 2*M_PI;
		// not going to loop coordinates; 0,2pi is bots front, so it provides a good point to "bounce" off of
	}
	
	// create brain here
	child.brain= this->brain.crossover(father.brain);
	child.brain.initMutate(MR,MR2);
	
	return child;
}

void Agent::printSelf()
{
	printf("\nAGENT, ID: %i\n", id);
	printf("Species # = %i\n", species);
	printf("MUTRATE1 = %f, MUTRATE2 HASTIGHET2 = %f\n", MUTRATE1, MUTRATE2);
	printf("children = %i\n", children);
	for (int i = 0; i < mutations.size(); i++)
	{
		std::cout << mutations[i];
	}
}
void Agent::initEvent(float size, float r, float g, float b)
{
	indicator = size;
	ir = r;
	ig = g;
	ib = b;
}

void Agent::setActivity()
{
	brainActivity = brain.getActivity();
}
void Agent::tick()
{
	brain.tick(in, out);
}

#endif /* MY_VECTORS */
