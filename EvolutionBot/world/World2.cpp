#include "config.h"
#ifdef HAVE_MY_VECTORS


#include "World.h"


#include <ctime>


#include "settings.h"
#include "helpers.h"
#include "vmath.h"
#include <stdio.h>
#include <iostream>
#include "MyEnum.hpp"


using namespace std;


World::World() :
	CLOSED(false),
#ifdef HAVE_DEBUG
	IsDebug(false),
#endif
	pcontrol(false),
	pright(0),
	pleft(0),
	pinput1(0)
{
	//inititalize
	reset();
}

void World::update()
{
	modcounter++;
	vector<int> dt;
	float tinit;
	float tfin;

	//Process periodic events
	//Age goes up!
	if (modcounter % 100 == 0)
	{
		for (int n = 0, m = agents.TotalSizeOfAllocatedSpace(); n < m; n++)
		{
			Agent& rfa = agents[n];
			if (rfa.getStatusAgentAlive())
				rfa.age++;
		}
	}

	if (conf::REPORTS_PER_EPOCH > 0 && (modcounter % (10000 / conf::REPORTS_PER_EPOCH) == 0)) {
		//write report and record counts
		numHerbivore[ptr] = getHerbivores();
		numCarnivore[ptr] = getCarnivores();
		numFrugivore[ptr] = getFrugivores();
		numHybrid[ptr] = getHybrids();
		numTotal[ptr] = getAgents();
		ptr++;
		if (ptr == conf::RECORD_SIZE) ptr = 0;

		writeReport();
	}

	if (modcounter >= conf::FRAMES_PER_EPOCH)
	{
		modcounter = 0;
		current_epoch++;
	}
	if ((modcounter % conf::FOODADDFREQ == 0 && !CLOSED) || getFood() < conf::MINFOOD)
	{
		cx = randi(0, CW);
		cy = randi(0, CH);
		cells[0][cx][cy] = conf::FOODMAX;
	}
	if (modcounter % conf::HAZARDFREQ == 0)
	{
		cx = randi(0, CW);
		cy = randi(0, CH);
		cells[2][cx][cy] = conf::HAZARDMAX;
	}
	if (modcounter % conf::FRUITADDFREQ == 0)
	{
		cx = randi(0, CW);
		cy = randi(0, CH);
		if (cells[0][cx][cy] > conf::FOODMAX* conf::FRUITREQUIRE) cells[3][cx][cy] = conf::FRUITMAX;
	}

	for (cx = 0; cx < CW; cx++)
	{
		for (cy = 0; cy < CH; cy++)
		{
			//food = cells[0]...
			if (cells[0][cx][cy] > 0 && cells[0][cx][cy] < conf::FOODMAX) {
				cells[0][cx][cy] += conf::FOODGROWTH; //food quantity is changed by FOODGROWTH
			}
			cells[0][cx][cy] = capCell(cells[0][cx][cy], conf::FOODMAX); //cap food at FOODMAX

			//meat = cells[1]...
			if (cells[1][cx][cy] > 0 && cells[1][cx][cy] <= conf::MEATMAX)
			{
				cells[1][cx][cy] -= conf::MEATDECAY / cells[1][cx][cy]; //meat decays exponentially
			}
			cells[1][cx][cy] = capCell(cells[1][cx][cy], conf::MEATMAX); //cap at MEATMAX

			//hazard = cells[2]...
			if (cells[2][cx][cy] > 0)
			{
				cells[2][cx][cy] -= conf::HAZARDDECAY; //hazard decays
			}
			if (cells[2][cx][cy] > conf::HAZARDMAX * 9 / 10 && randf(0, 1) < 0.125)
			{
				cells[2][cx][cy] = 0; //instant hazards will be reset to zero
			}
			cells[2][cx][cy] = capCell(cells[2][cx][cy], conf::HAZARDMAX); //cap at HAZARDMAX

			//fruit = cells[3]...
			if (cells[0][cx][cy] <= conf::FOODMAX * conf::FRUITREQUIRE && cells[3][cx][cy] > 0)
			{
				cells[3][cx][cy] -= conf::FRUITDECAY; //fruit decays from lack of plant life
			}
			if (randf(0, 1) < conf::FOODSPREAD && cells[3][cx][cy] > 0.5 * conf::FRUITMAX)
			{
				//food seeding
				int ox = randi(cx - 1 - conf::FOODRANGE, cx + 2 + conf::FOODRANGE);
				int oy = randi(cy - 1 - conf::FOODRANGE, cy + 2 + conf::FOODRANGE);
				if (ox < 0) ox += CW;
				if (ox > CW - 1) ox -= CW;
				if (oy < 0) oy += CH;
				if (oy > CH - 1) oy -= CH; //code up to this point ensures world edges are crossed and not skipped
				if (cells[0][ox][oy] < conf::FOODMAX * 3 / 4) cells[0][ox][oy] += conf::FOODMAX / 4;
			}
			cells[3][cx][cy] = capCell(cells[3][cx][cy], conf::FRUITMAX); //cap
		}
	}

	//reset any counter variables per agent
	for (int n = 0, m = agents.TotalSizeOfAllocatedSpace(); n < m; n++)
	{
		Agent& rfa = agents[n];
		if (rfa.getStatusAgentAlive())
		{
			//reset spiked flag
			rfa.spiked = false;
			//process indicator (used in drawing)
			if (rfa.indicator > 0) rfa.indicator--;
			//reset dfood for processOutputs
			rfa.dfood = 0;
		}
	}

	//give input to every agent. Sets in[] array
	setInputs();

	//brains tick. computes in[] -> out[]
	brainsTick();

	//read output and process consequences of bots on environment. requires out[]
	processOutputs();

	//process bots:
	for (int n = 0, m = agents.TotalSizeOfAllocatedSpace(); n < m; n++)
	{
		Agent& rfa = agents[n];
		if (rfa.getStatusAgentAlive())
		{
			//find brain activity on this tick
			rfa.setActivity();

			//doLiveMutate
			float MR = rfa.MUTRATE1;
			float MR2 = rfa.MUTRATE2;
			rfa.brain.liveMutate(MR, MR2, rfa.out);
		}
	}

	for (int n = 0, m = agents.TotalSizeOfAllocatedSpace(); n < m; n++)
	{
		Agent& rfa = agents[n];
		if (rfa.getStatusAgentAlive())
		{
			//health and deaths
			float baseloss = conf::HEALTHLOSS_WHEELS * (abs(rfa.w1) + abs(rfa.w2)) / 2;

			baseloss *= rfa.metabolism / conf::MAXMETABOLISM;

			baseloss += rfa.age / conf::MAXAGE * conf::HEALTHLOSS_AGING; //getting older reduces health.

			if (rfa.boost) { //if boosting, init baseloss + age loss is multiplied
				baseloss *= conf::HEALTHLOSS_BOOSTMULT;
			}

			//process temperature preferences
			//calculate temperature at the agents spot. (based on distance from horizontal equator)
			float dd = 2.0 * abs(rfa.pos.y / conf::HEIGHT - 0.5);
			float discomfort = sqrt(abs(dd - rfa.temperature_preference));
			if (discomfort < 0.08) discomfort = 0;
			baseloss += conf::TEMPERATURE_DISCOMFORT * discomfort; //add to baseloss

			//brain activity reduces health
			baseloss += conf::HEALTHLOSS_BRAINUSE * rfa.brainActivity;

			rfa.health -= baseloss;
		}
	}

	for (int n = 0, m = agents.TotalSizeOfAllocatedSpace(); n < m; n++)
	{
		Agent& rfaI = agents[n];
		if (rfaI.getStatusAgentAlive())
		{
			//handle reproduction
			if (rfaI.repcounter < 0 && rfaI.health >= conf::MINMOMHEALTH && modcounter % 10 == 0)
			{
				//agent is healthy and is ready to reproduce.
				if (rfaI.give > 0.5)
				{
					for (int i = 0, j = agents.TotalSizeOfAllocatedSpace(); i < j; i++)
					{
						Agent& rfaJ = agents[i];
						if (rfaJ.getStatusAgentAlive())
						{
							float d = (rfaI.pos - rfaJ.pos).length();
							float deviation = abs(rfaI.species - rfaJ.species); //species deviation check
							if (d < conf::FOOD_SHARING_DISTANCE && n != i && rfaJ.give>0.5 && deviation <= conf::MAXDEVIATION)
							{
								//this adds conf::BABIES new agents to agents[], but with two parents
								reproduce(n, i, rfaI.MUTRATE1, rfaI.MUTRATE2, rfaJ.MUTRATE1, rfaJ.MUTRATE2);
								rfaI.children++;
								rfaJ.children++;
								//agents[i].health -= 0.2; //reduce health of birthing parent; not as much as assexual reproduction
								rfaI.repcounter = conf::REPRATE;
								rfaJ.repcounter = conf::REPRATE;
								break;
								continue;
							}
						}
					}
				}

				if (rfaI.give <= 0.5 && randf(0, 1) < 0.01)
				{
					//this adds conf::BABIES new agents to agents[], with just one parent
					reproduce(n, n, rfaI.MUTRATE1, rfaI.MUTRATE2, rfaI.MUTRATE1, rfaI.MUTRATE2);
					rfaI.children++;
					rfaI.health -= 0.4; //reduce health of birthing parent
					rfaI.repcounter = conf::REPRATE;
					continue;
				}
			}
		}
	}

	for (int n = 0, m = agents.TotalSizeOfAllocatedSpace(); n < m; n++)
	{
		Agent& rfa = agents[n];
		if (rfa.getStatusAgentAlive())
		{
			//remove dead agents. first distribute meat
			if (rfa.health <= 0)
			{
				cx = (int)rfa.pos.x / conf::CZ;
				cy = (int)rfa.pos.y / conf::CZ;

				float meat = cells[1][cx][cy];
				float agemult = 1.0;
				float spikedmult = 0.5;
				float stomachmult = (2 - rfa.stomach[1]) / 2; //carnivores give 50%
				if (rfa.age < 10) agemult = rfa.age * 0.1; //young killed agents should give very little resources until age 10
				if (rfa.spiked) spikedmult = 1; //agents which were spiked will give even more meat to the table

				meat += conf::MEATVALUE * conf::MEATMAX * agemult * spikedmult * stomachmult;
				cells[1][cx][cy] = capCell(meat, conf::MEATMAX);
			}
		}
	}

	for (int n = 0, m = agents.TotalSizeOfAllocatedSpace(); n < m; n++)
	{
		Agent& rfa = agents[n];
		if (rfa.getStatusAgentAlive())
		{
			if (rfa.health <= 0)
			{
				rfa.setStatusAgentAlive(false);
			}
			else if (rfa.selectflag == 1 && deleting == 1)
			{
				deleting = 0;
				rfa.setStatusAgentAlive(false);
			}
		}
	}

	//add new agents, if environment isn't closed
	if (!CLOSED)
	{
		//make sure environment is always populated with at least NUMBOTS bots
		while (agents.NrOfBots() < conf::NUMBOTS)
		{
			addRandomBots(1);
		}
		if (modcounter % 200 == 0)
		{
			if (randf(0, 1) < 0.5)
			{
				addRandomBots(1); //every now and then add random bots in
			}
		}
	}
}

