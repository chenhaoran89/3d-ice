/******************************************************************************
 * Source file "3D-ICe/sources/termal_data.c"                                 *
 *                                                                            *
 * This file is part of 3D-ICe (http://esl.epfl.ch/3D-ICe), revision 0.1      *
 *                                                                            *
 * 3D-ICe is free software: you can redistribute it and/or modify it under    *
 * the terms of the GNU General Public License as published by the Free       *
 * Software Foundation, either version 3 of the License, or any later         *
 * version.                                                                   *
 *                                                                            *
 * 3D-ICe is distributed in the hope that it will be useful, but WITHOUT      *
 * ANY WARRANTY; without even the implied warranty of MERCHANTABILITY or      *
 * FITNESS FOR A PARTICULAR PURPOSE. See the GNU General Public License for   *
 * more details.                                                              *
 *                                                                            *
 * You should have received a copy of the GNU General Public License along    *
 * with 3D-ICe.  If not, see <http://www.gnu.org/licenses/>.                  *
 *                                                                            *
 * Copyright (C) 2010,                                                        *
 * Embedded Systems Laboratory - Ecole Polytechnique Federale de Lausanne.    *
 * All Rights Reserved.                                                       *
 *                                                                            *
 * Authors: Alessandro Vincenzi, Arvind Sridhar.                              *
 *                                                                            *
 * EPFL-STI-IEL-ESL                                                           *
 * Bâtiment ELG, ELG 130                                                      *
 * Station 11                                                                 *
 * 1015 Lausanne, Switzerland                          threed-ice@esl.epfl.ch *
 ******************************************************************************/

#include <stdlib.h>
#include <stdio.h>

#include "thermal_data.h"

/******************************************************************************/

static void init_data (double* data, Quantity_t size, double init_value)
{
  while (size--) *data++ = init_value ;
}

/******************************************************************************/

int init_thermal_data
(
  StackDescription* stkd,
  ThermalData*      tdata,
  enum MatrixStorage_t  storage,
  Temperature_t     initial_temperature,
  Time_t            delta_time
)
{
  tdata->Size                = get_number_of_cells(stkd->Dimensions) ;
  tdata->initial_temperature = initial_temperature ;
  tdata->delta_time          = delta_time ;

  /* Memory allocation */

  if ( (tdata->Temperatures
          = (Temperature_t*) malloc ( sizeof(Temperature_t) * tdata->Size )
       ) == NULL )

    goto temperatures_fail ;

  if ( (tdata->Sources
          = (Source_t*) malloc ( sizeof(Source_t) * tdata->Size )
       ) == NULL )

    goto sources_fail ;

  if ( (tdata->Capacities
         = (Capacity_t*) malloc ( sizeof(Capacity_t) * tdata->Size )
       ) == NULL )

    goto capacities_fail ;

  if ( (tdata->Conductances
          = (Conductances*) malloc (sizeof(Conductances)*tdata->Size)
       ) == NULL)

    goto conductances_fail ;

  if ( (tdata->SLU_PermutationMatrixR
         = (int *) malloc ( sizeof(int) * tdata->Size )) == NULL )

    goto slu_perm_r_fail ;

  if ( (tdata->SLU_PermutationMatrixC
    = (int *) malloc ( sizeof(int) * tdata->Size )) == NULL )

    goto slu_perm_c_fail ;

  if ( (tdata->SLU_Etree
         = (int *) malloc ( sizeof(int) * tdata->Size )) == NULL )

    goto slu_etree_fail ;

  if ( alloc_system_matrix (&tdata->SM_A, storage,
                            tdata->Size, stkd->Dimensions->Grid.NNz) == 0)
    goto sm_a_fail ;

  if ( alloc_system_vector (&tdata->SV_B, tdata->Size) == 0 )
    goto sv_b_fail ;

  StatInit (&tdata->SLU_Stat) ;

  /* Set initial values */

  init_data (tdata->Temperatures, tdata->Size, initial_temperature) ;

  init_data (tdata->Sources, tdata->Size, 0.0) ;

  set_default_options (&tdata->SLU_Options) ;

  tdata->SLU_Options.Fact            = DOFACT ;
  tdata->SLU_Options.PrintStat       = NO ;
  tdata->SLU_Options.Equil           = NO ;
  tdata->SLU_Options.SymmetricMode   = YES ;
  tdata->SLU_Options.ColPerm         = MMD_AT_PLUS_A ;
  tdata->SLU_Options.DiagPivotThresh = 0.01 ;

  if (storage == TL_CRS_MATRIX)

    dCreate_CompRow_Matrix  /* Matrix A */
    (
      &tdata->SLUMatrix_A, tdata->Size, tdata->Size, tdata->SM_A.NNz,
      tdata->SM_A.Values, tdata->SM_A.Columns, tdata->SM_A.Rows,
      SLU_NR, SLU_D, SLU_GE
    ) ;

  else

    dCreate_CompCol_Matrix  /* Matrix A */
    (
      &tdata->SLUMatrix_A, tdata->Size, tdata->Size, tdata->SM_A.NNz,
      tdata->SM_A.Values, tdata->SM_A.Rows, tdata->SM_A.Columns,
      SLU_NC, SLU_D, SLU_GE
    ) ;

  dCreate_Dense_Matrix  /* Vector B */
  (
    &tdata->SLUMatrix_B, tdata->Size, 1,
    tdata->SV_B.Values, tdata->Size,
    SLU_DN, SLU_D, SLU_GE
  );

  return 1 ;

  /* Free if malloc errors */

sv_b_fail :
  free_system_matrix (&tdata->SM_A) ;
sm_a_fail :
  free (tdata->SLU_Etree) ;
slu_etree_fail :
  free (tdata->SLU_PermutationMatrixR);
slu_perm_c_fail :
  free (tdata->SLU_PermutationMatrixR) ;
slu_perm_r_fail :
  free (tdata->Conductances) ;
conductances_fail :
  free (tdata->Capacities) ;
capacities_fail :
  free (tdata->Sources) ;
sources_fail :
  free (tdata->Temperatures) ;
temperatures_fail :

  return 0 ;
}

