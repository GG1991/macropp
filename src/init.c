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
 */


#include "macroc.h"

int init()
{
    int ierr;
    char mess[64];

    final_time = FINAL_TIME;
    ts = TIME_STEPS;
    dt = DT;

    nx = 10; ny = 10; nz = 10;

    PetscOptionsGetReal(NULL, NULL, "-dt", &dt, NULL);
    PetscOptionsGetInt(NULL, NULL, "-ts", &ts, NULL);

    DMBoundaryType bx = DM_BOUNDARY_NONE, by = DM_BOUNDARY_NONE,
                   bz = DM_BOUNDARY_NONE;
    ierr = DMDACreate3d(PETSC_COMM_WORLD, bx, by, bz, DMDA_STENCIL_BOX,
                        10, 10, 10,
                        PETSC_DECIDE, PETSC_DECIDE, PETSC_DECIDE,
                        3, 1, NULL, NULL, NULL, &DA);
    ierr = DMDASetUniformCoordinates(DA, 0.0, 1.0, 0.0, 1.0, 0.0, 1.0);
    CHKERRQ(ierr);
    ierr = DMSetMatType(DA, MATAIJ); CHKERRQ(ierr);
    ierr = DMSetFromOptions(DA); CHKERRQ(ierr);
    ierr = DMSetUp(DA); CHKERRQ(ierr);
    ierr = DMCreateMatrix(DA, &A); CHKERRQ(ierr);
    ierr = DMCreateGlobalVector(DA, &u); CHKERRQ(ierr);
    ierr = DMCreateGlobalVector(DA, &b); CHKERRQ(ierr);
    ierr = DMCreateGlobalVector(DA, &du); CHKERRQ(ierr);

    int istart, iend;
    //MatGetOwnershipRange(A, &istart, &iend);
    //printf("rank %d - A: istart: %d - iend: %d\n", rank, istart, iend);

    int sx, sy, sz;
    //ierr = DMDAGetGhostCorners(DA, &sx, &sy, &sz, &nxl, &nyl, &nzl); CHKERRQ(ierr);
    //printf("rank %d - sx: %d - sy: %d - sz: %d - nxl: %d - nyl: %d - nzl: %d\n", rank, sx, sy, sz, nxl, nyl, nzl);

    ierr = DMDAGetElementsSizes(DA, &nexl, &neyl, &nezl); CHKERRQ(ierr);
    printf("rank %d - nexl: %d - neyl: %d - nezl: %d\n", rank, nexl, neyl, nezl);

    //ierr = VecGetOwnershipRange(u, &istart, &iend); CHKERRQ(ierr);
    //printf("rank %d - start: %d - end: %d\n", rank, istart, iend);

    dx = 1.;
    dy = 1.;
    dz = 1.;
    wg = dx * dy * dz / NPE;

    // Initializes <materials> declared in <micropp_c_wrapper.h>
    micropp_C_material_create();
    micropp_C_material_set(0, 1.0e7, 0.25, 1.0e4, 1.0e7, 0);
    micropp_C_material_set(1, 1.0e7, 0.25, 1.0e4, 1.0e7, 1);
    print0("Material Values:\n");
    if(!rank) {
        micropp_C_material_print(0);
        micropp_C_material_print(1);
    }

    // Initializes <micro> declared in <micropp_c_wrapper.h>
    int ngpl = nexl * neyl * nezl * NGP; 
    int size[3] = { 5, 5, 5 };
    int type = 1;
    double params[4] = { 1., 1., 1., .5 };
    micropp_C_create3(ngpl, size, type, params);

    return ierr;
}


int finish()
{
    int ierr;
    ierr = MatDestroy(&A); CHKERRQ(ierr);
    ierr = PetscFinalize();
    return ierr;
}