void World::setInputs()
{
	// R1 G1 B1  R2 G2 B2  R3 G3 B3  R4 G4 B4 HEALTH CLOCK1 CLOCK2 SOUND HEARING SMELL BLOOD TEMP_DISCOMFORT PLAYER_INPUT1
	// 0  1  2   3  4  5   6  7  8   9  10 11   12	  13	  14	15	  16	  17	18	       19			   20

	float PI8 = M_PI / 8 / 2; //pi/8/2
	float PI38 = 3 * PI8; //3pi/8/2
	float PI4 = M_PI / 4;

#ifdef HAVE_OPENMP
#pragma omp parallel for
#endif
	for (int n = 0, m = agents.TotalSizeOfAllocatedSpace(); n < m; n++)
	{
		Agent& rfa = agents[n];
		if (rfa.getStatusAgentAlive())
		{
			//HEALTH
			rfa.in[12] = cap(rfa.health / 2); //divide by 2 since health is in [0,2]

			//FOOD
			cx = 0;
			cy = 0;
			//		a->in[#]= cells[0][cx][cy]/conf::FOODMAX;

			//		a->in[#]= cells[1][cx][cy]/conf::MEATMAX;

					//SOUND SMELL EYES
			vector<float> r(NUMEYES, 0);
			vector<float> g(NUMEYES, 0);
			vector<float> b(NUMEYES, 0);

			float smaccum = 0;

			vector<float> soaccum(NUMEARS, 0);
			vector<float> hearaccum(NUMEARS, 0);

			//BLOOD ESTIMATOR
			float blood = 0;

			//cell sense
			int minx, maxx, miny, maxy;
			int scx = (int)(rfa.pos.x / conf::CZ);
			int scy = (int)(rfa.pos.y / conf::CZ);

			minx = max((scx - 1 - conf::DIST / conf::CZ), (float)0);
			maxx = min((scx + 2 + conf::DIST / conf::CZ), (float)CW);
			miny = max((scy - 1 - conf::DIST / conf::CZ), (float)0);
			maxy = min((scy + 2 + conf::DIST / conf::CZ), (float)CH);

			//eyes: cell sight
			for (scx = minx; scx < maxx; scx++)
			{
				for (scy = miny; scy < maxy; scy++)
				{
					if (cells[0][scx][scy] == 0 && cells[1][scx][scy] == 0 && cells[2][scx][scy] == 0) continue;
					Vector2f cellpos = Vector2f((float)(scx * conf::CZ + conf::CZ / 2), (float)(scy * conf::CZ + conf::CZ / 2)); //find midpoint of the cell
					float d = (rfa.pos - cellpos).length();

					if (d < conf::DIST)
					{
						float angle = (cellpos - rfa.pos).get_angle();

						for (int q = 0; q < NUMEYES; q++)
						{
							float aa = rfa.angle + rfa.eyedir[q];
							if (aa < -M_PI) aa += 2 * M_PI;
							if (aa > M_PI) aa -= 2 * M_PI;

							float diff1 = aa - angle;
							if (fabs(diff1) > M_PI) diff1 = 2 * M_PI - fabs(diff1);
							diff1 = fabs(diff1);

							float fov = rfa.eyefov[q];
							if (diff1 < fov)
							{
								//we see this cell with this eye. Accumulate stats
								float mul1 = rfa.eye_see_cell_mod * (fabs(fov - diff1) / fov) * ((conf::DIST - d) / conf::DIST) * (d / conf::DIST) * 2;
								r[q] += mul1 * 0.25 * cells[1][scx][scy]; //meat looks red
								g[q] += mul1 * 0.25 * cells[0][scx][scy]; //plants are green
								r[q] += mul1 * 0.25 * cells[3][scx][scy]; //fruit looks yellow
								g[q] += mul1 * 0.25 * cells[3][scx][scy];
								b[q] += mul1 * 0.25 * cells[2][scx][scy]; //hazards are blue???
							#ifdef HAVE_DEBUG
								if (a.selectflag && getIsDebug())
								{
									linesA.push_back(a.pos);
									linesB.push_back(cellpos);
								}
							#endif
							}
						}

						float forwangle = rfa.angle;
						float diff4 = forwangle - angle;
						if (fabs(forwangle) > M_PI) diff4 = 2 * M_PI - fabs(forwangle);
						diff4 = fabs(diff4);
						if (diff4 < PI38)
						{
							float mul4 = ((PI38 - diff4) / PI38) * (1 - d / conf::DIST);
							//meat can also be sensed with blood sensor
							blood += mul4 * 0.3 * cells[1][scx][scy];
						}
								}
							}
						}

			for (int i = 0, j = agents.TotalSizeOfAllocatedSpace(); i < j; i++)
			{
				if (n == i) continue;
				Agent& rfa2 = agents[i];
				if (rfa2.getStatusAgentAlive())
				{
					if (rfa.pos.x<rfa2.pos.x - conf::DIST || rfa.pos.x>rfa2.pos.x + conf::DIST
						|| rfa.pos.y > rfa2.pos.y + conf::DIST || rfa.pos.y < rfa2.pos.y - conf::DIST) continue;

					float d = (rfa.pos - rfa2.pos).length();

					if (d < conf::DIST)
					{
						//smell
						smaccum += 1 - d / conf::DIST;

						//sound and hearing
						for (int q = 0; q < NUMEARS; q++)
						{
							Vector2f v(rfa.radius, 0);
							v.rotate(rfa.angle + rfa.eardir[q]);

							Vector2f earpos = rfa.pos + v;

							float eardist = (earpos - rfa2.pos).length();

							//sound
							soaccum[q] += rfa.sound_mod * (1 - eardist / conf::DIST) * (max(fabs(rfa2.w1), fabs(rfa2.w2)));

							//hearing. Listening to other agents
							hearaccum[q] += rfa.hear_mod * (1 - eardist / conf::DIST) * rfa2.soundmul;
						}

						//eyes: bot sight
						float ang = (rfa2.pos - rfa.pos).get_angle(); //current angle between bots

						for (int q = 0; q < NUMEYES; q++)
						{
							float aa = rfa.angle + rfa.eyedir[q];
							if (aa < -M_PI) aa += 2 * M_PI;
							if (aa > M_PI) aa -= 2 * M_PI;

							float diff1 = aa - ang;
							if (fabs(diff1) > M_PI) diff1 = 2 * M_PI - fabs(diff1);
							diff1 = fabs(diff1);

							float fov = rfa.eyefov[q];
							if (diff1 < fov)
							{
								//we see a2 with this eye. Accumulate stats
								float mul1 = rfa.eye_see_agent_mod * (fabs(fov - diff1) / fov) * (1 - d / conf::DIST) * (1 - d / conf::DIST);
								//p[q] += mul1*(d/conf::DIST);
								r[q] += mul1 * rfa2.red;
								g[q] += mul1 * rfa2.gre;
								b[q] += mul1 * rfa2.blu;
							#ifdef HAVE_DEBUG
								if (rfa.selectflag && getIsDebug())
								{
									linesA.push_back(rfa.pos);
									linesB.push_back(rfa2.pos);
								}
							#endif
							}
						}

						//blood sensor
						float forwangle = rfa.angle;
						float diff4 = forwangle - ang;
						if (fabs(forwangle) > M_PI) diff4 = 2 * M_PI - fabs(forwangle);
						diff4 = fabs(diff4);
						if (diff4 < PI38)
						{
							float mul4 = ((PI38 - diff4) / PI38) * (1 - d / conf::DIST);
							//if we can see an agent close with both eyes in front of us
							blood += mul4 * (1 - rfa2.health / 2); //remember: health is in [0 2]
							//agents with high life dont bleed. low life makes them bleed more
						}
					}
				}
			}

			//temperature varies from 0 to 1 across screen.
			//it is 0 at equator (in middle), and 1 on edges. Agents can sense this range
			float temp = 2.0 * abs(rfa.pos.y / conf::HEIGHT - 0.5);

			smaccum *= rfa.smell_mod;
			blood *= rfa.blood_mod;

			//		for(int e;e<NUMEYES;e++){

			rfa.in[0] = cap(r[0]);
			rfa.in[1] = cap(g[0]);
			rfa.in[2] = cap(b[0]);

			rfa.in[3] = cap(r[1]);
			rfa.in[4] = cap(g[1]);
			rfa.in[5] = cap(b[1]);

			rfa.in[6] = cap(r[2]);
			rfa.in[7] = cap(g[2]);
			rfa.in[8] = cap(b[2]);

			rfa.in[9] = cap(r[3]);
			rfa.in[10] = cap(g[3]);
			rfa.in[11] = cap(b[3]);

			rfa.in[13] = abs(sin((modcounter + current_epoch / conf::FRAMES_PER_EPOCH) / rfa.clockf1));
			rfa.in[14] = abs(sin((modcounter + current_epoch / conf::FRAMES_PER_EPOCH) / rfa.clockf2));
			rfa.in[15] = cap(soaccum[0]);
			rfa.in[16] = cap(soaccum[1]);
			rfa.in[17] = cap(hearaccum[0]);
			rfa.in[18] = cap(hearaccum[1]);
			rfa.in[19] = cap(smaccum);
			rfa.in[20] = cap(blood);
			rfa.in[21] = temp;

			if (rfa.selectflag)
			{
				rfa.in[22] = pinput1;
			}
			else
			{
				rfa.in[22] = 0;
			}
		}
	}
}