/******************************************************************************/

void free_thermal_data (ThermalData* tdata)
{
  if (tdata == NULL) return ;

  free (tdata->Temperatures) ;
  free (tdata->Sources) ;
  free (tdata->Capacities) ;
  free (tdata->Conductances) ;

  free (tdata->SLU_PermutationMatrixR) ;
  free (tdata->SLU_PermutationMatrixC) ;
  free (tdata->SLU_Etree) ;

  StatFree (&tdata->SLU_Stat) ;

  Destroy_SuperMatrix_Store (&tdata->SLUMatrix_A) ;
  free_system_matrix        (&tdata->SM_A) ;

  Destroy_SuperMatrix_Store (&tdata->SLUMatrix_B);
  free_system_vector        (&tdata->SV_B) ;

  if (tdata->SLU_Options.Fact != DOFACT )
  {
    Destroy_CompCol_Permuted (&tdata->SLUMatrix_A_Permuted) ;
    Destroy_SuperNode_Matrix (&tdata->SLUMatrix_L) ;
    Destroy_CompCol_Matrix   (&tdata->SLUMatrix_U) ;
  }
}

/******************************************************************************/

int fill_thermal_data
(
  StackDescription* stkd,
  ThermalData*      tdata
)
{
  if (stkd->Channel->FlowRateChanged == TRUE_V)
  {
    fill_conductances_stack_description (stkd, tdata->Conductances) ;

    //fill_conductances_heatsink (stkd->HeatSink, tdata->Conductances) ;

    fill_capacities_stack_description (stkd, tdata->Capacities,
                                             tdata->delta_time) ;

    fill_system_matrix
    (
      stkd,
      &tdata->SM_A,
      tdata->Conductances,
      tdata->Capacities
    ) ;

    if (tdata->SLU_Options.Fact == FACTORED)

      tdata->SLU_Options.Fact = SamePattern ;

    else

      tdata->SLU_Options.Fact = DOFACT ;


    fill_sources_stack_description (stkd, tdata->Sources,
                                          tdata->Conductances) ;

    fill_system_vector (&tdata->SV_B, tdata->Sources,
                                      tdata->Capacities,
                                      tdata->Temperatures) ;

    stkd->Channel->FlowRateChanged = FALSE_V ;
    stkd->PowerValuesChanged = FALSE_V ;
  }

  if (stkd->PowerValuesChanged == TRUE_V)
  {
    fill_sources_stack_description (stkd, tdata->Sources,
                                          tdata->Conductances) ;

    fill_system_vector (&tdata->SV_B, tdata->Sources,
                                      tdata->Capacities,
                                      tdata->Temperatures) ;

    stkd->PowerValuesChanged = FALSE_V ;
  }

  if (tdata->SLU_Options.Fact != FACTORED )
  {
    get_perm_c (tdata->SLU_Options.ColPerm,
                &tdata->SLUMatrix_A,
                tdata->SLU_PermutationMatrixC) ;

    sp_preorder (&tdata->SLU_Options, &tdata->SLUMatrix_A,
                 tdata->SLU_PermutationMatrixC, tdata->SLU_Etree,
                 &tdata->SLUMatrix_A_Permuted) ;

    dgstrf (&tdata->SLU_Options, &tdata->SLUMatrix_A_Permuted,
            sp_ienv(2), sp_ienv(1), /* relax and panel size */
            tdata->SLU_Etree,
            NULL, 0,                /* work and lwork */
            tdata->SLU_PermutationMatrixC, tdata->SLU_PermutationMatrixR,
            &tdata->SLUMatrix_L, &tdata->SLUMatrix_U,
            &tdata->SLU_Stat, &tdata->SLU_Info) ;

    tdata->SLU_Options.Fact = FACTORED ;

    return tdata->SLU_Info ;
  }

  return 0 ;
}

/******************************************************************************/

int solve_system (ThermalData* tdata, Time_t total_time)
{
  int counter;

  if (tdata->SLU_Options.Fact == DOFACT)
    return 1 ;

  for ( ; total_time > 0 ; total_time -= tdata->delta_time)
  {
    dgstrs
    (
      NOTRANS,
      &tdata->SLUMatrix_L,
      &tdata->SLUMatrix_U,
      tdata->SLU_PermutationMatrixC,
      tdata->SLU_PermutationMatrixR,
      &tdata->SLUMatrix_B,
      &tdata->SLU_Stat,
      &tdata->SLU_Info
    ) ;

    if (tdata->SLU_Info != 0)
      break ;

    for (counter = 0 ; counter < tdata->SV_B.Size ; counter++)

      tdata->Temperatures[counter] = tdata->SV_B.Values[counter] ;

    fill_system_vector
    (
      &tdata->SV_B,
      tdata->Sources,
      tdata->Capacities,
      tdata->Temperatures
    ) ;
  }

  return tdata->SLU_Info ;
}

/******************************************************************************/
