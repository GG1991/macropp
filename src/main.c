/*
 *  This source code is part of MacroC: a finite element code
 *  to solve macrostructural problems for composite materials.
 *
 *  Copyright (C) - 2018 - Guido Giuntoli <gagiuntoli@gmail.com>
 *                         Based on the PETSc example develop by:
 *                         Dave May
 *
 *  This program is free software: you can redistribute it and/or modify
 *  it under the terms of the GNU General Public License as published by
 *  the Free Software Foundation, either version 3 of the License, or
 *  (at your option) any later version.
 *
 *  This program is distributed in the hope that it will be useful,
 *  but WITHOUT ANY WARRANTY; without even the implied warranty of
 *  MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.  See the
 *  GNU General Public License for more details.
 *
 *  You should have received a copy of the GNU General Public License
 *  along with this program.  If not, see <https://www.gnu.org/licenses/>.
 */


#include "macroc.h"


int main(int argc,char **args)
{
  	int ierr;
  	int nx = 10, ny = 10, nz = 10;
  	int time_s, newton_it, tsteps = 1;
	double norm;
  	char mess[64];

  	ierr = PetscInitialize(&argc,&args,(char*)0,help); if(ierr) return ierr;
  	ierr = PetscOptionsGetInt(NULL,NULL,"-nx",&nx,NULL);CHKERRQ(ierr);
  	ierr = PetscOptionsGetInt(NULL,NULL,"-ny",&ny,NULL);CHKERRQ(ierr);
  	ierr = PetscOptionsGetInt(NULL,NULL,"-zy",&nz,NULL);CHKERRQ(ierr);
  	ierr = PetscOptionsGetInt(NULL,NULL,"-ts",&tsteps,NULL);CHKERRQ(ierr);

  	MPI_Comm_rank(MPI_COMM_WORLD, &rank);
  	MPI_Comm_size(MPI_COMM_WORLD, &nproc);

  	ierr = init(nx, ny, nz);

  	sprintf(mess, "Problem size %d\n", nproc);
  	print0(mess);

  	for(time_s = 0; time_s < tsteps; ++time_s) {

		ierr = set_bc(time_s);

        newton_it = 0;
  		while(newton_it < NEWTON_ITS ) {

	  		ierr = assembly_res();
        	/* norm = |b| */

	  		ierr = assembly_jac();
	  		ierr = solve_Ax();
        	/* u = u + du */

        	newton_it ++;
        }
  	}

  	ierr = solve_elasticity_2d(nx,ny);CHKERRQ(ierr);
  	ierr = PetscFinalize();
  	return ierr;
}