void World::processOutputs()
{
	//assign meaning
	//LEFT RIGHT R G B SPIKE BOOST SOUND_MULTIPLIER GIVING CHOICE STIMULANT JUMP
	// 0	1	 2 3 4   5	   6		 7			   8	  9	     10		 11
#ifdef HAVE_OPENMP
#pragma omp parallel for
#endif
	for (int i = 0, j = agents.TotalSizeOfAllocatedSpace(); i < j; i++)
	{
		Agent& rfa = agents[i];
		if (rfa.getStatusAgentAlive())
		{
			if (rfa.jump <= 0) //if not jumping, then change wheel speeds. otherwise, we want to keep wheel speeds constant
			{
				if (pcontrol && rfa.selectflag)
				{
					rfa.w1 = pright;
					rfa.w2 = pleft;
				}
				else
				{
					rfa.w1 = rfa.out[0]; //-(2*a->out[0]-1);
					rfa.w2 = rfa.out[1]; //-(2*a->out[1]-1);
				}
			}
			rfa.red += 0.2 * (rfa.out[2] - rfa.red);
			rfa.gre += 0.2 * (rfa.out[3] - rfa.gre);
			rfa.blu += 0.2 * (rfa.out[4] - rfa.blu);
			if (rfa.jump <= 0) rfa.boost = rfa.out[6] > 0.5; //if jump height is zero, boost can change
			rfa.soundmul = rfa.out[7];
			rfa.give = rfa.out[8];

			//spike length should slowly tend towards out[5]
			float g = rfa.out[5];
			if (rfa.spikeLength < g) {
				rfa.spikeLength += conf::SPIKESPEED;
				rfa.health -= conf::HEALTHLOSS_SPIKE_EXT;
			}
			else if (rfa.spikeLength > g) rfa.spikeLength = g; //its easy to retract spike, just hard to put it up

			//jump gets set to 2*(out[11] - 0.5) if itself is zero (the bot is on the ground) and if out[11] is greater than 0.5
			float height = (rfa.out[11] - 0.5);
			if (rfa.jump == 0 && height > 0) rfa.jump = height * 2;
		}
	}

	//move bots
#ifdef HAVE_OPENMP
#pragma omp parallel for
#endif
	for (int i = 0, j = agents.TotalSizeOfAllocatedSpace(); i < j; i++)
	{
		Agent& rfa = agents[i];
		if (rfa.getStatusAgentAlive())
		{
			//IDK where else to put this, but this looks like as good a place as any
			rfa.jump -= conf::GRAVITYACCEL;
			if (rfa.jump < -1) rfa.jump = 0; //-1 because we will be nice and give a "recharge" time between jumps

			Vector2f v(rfa.radius / 2, 0);
			v.rotate(rfa.angle + M_PI / 2);

			Vector2f w1p = rfa.pos + v; //wheel positions
			Vector2f w2p = rfa.pos - v;

			float BW1 = conf::BOTSPEED * rfa.w1;
			float BW2 = conf::BOTSPEED * rfa.w2;
			if (rfa.boost) //if boosting
			{
				BW1 = BW1 * conf::BOOSTSIZEMULT;
				BW2 = BW2 * conf::BOOSTSIZEMULT;
			}

			//move bots
			Vector2f vv = w2p - rfa.pos;
			vv.rotate(-BW1);
			rfa.pos = w2p - vv;
			if (rfa.jump <= 0)
			{
				rfa.angle -= BW1;
			}
			if (rfa.angle < -M_PI) rfa.angle = M_PI - (-M_PI - rfa.angle);
			vv = rfa.pos - w1p;
			vv.rotate(BW2);
			rfa.pos = w1p + vv;
			if (rfa.jump <= 0)
			{
				rfa.angle += BW2;
			}
			if (rfa.angle > M_PI) rfa.angle = -M_PI + (rfa.angle - M_PI);

			//wrap around the map
			if (rfa.pos.x < 0) rfa.pos.x += conf::WIDTH;
			if (rfa.pos.x >= conf::WIDTH) rfa.pos.x -= conf::WIDTH;
			if (rfa.pos.y < 0) rfa.pos.y += conf::HEIGHT;
			if (rfa.pos.y >= conf::HEIGHT) rfa.pos.y -= conf::HEIGHT;
		}
	}

	//process interaction with cells
#ifdef HAVE_OPENMP
#pragma omp parallel for
#endif
	for (int i = 0, j = agents.TotalSizeOfAllocatedSpace(); i < j; i++)
	{
		Agent& rfa = agents[i];
		if (rfa.getStatusAgentAlive())
		{
			int scx = (int)rfa.pos.x / conf::CZ;
			int scy = (int)rfa.pos.y / conf::CZ;

			if (rfa.jump <= 0) //no interaction with these cells if jumping
			{
				//plant food: layer 0
				float food = cells[0][scx][scy];
				float plantintake = 0;
				if (food > 0 && rfa.health < 2)
				{
					//agent eats the food
					float speedmul = 1 - max(abs(rfa.w1), abs(rfa.w2));
					plantintake = min(food, conf::FOODINTAKE) * rfa.stomach[0] * (1 - rfa.stomach[1]) * (1 - rfa.stomach[2]) * speedmul;
					if (rfa.health >= conf::MINMOMHEALTH) rfa.repcounter -= rfa.metabolism * plantintake;
					rfa.health += plantintake;
					cells[0][scx][scy] -= min(food, conf::FOODWASTE * plantintake / conf::FOODINTAKE);
				}

				//meat food: layer 1
				float meat = cells[1][scx][scy];
				float meatintake = 0;
				if (meat > 0 && rfa.health < 2)
				{
					//agent eats meat
					float speedmul = 1 - max(abs(rfa.w1), abs(rfa.w2));
					meatintake = min(meat, conf::MEATINTAKE) * rfa.stomach[1] * (1 - rfa.stomach[0]) * (1 - rfa.stomach[2]) * speedmul;
					if (rfa.health >= conf::MINMOMHEALTH) rfa.repcounter -= rfa.metabolism * meatintake;
					rfa.health += meatintake;
					cells[1][scx][scy] -= min(meat, conf::MEATWASTE * meatintake / conf::MEATINTAKE);
				}

				//hazards: layer 2
				float hazard = cells[2][scx][scy];
				if (hazard > 0)
				{
					rfa.health -= conf::HAZARDDAMAGE * hazard;
				}
				//agents fill up hazard cells only up to 9/10, because any greater can be reset to zero
				if (modcounter % 5 == 0)
				{
					if ((hazard + conf::HAZARDDEPOSIT) <= conf::HAZARDMAX * 9 / 10) hazard += min(conf::HAZARDDEPOSIT, conf::HAZARDMAX * 9 / 10 - hazard);
					cells[2][scx][scy] = capCell(hazard, conf::HAZARDMAX);
				}

				//Fruit food: layer 3
				float fruit = cells[3][scx][scy];
				float fruitintake = 0;
				if (fruit > 0 && rfa.health < 2)
				{
					//agent eats meat
					float speedmul = 1 - max(abs(rfa.w1), abs(rfa.w2));
					fruitintake = min(fruit, conf::FRUITINTAKE) * rfa.stomach[2] * (1 - rfa.stomach[0]) * (1 - rfa.stomach[1]) * speedmul;
					if (rfa.health >= conf::MINMOMHEALTH) rfa.repcounter -= rfa.metabolism * fruitintake;
					rfa.health += fruitintake;
					cells[3][scx][scy] -= min(fruit, conf::FRUITWASTE * fruitintake / conf::FRUITINTAKE);
				}
			}

			//land/water: layer 4
			float air = cells[4][scx][scy];
			if (pow(air - rfa.lungs, 2) >= 0.01) rfa.health -= conf::HEALTHLOSS_BADAIR * pow(air - rfa.lungs, 2);

			if (rfa.health > 2) //if health has been increased over the cap, convert the overflow into repcounter
			{
				rfa.repcounter -= rfa.metabolism * (rfa.health - 2);
				rfa.health = 2;
			}
		}
	}

	//process giving and receiving of food
#ifdef HAVE_OPENMP
#pragma omp parallel for //CONSIDER: making this modcounter-based w/ timed reset
#endif
	for (int i = 0, j = agents.TotalSizeOfAllocatedSpace(); i < j; i++)
	{
		Agent& rfaI = agents[i];
		if (rfaI.getStatusAgentAlive())
		{
			if (rfaI.give > 0.5)
			{
				for (int n = 0, m = agents.TotalSizeOfAllocatedSpace(); n < m; n++)
				{
					if (i == n) continue;

					Agent& rfaJ = agents[n];
					if (rfaJ.getStatusAgentAlive())
					{
						float d = (rfaI.pos - rfaJ.pos).length();
						if (d < conf::FOOD_SHARING_DISTANCE && rfaJ.health < rfaI.health)
						{
							//initiate transfer
							rfaJ.health += conf::FOODTRANSFER;
							rfaI.health -= conf::FOODTRANSFER;
							rfaJ.dfood += conf::FOODTRANSFER; //only for drawing
							rfaI.dfood -= conf::FOODTRANSFER;
						}
					}
				}
			}
		}
	}

	//process collision dynamics
	if (modcounter % 2 == 0) //we dont need to do this TOO often. can save efficiency here since this is n^2 op in #agents
	{
		//		#pragma omp parallel for
		for (int i = 0, j = agents.TotalSizeOfAllocatedSpace(); i < j; i++)
		{
			Agent& rfaI = agents[i];
			if (rfaI.getStatusAgentAlive())
			{
				for (int j = agents.TotalSizeOfAllocatedSpace() - 1; j; j--)
				{
					if (i == j) continue;

					Agent& rfaJ = agents[j];
					if (rfaJ.getStatusAgentAlive())
					{
						float d = (rfaI.pos - rfaJ.pos).length();
						float sumrad = rfaI.radius + rfaJ.radius;
						if (d < sumrad && rfaI.jump <= 0 && rfaJ.jump <= 0)
						{
							//if inside each others radii and neither are jumping, fix physics
							float ov = (sumrad - d);
							if (ov > 0 && d > 0.0001)
							{
								if (ov > conf::TOOCLOSE) //if bots are too close, they get injured before being pushed away
								{
									float DMG = ov * conf::HEALTHLOSS_BUMP;
									rfaI.health -= DMG * sumrad / 2 / rfaI.radius; //larger bots take less damage, bounce less
									rfaJ.health -= DMG * sumrad / 2 / rfaJ.radius;
									if (rfaI.ir == 0) { rfaI.initEvent(30, 0, 0.5, 1); rfaJ.initEvent(30, 0, 0.5, 1); }
								}
								float ff1 = ov / d * rfaJ.radius / rfaI.radius * conf::REACTPOWER; //the radii come in here for inertia-like effect
								float ff2 = ov / d * rfaI.radius / rfaJ.radius * conf::REACTPOWER;
								rfaI.pos.x -= (rfaJ.pos.x - rfaI.pos.x) * ff1;
								rfaI.pos.y -= (rfaJ.pos.y - rfaI.pos.y) * ff1;
								rfaJ.pos.x += (rfaJ.pos.x - rfaI.pos.x) * ff2;
								rfaJ.pos.y += (rfaJ.pos.y - rfaI.pos.y) * ff2;

								if (rfaI.pos.x > conf::WIDTH) rfaI.pos.x -= conf::WIDTH;
								if (rfaI.pos.y > conf::HEIGHT) rfaI.pos.y -= conf::HEIGHT;
								if (rfaJ.pos.x > conf::WIDTH) rfaJ.pos.x -= conf::WIDTH;
								if (rfaJ.pos.y > conf::HEIGHT) rfaJ.pos.y -= conf::HEIGHT;
								if (rfaI.pos.x < 0) rfaI.pos.x += conf::WIDTH;
								if (rfaI.pos.y < 0) rfaI.pos.y += conf::HEIGHT;
								if (rfaJ.pos.x < 0) rfaJ.pos.x += conf::WIDTH;
								if (rfaJ.pos.y < 0) rfaJ.pos.y += conf::HEIGHT;

								//printf("%f, %f, %f, %f, and %f\n", a->pos.x, a->pos.y, a2->pos.x, a2->pos.y, ov);
							}
						}

						//low speed doesn't count, nor does a small spike (duh). If the target is jumping in midair, can't attack either
						if (rfaI.spikeLength * conf::SPIKELENGTH < rfaI.radius || rfaI.w1 < 0.2 || rfaI.w2 < 0.2 || rfaJ.jump>0) continue;
						else if (d <= sumrad + conf::SPIKELENGTH * rfaI.spikeLength)
						{
							//these two are in collision and agent i has extended spike and is going decent fast!
							Vector2f v(1, 0);
							v.rotate(rfaI.angle);
							float diff = v.angle_between(rfaJ.pos - rfaI.pos);
							if (fabs(diff) < M_PI / 8)
							{
								//bot i is also properly aligned!!! that's a hit
								float DMG = conf::SPIKEMULT * rfaI.spikeLength * max(fabs(rfaI.w1), fabs(rfaI.w2));

								rfaJ.health -= DMG;

								if (rfaI.health > 2) rfaI.health = 2; //cap health at 2
								rfaI.spikeLength = 0; //retract spike back down

								rfaI.initEvent(20 * DMG, 1, 1, 0); //yellow event means bot has spiked the other bot. nice!
								if (rfaJ.health <= 0) rfaI.initEvent(20 * DMG, 1, 0, 0); //red event means bot has killed the other bot. nice!

								Vector2f v2(1, 0);
								v2.rotate(rfaJ.angle);
								float adiff = v.angle_between(v2);
								if (fabs(adiff) < M_PI / 2)
								{
									//this was attack from the back. Retract spike of the other agent (startle!)
									//this is done so that the other agent cant right away "by accident" attack this agent
									rfaJ.spikeLength = 0;
								}

								rfaJ.spiked = true; //set a flag saying that this agent was hit this turn
							}
						}
					}
				}
			}
		}
	}
}

