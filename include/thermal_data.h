/******************************************************************************
 * This file is part of 3D-ICE, version 1.0.2 .                               *
 *                                                                            *
 * 3D-ICE is free software: you can  redistribute it and/or  modify it  under *
 * the terms of the  GNU General  Public  License as  published by  the  Free *
 * Software  Foundation, either  version  3  of  the License,  or  any  later *
 * version.                                                                   *
 *                                                                            *
 * 3D-ICE is  distributed  in the hope  that it will  be useful, but  WITHOUT *
 * ANY  WARRANTY; without  even the  implied warranty  of MERCHANTABILITY  or *
 * FITNESS  FOR A PARTICULAR  PURPOSE. See the GNU General Public License for *
 * more details.                                                              *
 *                                                                            *
 * You should have  received a copy of  the GNU General  Public License along *
 * with 3D-ICE. If not, see <http://www.gnu.org/licenses/>.                   *
 *                                                                            *
 *                             Copyright (C) 2010                             *
 *   Embedded Systems Laboratory - Ecole Polytechnique Federale de Lausanne   *
 *                            All Rights Reserved.                            *
 *                                                                            *
 * Authors: Arvind Sridhar                                                    *
 *          Alessandro Vincenzi                                               *
 *          Martino Ruggiero                                                  *
 *          Thomas Brunschwiler                                               *
 *          David Atienza                                                     *
 *                                                                            *
 * For any comment, suggestion or request  about 3D-ICE, please  register and *
 * write to the mailing list (see http://listes.epfl.ch/doc.cgi?liste=3d-ice) *
 *                                                                            *
 * EPFL-STI-IEL-ESL                                                           *
 * Batiment ELG, ELG 130                Mail : 3d-ice@listes.epfl.ch          *
 * Station 11                                  (SUBSCRIPTION IS NECESSARY)    *
 * 1015 Lausanne, Switzerland           Url  : http://esl.epfl.ch/3d-ice.html *
 ******************************************************************************/

#ifndef _3DICE_THERMAL_DATA_H_
#define _3DICE_THERMAL_DATA_H_

/*! \file thermal_data.h */

