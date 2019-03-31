#include <stdio.h>
#include <stdlib.h>
#include <math.h>
#include "physics.h"
#include "init_program.h"
#include "linkedList.h"
#include "debug.h"
#include <omp.h>


/* Fazer um handler para tratar dos argumentos de entrada e passar para o init_particles*/

int main(int argc, char *argv[])
{
	//FILE *fp;
	grid_t grid;
	particle_t *par;
	parameters params; 
	int k = 0;
	int tid;

	omp_set_num_threads(6);

	//fp = fopen("positions.txt", "w");
	printf("Init particles\n");
	par = handler_input(argc, argv, &params);
	printf("Particles initiated\n");
	// Init Grid
	grid = initGrid(grid, params.ncside);
	printf("Start Simulation\n");
	// Time Step simulation
	for (k = 0; k < params.timeStep; k++) {
		#pragma omp parallel private(tid)
		{
			// get id of thread
      		tid = omp_get_thread_num();
      		if(tid == 0)
				printf("TIME STEP %d\n", k);
			#pragma omp for 
			// Run throw all the cells and computes all the center of mass
			for (int i = 0; i < params.ncside*params.ncside; ++i) {
				grid.m[i] = 0;
				grid.centerOfMassX[i] = 0;
				grid.centerOfMassY[i] = 0;	
			}
			
			int x, y;
			double *auxm = grid.m;
			double *auxCMx = grid.centerOfMassX;
			double *auxCMy = grid.centerOfMassY;
			double *auxMend;
			double *auxCMxEnd;
			double *auxCMyEnd;
			double auxMval;
			double auxCMxVal;
			double auxCMyVal;
			//#pragma omp parallel for reduction(+:auxm[:params.ncside * params.ncside], auxCMx[:params.ncside * params.ncside], auxCMy[:params.ncside * params.ncside])
			#pragma omp for 
			// Calculate center of mass of each grid cell
			for(int i = 0; i < params.n_part; i++) {
				x = par[i].gridCoordinateX;
				y = par[i].gridCoordinateY;
				auxMend = &(auxm[MATRIX(x, y, params.ncside)]);
				auxCMxEnd = &(auxCMx[MATRIX(x, y, params.ncside)]);
				auxCMyEnd = &(auxCMy[MATRIX(x, y, params.ncside)]);
				#pragma omp atomic read 
				auxMval = *auxMend;
				#pragma omp atomic read 
				auxCMxVal = *auxCMxEnd;
				#pragma omp atomic read 
				auxCMyVal = *auxCMyEnd;

				auxMval += par[i].m;
				auxCMxVal += par[i].m * par[i].positionX;
				auxCMyVal += par[i].m * par[i].positionY;

				#pragma omp atomic write 
				*auxCMxEnd = auxCMxVal;
				#pragma omp atomic write 
				*auxCMyEnd = auxCMyVal;
				#pragma omp atomic write 
				*auxMend = auxMval;
			}
			#pragma omp for 
			for (int i = 0; i < params.ncside*params.ncside; i++)
			{
				grid.centerOfMassX[i] = grid.centerOfMassX[i]/grid.m[i];
				grid.centerOfMassY[i] = grid.centerOfMassY[i]/grid.m[i];
			}
			
			// Compute interactions
			// Run all particles
			#pragma omp for 
			for(int i = 0; i < params.n_part; i++){

				//printf("tid %d - PARTICLE %d\n", tid, i);
				par[i].appliedForceX = 0;
				par[i].appliedForceY = 0;

				// Run the adjacent grids
				for(int j = -1; j <= 1; j++) {
					for (int m = -1; m <= 1; m++) {

						int sideUPDOWN = MIDDLE;
						int sideLEFTRIGHT = MIDDLE;
						int aux1 = 0, aux2 = 0;

						if(par[i].gridCoordinateX+j == -1) 
							sideLEFTRIGHT = LEFT;
						if(par[i].gridCoordinateX+j == params.ncside)
							sideLEFTRIGHT = RIGHT;
						if(par[i].gridCoordinateY+m == -1)
							sideUPDOWN = DOWN;
						if(par[i].gridCoordinateY+m == params.ncside)
							sideUPDOWN = UP;
						
						if(par[i].gridCoordinateX+j == -1) aux1 = par[i].gridCoordinateX+j + params.ncside;
						else if(par[i].gridCoordinateX+j == params.ncside) aux1 = par[i].gridCoordinateX+j - params.ncside;
						else aux1 = par[i].gridCoordinateX+j;
						
						if(par[i].gridCoordinateY+m == -1) aux2 = par[i].gridCoordinateY+m + params.ncside;
						else if(par[i].gridCoordinateY+m == params.ncside) aux2 = par[i].gridCoordinateY+m - params.ncside;
						else aux2 = par[i].gridCoordinateY+m;
						
						calculateGravForce(&(par[i]), grid.centerOfMassX[MATRIX(aux1, aux2, params.ncside) ], 
													grid.centerOfMassY[ MATRIX(aux1, aux2, params.ncside) ], 
													grid.m[ MATRIX(aux1, aux2, params.ncside) ], sideUPDOWN, sideLEFTRIGHT); //for each adjacent cell.---- 
					}
				}
				// Updates particles position and velocity and position on the grid
				par[i].vx += par[i].appliedForceX/par[i].m; //a = F/m 
				par[i].vy += par[i].appliedForceY/par[i].m;

				par[i].positionX += par[i].vx + 0.5 * par[i].appliedForceX/par[i].m;//x = x0 + v0t + 0.5 a t^2 (t = 1)
				par[i].positionY += par[i].vy + 0.5 * par[i].appliedForceY/par[i].m;

				//See if its out of bounds
				if(par[i].positionX >= 1) par[i].positionX = par[i].positionX - floor(par[i].positionX);
				else if(par[i].positionX < 0) par[i].positionX = 1 + (par[i].positionX - ceil(par[i].positionX)); 

				if(par[i].positionY >= 1) par[i].positionY = par[i].positionY - floor(par[i].positionY);
				else if(par[i].positionY < 0) par[i].positionY = 1 + (par[i].positionY - ceil(par[i].positionY));


				par[i].gridCoordinateX = par[i].positionX * params.ncside / 1;
				par[i].gridCoordinateY = par[i].positionY * params.ncside / 1;
			}
		}
	}
	

	double centerOfMassX = 0;
	double centerOfMassY = 0;
	double totalMass = 0;
	for(int i = 0; i < params.n_part; i++) {
		centerOfMassX += par[i].m * par[i].positionX;
		centerOfMassY += par[i].m * par[i].positionY;
		totalMass += par[i].m;
	}
	centerOfMassX /= totalMass;
	centerOfMassY /= totalMass;


	//printGrid(grid, params.ncside);

	printf("%.2f %.2f\n", par[0].positionX, par[0].positionY);
	printf("%.2f %.2f\n", centerOfMassX, centerOfMassY);
	
	/*FILE output for debugging*/
	/*for (i = 0; i < params.n_part; i++)
	{
		for ( j = 0; j < params.timeStep; j++)
		{
			fprintf(fp, "%Lf,%Lf\r\n", par[i].pastPositions[j].x, par[i].pastPositions[j].y);
		}
		printf("----\n");
		free(par[i].pastPositions);
	}
	fclose(fp);*/
	/*FILE output for debugging*/

	freeEverything(par, grid, params.ncside);
	return 0;
}