void World::brainsTick()
{
#ifdef HAVE_OPENMP
#pragma omp parallel for
#endif
	for (int i = 0, j = agents.TotalSizeOfAllocatedSpace(); i < j; i++)
	{
		Agent& rfa = agents[i];
		if (rfa.health>0)
			rfa.tick();
	}
}

void World::positionOfInterest(int type, float& xi, float& yi)
{
	int maxi = -1;

	switch (type)
	{
		case 1: //the interest of type 1 is the oldest agent
			{
				int maxage = -1;

				for (int i = 0, j = agents.TotalSizeOfAllocatedSpace(); i < j; i++)
				{
					Agent& rfaI = agents[i];
					if (rfaI.getStatusAgentAlive())
					{
						if (rfaI.age > maxage)
						{
							maxage = rfaI.age;
							maxi = i;
						}
					}
				}
			}
			break;
		case 2: //interest of type 2 is the selected agent
			for (int i = 0, j = agents.TotalSizeOfAllocatedSpace(); i < j; i++)
			{
				Agent& rfaI = agents[i];
				if (rfaI.getStatusAgentAlive())
				{
					if (rfaI.selectflag == 1)
					{
						maxi = i;
						break;
					}
				}
			}
			break;
		case 3: //interest of type 3 is most advanced generation
			{
				int maxgen = 0;
				for (int i = 0, j = agents.TotalSizeOfAllocatedSpace(); i < j; i++)
				{
					Agent& rfaI = agents[i];
					if (rfaI.getStatusAgentAlive())
					{
						if (rfaI.gencount > maxgen)
						{
							maxgen = rfaI.gencount;
							maxi = i;
						}
					}
				}
			}
			break;
		case 4: //interest of type 4 is healthiest
			{
				float maxhealth = -1;
				for (int i = 0, j = agents.TotalSizeOfAllocatedSpace(); i < j; i++)
				{
					Agent& rfaI = agents[i];
					if (rfaI.getStatusAgentAlive())
					{
						if (rfaI.health > maxhealth * 0.9)
						{ //0.9 multiplier is there to reduce quick-jumps when there is competition
							maxhealth = rfaI.health;
							maxi = i;
						}
					}
				}
			}
			break;
		case 5: //interest of type 5 is most productive
		{
			float maxprog = -1;
			for (int i = 0, j = agents.TotalSizeOfAllocatedSpace(); i < j; i++)
			{
				Agent& rfaI = agents[i];
				if (rfaI.getStatusAgentAlive())
				{
					if (rfaI.age != 0)
					{
						if ((float)(rfaI.children / rfaI.age) > maxprog)
						{
							maxprog = rfaI.children / rfaI.age;
							maxi = i;
						}
					}
				}
			}
		}
		break;
		default:
			break;
	}

	if (maxi != -1)
	{
		xi = agents[maxi].pos.x;
		yi = agents[maxi].pos.y;
	}
}

