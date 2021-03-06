#ifndef INIT_PROGRAM_H
#define INIT_PROGRAM_H
#include "physics.h"

#include <mpi.h>


#define LEFTPROCESS 0
#define	UPLEFTPROCESS 1
#define	UPPROCESS 2
#define	UPRIGHTPROCESS 3
#define	RIGHTPROCESS 4
#define	DOWNRIGHTPROCESS 5
#define	DOWNPROCESS 6
#define	DOWNLEFTPROCESS 7

typedef struct _parameters{
	long seed;
	long ncside;
	long long gridSize;
	long long n_part;
	long long activeParticles;
	long long partVectSize;
	long long reallocInc;
	long timeStep;
	long xSize;
	long ySize;
	long xLowerBound;
	long xUpperBound;
	long yLowerBound;
	long yUpperBound;
	long sizeVertical;
	long sizeHorizontal;
	long sizeVerticalBase;
	long sizeHorizontalBase;
} parameters;

extern parameters params;

void handler_input(int argc ,char *argv[]);

vector2grid findPosition(particle_t par, long ncside);

particle_t * init_particles(particle_t *par, int numberOfProcess, int rank);

particle_t * CreateParticleArray(int numberOfProcess);

int findGridDivision(int numberOfProcess, int rank);

grid_tt ** initGridSendReceive(int rank);

grid_t initTotalGrid(grid_t grid, long ncside);

grid_t initPartialGrid(int numberOfProcess, int processID, grid_t grid);

int * findNeighborsRank(int* idToSend, int rank, int numberOfProcess);

void freeParticles(particle_t *par);

void freeParticlesFinal(particle_t_final *particle_recv);

void freeParReceive(particle_t_reduced *parReceive);

void freeParSend(particle_t_reduced **parSend);

void freeGridSendReceive(grid_tt **gridSendReceive);

void freeGrid(grid_t particleGrid);

particle_t_reduced * initParReceived(long long n_part, long *size, int rank, long *incSizeParReceive);

particle_t_reduced ** initParSend(long long n_part, long *size, int rank, long *incSizeParSend);

#endif