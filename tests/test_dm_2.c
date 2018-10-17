/*
 *  This source code is part of MacroC: a finite element code
 *  to solve macrostructural problems for composite materials.
 *
 *  Copyright (C) - 2018 - Guido Giuntoli <gagiuntoli@gmail.com>
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
 *
 * ------------------------------------------------------------ 
 *  
 *  This example shows how to the boundary conditions on a DMDA object
 *  using the ISLocalToGlobalMapping given by DMGetLocalToGlobalMapping
 *
 */

#define DOF 1
#define NX 2
#define NY 2

#include "petscdm.h"
#include "petscdmda.h"

int main(int argc,char **argv)
{
    int ierr;
    DM da;
    Vec x;

    PetscInt *bc_global_ids;
    PetscReal *bc_values;
    const PetscInt *g_idx;

    int i, j, d;
    int si, sj;
    int nx, ny;
    int M, N;
    int nbcs;

    int rank;

    ISLocalToGlobalMapping ltogm;
    ierr = PetscInitialize(&argc, &argv, NULL, NULL);

    MPI_Comm_rank(PETSC_COMM_WORLD, &rank);

    ierr = DMDACreate2d(PETSC_COMM_WORLD, DM_BOUNDARY_NONE, DM_BOUNDARY_NONE,
                        DMDA_STENCIL_BOX, NX, NY, PETSC_DECIDE, PETSC_DECIDE,
                        DOF, 1, NULL, NULL, &da); CHKERRQ(ierr);

    ierr = DMSetFromOptions(da); CHKERRQ(ierr);
    ierr = DMSetUp(da); CHKERRQ(ierr);

    ierr = DMGetLocalToGlobalMapping(da, &ltogm); CHKERRQ(ierr);
    ierr = ISLocalToGlobalMappingGetIndices(ltogm, &g_idx); CHKERRQ(ierr);
    ierr = DMDAGetGhostCorners(da, &si, &sj, 0, &nx, &ny, 0); CHKERRQ(ierr);
    printf("rank %d\tsi:%d\tsj:%d\tnx:%d\tny:%d\n", rank, si, sj, nx, ny);

    printf("rank:%d\tg_idx:\n", rank);
    for (i = 0; i < (nx * ny) * DOF; ++i){
        printf("(%d,%d)\t", i, g_idx[i]);
    }

    bc_global_ids = malloc((2 * nx + 2 * ny) * DOF * sizeof(int));
    bc_values = malloc((2 * nx + 2 * ny) * DOF * sizeof(double));

    ierr = DMCreateGlobalVector(da, &x); CHKERRQ(ierr);
    ierr = VecZeroEntries(x); CHKERRQ(ierr);

    /* init the entries to -1 so VecSetValues will ignore them */
    for (i = 0; i < (2 * nx + 2 * ny) * DOF; ++i){
        bc_global_ids[i] = -1;
    }

    i = 0; /* X = 0 */
    for (j = 0; j < ny; ++j) {
        for (d = 0; d < DOF; ++d) {
            int local_id = i + j * nx;
            bc_global_ids[j * DOF + d] = g_idx[local_id * DOF + d];
            bc_values[j * DOF + d] = 1;
        }
    }
    printf("\n");

    nbcs = 0;
    if (si == 0) {
        nbcs = ny * DOF;
    }

    ISLocalToGlobalMappingRestoreIndices(ltogm,&g_idx);

    ierr = VecSetValues(x, nbcs, bc_global_ids, bc_values,
                        INSERT_VALUES); CHKERRQ(ierr);
    ierr = VecAssemblyBegin(x); CHKERRQ(ierr);
    ierr = VecAssemblyEnd(x); CHKERRQ(ierr);

    ierr = VecView(x, PETSC_VIEWER_STDOUT_WORLD); CHKERRQ(ierr);

    ierr = VecDestroy(&x); CHKERRQ(ierr);
    ierr = DMDestroy(&da); CHKERRQ(ierr);
    ierr = PetscFinalize();

    return ierr;
}