void World::addCarnivore()
{
	agents.AddAgent(idcounter, FeedingStrategi::Carnivore);
	++idcounter;
}

void World::addHerbivore()
{
	agents.AddAgent(idcounter, FeedingStrategi::Herbivore);
	++idcounter;
}

void World::addRandomBots(int num, int type)
{
	for (int i = 0; i < num; i++)
	{
		if (type == 1) addHerbivore(); //herbivore
		else if (type == 2) addCarnivore(); //carnivore
		else
		{
			agents.AddAgent(idcounter, FeedingStrategi::Unspecific);
			++idcounter;
		}
	}
}

void World::reproduce(int ai, int bi, float aMR, float aMR2, float bMR, float bMR2)
{
	float MR = min(aMR, bMR);
	float MR2 = min(aMR2, bMR2); //Using min because max, though correct, will lead to rampant mutation (is that perhaps wanted?) will test
	if (randf(0, 1) < 0.04) MR = MR * randf(1, 10);
	if (randf(0, 1) < 0.04) MR2 = MR2 * randf(1, 10);

	Agent that = agents[bi];
	Agent a = agents[ai];
	if (ai == bi)
	{
		that = agents[ai];
		agents[ai].initEvent(30, 0, 0.8, 0); //green event means agent asexually reproduced
	}
	else
	{
		agents[ai].initEvent(30, 0, 0, 0.8);
		agents[bi].initEvent(30, 0, 0, 0.8); //blue events mean agents sexually reproduced.
	}

	for (int i = 0; i < conf::BABIES; i++)
	{
		Agent a2 = a.reproduce(that, MR, MR2);
		if (ai != bi)
		{
			a2.hybrid = true; //if parents are not the same agent (sexual reproduction), then mark the child
		}
		a2.id = idcounter;
		idcounter++;
		agents.ReproduceAgent(a2);
	}
}