#ifdef __cplusplus
extern "C"
{
#endif

/******************************************************************************/

#include <stdio.h>

#include "types.h"

#include "analysis.h"
#include "stack_description.h"
#include "system_matrix.h"
#include "thermal_cell.h"

#include "slu_ddefs.h"

/******************************************************************************/

    /*! \struct ThermalData
     *
     * \brief Structure to collect data to run thermal simulations
     *
     */

    struct ThermalData
    {
        /*! Array containing the temperature of each thermal cell */

        double *Temperatures ;

        /*! Array containing the source value of each thermal cell */

        double *Sources ;

        /*! Array of Thermal cells */

        ThermalCell *ThermalCells ;

        /*! The number of cells in the 3D grid */

        uint32_t    Size ;

        /*! The matrix A representing the linear system for thermal simulation
         *
         * The right hand vector B is represented by the Temperatures array,
         * since the SuperLU routine dgstrs overwrites the B rhs vector with
         * the computed result (temperatures).
         */

        SystemMatrix SM_A ;

        /*! SuperLU matrix A (wrapper arount our SystemMatrix SM_A )*/

        SuperMatrix SLUMatrix_A ;

        /*! SuperLU matrix A after the permutation */

        SuperMatrix SLUMatrix_A_Permuted ;

        /*! SuperLU vector B (wrapper around the Temperatures array) */

        SuperMatrix SLUMatrix_B ;

        /*! SuperLU matrix L after the A=LU factorization */

        SuperMatrix SLUMatrix_L ;

        /*! SuperLU matrix U after the A=LU factorization */

        SuperMatrix SLUMatrix_U ;

        /*! SuperLU structure for statistics */

        SuperLUStat_t     SLU_Stat ;

        /*! SuperLU structure for factorization options */

        superlu_options_t SLU_Options ;

        /*! SuperLU integer to code the result of the SLU routines */

        int  SLU_Info ;

        /*! SuperLU matrix R for permutation RAC = LU. */

        int* SLU_PermutationMatrixR ;

        /*! SuperLU matrix C for permutation RAC = LU. */

        int* SLU_PermutationMatrixC ;

        /*! SuperLU elimination tree */

        int* SLU_Etree ;

    } ;


    /*! Definition of the type ThermalData */

    typedef struct ThermalData ThermalData ;

/******************************************************************************/

    /*! Sets all the fields of \a tdata to a default value (zero or \c NULL )
     *  and configure the SLU fields to run a factorization
     *
     * \param tdata the address of the thermal data to initialize
     */

    void init_thermal_data (ThermalData *tdata) ;



    /*! Allocs and initialize memory and prepares the LU factorization
     *
     * \param tdata the address of the ThermalData to fill
     * \param stkd  the address of the StackDescription previously filled
     *              through the parsing of the stack file
     * \param analysis the address of the Analysis previously filled trough
     *                  the parsing of the stack file
     *
     * \return \c TDICE_FAILURE if the memory allocation fails or the syatem
     *              matrix cannot be split in A=LU.
     * \return \c TDICE_SUCCESS otherwise
     */

    Error_t fill_thermal_data

        (ThermalData* tdata, StackDescription* stkd, Analysis *analysis) ;



    /*! Frees the memory related to \a tdata
     *
     * The parametrer \a tdata must be the address of a static variable
     *
     * \param tdata the address of the ThermalData structure to free
     */

    void free_thermal_data  (ThermalData* tdata) ;



    /*! Simulates a time step
     *
     * \param tdata     address of the ThermalData structure
     * \param stkd      address of the StackDescription structure used to
     *                  fill the content of \a tdata and \a analysys
     * \param analysis  address of the Analysis structure
     *
     * \return \c TDICE_WRONG_CONFIG if the parameters refers to a steady
     *                               state simulation
     * \return \c TDICE_SOLVER_ERROR if the SLU functions report an error in
     *                               the structure of the system matrix.
     * \return \c TDICE_END_OF_SIMULATION if power values are over.
     * \return \c TDICE_STEP_DONE    if the time step has been simulated
     *                               correclty
     * \return \c TDICE_SLOT_DONE    if the time step has been simulated
     *                               correclty and the slot has been completed
     */

    SimResult_t emulate_step

        (ThermalData *tdata, StackDescription *stkd, Analysis *analysis) ;



    /*! Simulates a time slot
     *
     * \param tdata     address of the ThermalData structure
     * \param stkd      address of the StackDescription structure used to
     *                  fill the content of \a tdata and \a analysys
     * \param analysis  address of the Analysis structure
     *
     * \return \c TDICE_WRONG_CONFIG if the parameters refers to a steady
     *                               state simulation
     * \return \c TDICE_SOLVER_ERROR if the SLU functions report an error in
     *                               the structure of the system matrix.
     * \return \c TDICE_END_OF_SIMULATION if power values are over.
     * \return \c TDICE_SLOT_DONE    the slot has been simulated correclty
     */

    SimResult_t emulate_slot

        (ThermalData *tdata, StackDescription *stkd, Analysis *analysis) ;



    /*! Execute steady state simulation
     *
     * \param tdata     address of the ThermalData structure
     * \param stkd      address of the StackDescription structure used to
     *                  fill the content of \a tdata and \a analysys
     * \param analysis  address of the Analysis structure
     *
     * \return \c TDICE_WRONG_CONFIG if the parameters refers to a transient
     *                               simulation
     * \return \c TDICE_SOLVER_ERROR if the SLU functions report an error in
     *                               the structure of the system matrix.
     * \return \c TDICE_END_OF_SIMULATION if no power values are given or if
     *                                    the simulation suceeded
     */

    SimResult_t emulate_steady

        (ThermalData *tdata, StackDescription *stkd, Analysis *analysis) ;



    /*! Update the flow rate
     *
     * Sets the new value in the Channel structure, re-fill the system
     * matrix A and then execute the factorization A=LU again. If this
     * succeeds then the source vector will be upadted with the new inlet
     * source value.
     *
     * \param tdata address of the ThermalData structure
     * \param stkd  address of the StackDescription structure
     * \param new_flow_rate the new flow rate (in ml/min)
     *
     * \return \c TDICE_FAILURE if the syatem matrix cannot be split in A=LU.
     * \return \c TDICE_SUCCESS otherwise
     */

    Error_t update_coolant_flow_rate

        (ThermalData *tdata, StackDescription *stkd, double new_flow_rate) ;

/******************************************************************************/

  /* Get the maximum temperature of a given floorplan element                */
  /*                                                                         */
  /* stkd                  the StackDescription structure to query           */
  /* tdata                 the ThermalData structure to query                */
  /* floorplan_id          the id of the floorplan (the stack element id)    */
  /* floorplan_element_id  the id of the floorplan element                   */
  /* max_temperature       the address where the max temperature will be     */
  /*                       written                                           */
  /*                                                                         */
  /* Returns:                                                                */
  /*                                                                        */
  /*   0 if both the ids are correct                                         */
  /*  -1 if the stack element id floorplan_id does not exist                 */
  /*  -2 if floorplan_id exists but it is not the id of a die or it belongs  */
  /*     to a die but the floorplan itself does not exist                    */
  /*  -3 if floorplan_id exists but floorplan_element_id do not              */

  int get_max_temperature_of_floorplan_element
  (
    StackDescription* stkd,
    ThermalData*      tdata,
    char *          floorplan_id,
    char *          floorplan_element_id,
    double*    max_temperature
  ) ;

/******************************************************************************/

  /* Get the minimum temperature of a given floorplan element                */
  /*                                                                         */
  /* stkd                  the StackDescription structure to query           */
  /* floorplan_id          the id of the floorplan (the stack element id)    */
  /* floorplan_element_id  the id of the floorplan element                   */
  /* temperatures          the address of the array temperature to access    */
  /* min_temperature       the address where the min temperature will be     */
  /*                       written                                           */
  /*                                                                         */
  /* Returns:                                                                */
  /*   0 if both the ids are correct                                         */
  /*  -1 if the stack element id floorplan_id does not exist                 */
  /*  -2 if floorplan_id exists but it is not the id of a die or it belongs  */
  /*     to a die but the floorplan itself does not exist                    */
  /*  -3 if floorplan_id exists but floorplan_element_id do not              */

  int get_min_temperature_of_floorplan_element
  (
    StackDescription* stkd,
    ThermalData*      tdata,
    char *          floorplan_id,
    char *          floorplan_element_id,
    double*    min_temperature
  ) ;

/******************************************************************************/

  /* Get the average temperature of a given floorplan element                */
  /*                                                                         */
  /* stkd                  the StackDescription structure to query           */
  /* floorplan_id          the id of the floorplan (the stack element id)    */
  /* floorplan_element_id  the id of the floorplan element                   */
  /* temperatures          the address of the array temperature to access    */
  /* avg_temperature       the address where the avg temperature will be     */
  /*                       written                                           */
  /*                                                                         */
  /* Returns:                                                                */
  /*   0 if both the ids are correct                                         */
  /*  -1 if the stack element id floorplan_id does not exist                 */
  /*  -2 if floorplan_id exists but it is not the id of a die or it belongs  */
  /*     to a die but the floorplan itself does not exist                    */
  /*  -3 if floorplan_id exists but floorplan_element_id do not              */

  int get_avg_temperature_of_floorplan_element
  (
    StackDescription* stkd,
    ThermalData*      tdata,
    char *          floorplan_id,
    char *          floorplan_element_id,
    double*    avg_temperature
  ) ;

/******************************************************************************/

  /* Get the maximum temperature of all the floorplan element in a floorplan */
  /*                                                                         */
  /* stkd                  the StackDescription structure to query           */
  /* floorplan_id          the id of the floorplan (the stack element id)    */
  /* temperatures          the address of the array temperature to access    */
  /* max_temperature       the address where the max temperatures will be    */
  /*                       written                                           */
  /*                                                                         */
  /* Returns:                                                                */
  /*   0 if the ids is correct                                               */
  /*  -1 if the stack element id floorplan_id does not exist                 */
  /*  -2 if floorplan_id exists but it is not the id of a die or it belongs  */
  /*     to a die but the floorplan itself does not exist                    */
  /*                                                                         */
  /* If the floorplan named floorplan_id has n floorplan element, then the   */
  /* function will access n elements of type double starting from     */
  /* max_temperatures (i.e. the memory must be allocated before calling this */
  /* function). Temperatures will be written following the same order of     */
  /* declaration found in the corresponding .flp file (max_temperature[0] is */
  /* the maximum temperature of the first floorplan element found in the     */
  /* file).                                                                  */

  int get_all_max_temperatures_of_floorplan
  (
    StackDescription* stkd,
    ThermalData*      tdata,
    char *          floorplan_id,
    double*    max_temperature
  ) ;

/******************************************************************************/

  /* Get the minimum temperature of all the floorplan element in a floorplan */
  /*                                                                         */
  /* stkd                  the StackDescription structure to query           */
  /* floorplan_id          the id of the floorplan (the stack element id)    */
  /* temperatures          the address of the array temperature to access    */
  /* min_temperature       the address where the min temperatures will be    */
  /*                       written                                           */
  /*                                                                         */
  /* Returns:                                                                */
  /*   0 if the ids is correct                                               */
  /*  -1 if the stack element id floorplan_id does not exist                 */
  /*  -2 if floorplan_id exists but it is not the id of a die or it belongs  */
  /*     to a die but the Floorplan itself does not exist                    */
  /*                                                                         */
  /* If the floorplan named floorplan_id has n floorplan element, then the   */
  /* function will access n elements of type double starting from     */
  /* min_temperatures (i.e. the memory must be allocated before calling this */
  /* function). Temperatures will be written following the same order of     */
  /* declaration found in the corresponding .flp file (max_temperature[0] is */
  /* the minimum temperature of the first floorplan element found in the     */
  /* file).                                                                  */

  int get_all_min_temperatures_of_floorplan
  (
    StackDescription* stkd,
    ThermalData*      tdata,
    char *          floorplan_id,
    double*    min_temperature
  ) ;

/******************************************************************************/

  /* Get the average temperature of all the floorplan element in a floorplan */
  /*                                                                         */
  /* stkd                  the StackDescription structure to query           */
  /* floorplan_id          the id of the floorplan (the stack element id)    */
  /* temperatures          the address of the array temperature to access    */
  /* avg_temperature       the address where the avg temperatures will be    */
  /*                       written                                           */
  /*                                                                         */
  /* Returns:                                                                */
  /*   0 if the ids is correct                                               */
  /*  -1 if the stack element id floorplan_id does not exist                 */
  /*  -2 if floorplan_id exists but it is not the id of a die or it belongs  */
  /*     to a die but the Floorplan itself does not exist                    */
  /*                                                                         */
  /* If the floorplan named floorplan_id has n floorplan element, then the   */
  /* function will access n elements of type double starting from     */
  /* avg_temperatures (i.e. the memory must be allocated before calling this */
  /* function). Temperatures will be written following the same order of     */
  /* declaration found in the corresponding .flp file (max_temperature[0] is */
  /* the average temperature of the first floorplan element found in the     */
  /* file).                                                                  */

  int get_all_avg_temperatures_of_floorplan
  (
    StackDescription* stkd,
    ThermalData*      tdata,
    char *          floorplan_id,
    double*    avg_temperature
  ) ;

/******************************************************************************/

  /* Get the themperature of a given thermal cell                            */
  /*                                                                         */
  /* Returns:                                                                */
  /*   0 if the cell exists                                                  */
  /*  -1 if the cell does not exist                                          */
  /*  -2 if there has been an error when opening the file file_name          */

  int get_cell_temperature
  (
    StackDescription* stkd,
    ThermalData*      tdata,
    uint32_t   layer_index,
    uint32_t   row_index,
    uint32_t   column_index,
    double*    cell_temperature
  ) ;

/******************************************************************************/

  /* Given the address of a StackDescrption structure and the array of       */
  /* temperatures, print to file_name the thermal map of stack_element_id.   */
  /* stack_element_id is the id given to one of the stack element composing  */
  /* the 3D stack. If it refers to a die, the active source layer will be    */
  /* printed.                                                                */
  /*                                                                         */
  /* Returns:                                                                */
  /*   0 if the ids is correct                                               */
  /*  -1 if the stack element id floorplan_id does not exist                 */
  /*  -2 if there has been an error when opening the file file_name          */
  /*                                                                         */
  /* The thermal map is printex as a matrix with get_number_of_columns ()    */
  /* columns and get_number_of_rows () rows.                                 */

  int print_thermal_map
  (
    StackDescription* stkd,
    ThermalData*      tdata,
    char *          stack_element_id,
    char *          file_name
  ) ;

/******************************************************************************/

#ifdef __cplusplus
}
#endif

#endif /* _3DICE_THERMAL_DATA_H_ */