void World::writeReport()
{
	printf("Writing Report, Epoch: %i\n", current_epoch);
	//save all kinds of nice data stuff
	int topcarn = 0;
	int topfrug = 0;
	int topherb = 0;
	int happyspecies = 0;

	for (int i = 0, j = agents.TotalSizeOfAllocatedSpace(); i < j; i++)
	{
		Agent& rfa = agents[i];
		if (rfa.getStatusAgentAlive())
		{
			if ((rfa.stomach[0] >= rfa.stomach[1] && rfa.stomach[0] >= rfa.stomach[2])
				&& rfa.gencount > topherb) topherb = rfa.gencount;
			if ((rfa.stomach[1] >= rfa.stomach[0] && rfa.stomach[1] >= rfa.stomach[2])
				&& rfa.gencount > topfrug) topfrug = rfa.gencount;
			if ((rfa.stomach[2] >= rfa.stomach[0] && rfa.stomach[2] >= rfa.stomach[1])
				&& rfa.gencount > topcarn) topcarn = rfa.gencount;
		}
	}

	FILE* fr = fopen("report.txt", "a");
	fprintf(fr, "Epoch: %i #Agents: %i #Herbi: %i #Frugi: %i #Carni: %i #0.75Plants: %i #0.5Meat: %i #0.1Hazard: %i TopH: %i TopF: %i TopC: %i #Hybrids: %i #Spikes: %i\n",
		current_epoch, getAgents(), getHerbivores(), getFrugivores(), getCarnivores(), getFood(), getMeat(), getHazards(), topherb, topfrug, topcarn, getHybrids(), getSpiked());
	fclose(fr);
}


void World::reset()
{
	current_epoch = 0;
	modcounter = 0;
	idcounter = 0;
	CW = conf::WIDTH / conf::CZ;
	CH = conf::HEIGHT / conf::CZ; //note: can add custom variables from loaded savegames here possibly

	//kill all agents from previous simulation
	for (int n = 0, m = agents.TotalSizeOfAllocatedSpace(); n < m; n++)
		agents[n].setStatusAgentAlive(false);

	//handle layers
	for (cx = 0; cx < CW; cx++)
	{
		for (cy = 0; cy < CH; cy++)
		{
			for (int l = 0; l < LAYERS; l++)
			{
				cells[l][cx][cy] = 0;
			}
			//			cells[>>>TEMPERATURE_LAYER<<<][cx][cy]= 2.0*abs((float)cy/CH - 0.5); [old temperature indicating code]
		}
	}

	cellsLandMasses(4);

	//open report file; null it up if it exists
	FILE* fr = fopen("report.txt", "w");
	fclose(fr);

	ptr = 0;

	//add random food
	while (getFood() < conf::INITFOOD)
	{
		cells[0][randi(0, CW)][randi(0, CH)] = conf::FOODMAX;
	}

	addRandomBots(conf::NUMBOTS, 1);
}

void World::setClosed(bool close)
{
	CLOSED = close;
}

bool World::isClosed() const
{
	return CLOSED;
}


void World::processMouse(int button, int state, int x, int y)
{
	if (state == 0)
	{
		float mind = 1e10;
		float mini = -1;
		float d;

		for (int n = 0, m = agents.TotalSizeOfAllocatedSpace(); n < m; n++)
		{
			Agent& rfaI = agents[n];
			if (rfaI.getStatusAgentAlive())
			{
				d = pow(x - rfaI.pos.x, 2) + pow(y - rfaI.pos.y, 2);
				if (d < mind)
				{
					mind = d;
					mini = n;
				}
			}
		}
		if (mind < 1000)
		{
			//toggle selection of this agent
			for (int i = 0, j = agents.TotalSizeOfAllocatedSpace(); i < j; i++)
			{
				Agent& rfaJ = agents[i];
				if (rfaJ.getStatusAgentAlive())
				{
					if (i != mini) rfaJ.selectflag = false;
				}
			}
			agents[mini].selectflag = !agents[mini].selectflag;
			agents[mini].initEvent(10, 1.0, 1.0, 1.0);
			agents[mini].printSelf();
			setControl(false);
		}
	}
}

void World::draw(View* view, int layer)
{
	//draw cell layer
	if (layer != 0)
	{
		for (int x = 0; x < CW; x++)
		{
			for (int y = 0; y < CH; y++)
			{
				float val;
				switch (layer)
				{
					case 1: //plant food
						val = 0.5 * cells[0][x][y] / conf::FOODMAX;
						break;
					case 2: //meat food
						val = 0.5 * cells[1][x][y] / conf::MEATMAX;
						break;
					case 3: //hazards
						val = 0.5 * cells[2][x][y] / conf::HAZARDMAX;
						break;
					case 4: //fruit food
						val = 0.5 * cells[3][x][y] / conf::FRUITMAX;
						break;
					case 5: //land
						val = cells[4][x][y];
						break;
					/*case TEMPERATURE_LAYER: //temperature
						val = cells[3][x][y];
						break;*/
					default:
						break;
				}

				view->drawCell(x, y, val);
			}
		}
	}

	//draw all agents
	for (int n = 0, m = agents.TotalSizeOfAllocatedSpace(); n < m; n++)
		if (agents[n].getStatusAgentAlive())
			view->drawAgent(agents[n]);

	view->drawData();
}

int World::getHerbivores() const
{
	int count = 0;
	for (int n = 0, m = agents.TotalSizeOfAllocatedSpace(); n < m; n++)
	{
		Agent& rfa = agents[n];
		if (rfa.getStatusAgentAlive())
		{
			if (rfa.stomach[0] >= rfa.stomach[1] && rfa.stomach[0] >= rfa.stomach[2]) count++;
		}
	}
	return count;
}

int World::getCarnivores() const
{
	int count = 0;
	for (int n = 0, m = agents.TotalSizeOfAllocatedSpace(); n < m; n++)
	{
		Agent& rfa = agents[n];
		if (rfa.getStatusAgentAlive())
		{
			if (rfa.stomach[1] >= rfa.stomach[0] && rfa.stomach[1] >= rfa.stomach[2]) count++;
		}
	}
	return count;
}

int World::getFrugivores() const
{
	int count = 0;
	for (int n = 0, m = agents.TotalSizeOfAllocatedSpace(); n < m; n++)
	{
		Agent& rfa = agents[n];
		if (rfa.getStatusAgentAlive())
		{
			if (rfa.stomach[2] >= rfa.stomach[0] && rfa.stomach[2] >= rfa.stomach[1]) count++;
		}
	}
	return count;
}

int World::getAgents() const
{
	return agents.NrOfBots();
}

int World::getFood() const //count plant cells with 75% max or more
{
	int numfood = 0;
	for (int i = 0; i < CW; i++)
	{
		for (int j = 0; j < CH; j++)
		{
			float food = 0.5 * cells[0][i][j] / conf::FOODMAX;
			if (food > conf::FOODMAX * 3 / 4)
			{
				numfood++;
			}
		}
	}

	return numfood;
}

int World::getMeat() const //count meat cells with 50% max or more
{
	int nummeat = 0;
	for (int i = 0; i < CW; i++) {
		for (int j = 0; j < CH; j++) {
			float meat = 0.5 * cells[1][i][j] / conf::MEATMAX;
			if (meat > conf::MEATMAX / 2) {
				nummeat++;
			}
		}
	}
	return nummeat;
}

int World::getHazards() const //count hazard cells with 10% max or more
{
	int numhazards = 0;
	for (int i = 0; i < CW; i++) {
		for (int j = 0; j < CH; j++) {
			float hazard = 0.5 * cells[2][i][j] / conf::HAZARDMAX;
			if (hazard > conf::HAZARDMAX * 0.1) {
				numhazards++;
			}
		}
	}
	return numhazards;
}

int World::getHybrids() const
{
	int numhybrid = 0;
	for (int n = 0, m = agents.TotalSizeOfAllocatedSpace(); n < m; n++)
	{
		Agent& rfa = agents[n];
		if (rfa.getStatusAgentAlive())
		{
			if (rfa.hybrid) numhybrid++;
		}
	}
	return numhybrid;
}

int World::getSpiked() const
{
	//gets number of agents with spikes, not spiked agents
	int count = 0;
	
	for (int n = 0, m = agents.TotalSizeOfAllocatedSpace(); n < m; n++)
	{
		Agent& rfa = agents[n];
		if (rfa.getStatusAgentAlive())
		{
			if (rfa.spikeLength * conf::SPIKELENGTH >= rfa.radius) count++;
		}
	}

	return count;
}

int World::epoch() const
{
	return current_epoch;
}

void World::setControl(bool state)
{
	//reset left and right wheel controls 
	pleft = 0;
	pright = 0;

	pcontrol = state;
}

void World::cellsRandomFill(int layer, float amount, int number)
{
	for (int i = 0; i < number; i++) {
		cx = randi(0, CW);
		cy = randi(0, CH);
		cells[layer][cx][cy] = amount;
	}
}

void World::cellsLandMasses(int layer)
{
	//creates land masses for the layer given
	int leftcount = CW * CH;

	for (int i = 0; i < CW; i++)
	{
		for (int j = 0; j < CH; j++)
		{
			cells[layer][i][j] = -1; //"null" all cells
		}
	}

	for (int i = 0; i < conf::CONTINENTS; i++)
	{
		//spawn init continents (land= 1)
		cx = randi(0, CW);
		cy = randi(0, CH);
		cells[layer][cx][cy] = 1;
	}
	for (int i = 0; i < sqrt(CW * CH * conf::OCEANPERCENT) / 10; i++)
	{
		//spawn oceans (water= 0)
		cx = randi(0, CW);
		cy = randi(0, CH);
		cells[layer][cx][cy] = 0;
	}

	while (leftcount != 0)
	{
#ifdef HAVE_OPENMP
#pragma omp parallel for
#endif
		for (int i = 0; i < CW; i++)
		{
			for (int j = 0; j < CH; j++)
			{
				//land spread
				if (cells[layer][i][j] == 1)
				{
					int ox = randi(i - 1, i + 2);
					int oy = randi(j - 1, j + 2);
					if (ox < 0) ox += CW;
					if (ox > CW - 1) ox -= CW;
					if (oy < 0) oy += CH;
					if (oy > CH - 1) oy -= CH;
					if (cells[layer][ox][oy] == -1 && randf(0, 1) < 0.9) cells[layer][ox][oy] = 1;
				}

				//water spread
				if (cells[layer][i][j] == 0)
				{
					int ox = randi(i - 1, i + 2);
					int oy = randi(j - 1, j + 2);
					if (ox < 0) ox += CW;
					if (ox > CW - 1) ox -= CW;
					if (oy < 0) oy += CH;
					if (oy > CH - 1) oy -= CH;
					if (cells[layer][ox][oy] == -1 && randf(0, 1) < 0.9) cells[layer][ox][oy] = 0;
				}
			}
		}

		leftcount = 0;
		for (int i = 0; i < CW; i++)
		{
			for (int j = 0; j < CH; j++)
			{
				if (cells[layer][i][j] == -1)
				{
					leftcount++;
				}
			}
		}
	}
}

float World::capCell(float a, float top) const
{
	return min(top, max(a, (float)0));
}

#ifdef HAVE_DEBUG
void World::setIsDebug(bool state)
{
	IsDebug = state;
}
bool World::getIsDebug() const
{
	return IsDebug;
}
#endif


#endif /* HAVE_MY_VECTORS */
