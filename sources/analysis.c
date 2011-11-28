/******************************************************************************
 * This file is part of 3D-ICE, version 1.0.1 .                               *
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

#include <stdlib.h>
#include <string.h>

#include "analysis.h"
#include "macros.h"

/******************************************************************************/

void init_analysis (Analysis *analysis)
{
    analysis->AnalysisType         = ANALYSISTYPE_I;
    analysis->StepTime             = 0.0 ;
    analysis->SlotTime             = 0.0 ;
    analysis->SlotLength           = 0u ;
    analysis->CurrentTime          = 0u ;
    analysis->InitialTemperature   = 0.0 ;
    analysis->InspectionPointListFinal = NULL ;
    analysis->InspectionPointListSlot  = NULL ;
    analysis->InspectionPointListStep  = NULL ;
}

/******************************************************************************/

Analysis *alloc_and_init_analysis (void)
{
    Analysis *analysis = (Analysis *) malloc (sizeof(Analysis)) ;

    if (analysis != NULL)

        init_analysis (analysis) ;

    return analysis ;
}

/******************************************************************************/

void free_analysis (Analysis *analysis)
{
    FREE_LIST (InspectionPoint, analysis->InspectionPointListFinal, free_inspection_point) ;
    FREE_LIST (InspectionPoint, analysis->InspectionPointListSlot, free_inspection_point) ;
    FREE_LIST (InspectionPoint, analysis->InspectionPointListStep, free_inspection_point) ;
}

/******************************************************************************/

double get_simulated_time (Analysis *analysis)
{
  return analysis->CurrentTime * analysis->StepTime ;
}

/******************************************************************************/

bool slot_completed (Analysis *analysis)
{
  if (analysis->AnalysisType == TDICE_STEADY)

    return false ;

  if (analysis->CurrentTime % analysis->SlotLength == 0)

    return true ;

  return false ;
}

/******************************************************************************/

void print_formatted_analysis
(
    FILE     *stream,
    char     *prefix,
    Analysis *analysis
)
{
    fprintf (stream, "%ssolver : ", prefix) ;

    if (analysis->AnalysisType == TDICE_STEADY)

        fprintf (stream, "steady ;\n") ;

    else

        fprintf (stream, "transient step %.2f, slot %.2f ;\n",
            analysis->StepTime, analysis->SlotTime) ;

    fprintf (stream, "%s\n", prefix) ;

    fprintf (stream, "%sinitial temperature  %.2f ;\n",
        prefix, analysis->InitialTemperature) ;

    fprintf (stream, "%s\n", prefix) ;

    fprintf (stream, "%soutput :\n", prefix) ;

    fprintf (stream, "%s\n", prefix) ;

    print_formatted_inspection_point_list

        (stream, prefix, analysis->InspectionPointListFinal) ;

    fprintf (stream, "%s\n", prefix) ;

    print_formatted_inspection_point_list

        (stream, prefix, analysis->InspectionPointListSlot) ;

    fprintf (stream, "%s\n", prefix) ;

    print_formatted_inspection_point_list

        (stream, prefix, analysis->InspectionPointListStep) ;

    fprintf (stream, "%s\n", prefix) ;
}

/******************************************************************************/

void print_detailed_analysis
(
  FILE     *stream,
  char     *prefix,
  Analysis *analysis
)
{
    char *new_prefix = malloc (sizeof(char) * (5 + strlen(prefix))) ;

    if (new_prefix == NULL) return ;

    sprintf (new_prefix, "%s    ", prefix) ;

    fprintf (stream,
        "%sAnalysis                        = %p\n",
        prefix, analysis) ;

    fprintf (stream,
        "%s  AnalysisType                  = %d\n",
        prefix, analysis->AnalysisType) ;

    fprintf (stream,
        "%s  StepTime                      = %.2f\n",
        prefix, analysis->StepTime) ;

    fprintf (stream,
        "%s  SlotTime                      = %.2f\n",
        prefix, analysis->SlotTime) ;

    fprintf (stream,
        "%s  SlotLength                    = %d\n",
        prefix, analysis->SlotLength) ;

    fprintf (stream,
        "%s  CurrentTime                   = %d\n",
        prefix, analysis->CurrentTime) ;

    fprintf (stream,
        "%s  InitialTemperature            = %.2f\n",
        prefix, analysis->InitialTemperature) ;

    fprintf (stream,
        "%s  InspectionPointListFinal          = %p\n",
        prefix, analysis->InspectionPointListFinal) ;

    if (analysis->InspectionPointListFinal != NULL)
    {
        fprintf (stream, "%s\n", prefix) ;

        print_detailed_inspection_point_list

            (stream, new_prefix, analysis->InspectionPointListFinal) ;

        fprintf (stream, "%s\n", prefix) ;
    }

    fprintf (stream,
        "%s  InspectionPointListSlot           = %p\n",
        prefix, analysis->InspectionPointListSlot) ;

    if (analysis->InspectionPointListSlot != NULL)
    {
        fprintf (stream, "%s\n", prefix) ;

        print_detailed_inspection_point_list

            (stream, new_prefix, analysis->InspectionPointListSlot) ;

        fprintf (stream, "%s\n", prefix) ;
    }

    fprintf (stream,
        "%s  InspectionPointListStep           = %p\n",
        prefix, analysis->InspectionPointListStep) ;

    if (analysis->InspectionPointListStep != NULL)
    {
        fprintf (stream, "%s\n", prefix) ;

        print_detailed_inspection_point_list

            (stream, new_prefix, analysis->InspectionPointListStep) ;

        fprintf (stream, "%s\n", prefix) ;
    }

    FREE_POINTER (free, new_prefix) ;
}

/******************************************************************************/

void add_inspection_point_to_analysis
(
    Analysis        *analysis,
    InspectionPoint *inspection_point
)
{
    InspectionPoint **list = NULL ;

    if (   analysis->AnalysisType == TDICE_STEADY
        || inspection_point->InstanceType == TDICE_OUTPUT_FINAL)

        list = &analysis->InspectionPointListFinal ;

    else if (inspection_point->InstanceType==TDICE_OUTPUT_SLOT)

        list = &analysis->InspectionPointListSlot ;

    else if (inspection_point->InstanceType==TDICE_OUTPUT_STEP)

        list = &analysis->InspectionPointListStep ;

    while (*list != NULL) list = &( (*list)->Next ) ;

    *list = inspection_point ;

    // CHECKME any warning when using the same filename twice?
}

/******************************************************************************/

Error_t generate_analysis_headers
(
    Analysis   *analysis,
    Dimensions *dimensions,
    char       *prefix
)
{
    FOR_EVERY_ELEMENT_IN_LIST_FORWARD (InspectionPoint, final, analysis->InspectionPointListFinal)

        if (generate_inspection_point_header (final, dimensions, prefix) != TDICE_SUCCESS)

            return TDICE_FAILURE ;

    FOR_EVERY_ELEMENT_IN_LIST_FORWARD (InspectionPoint, slot, analysis->InspectionPointListSlot)

        if (generate_inspection_point_header (slot, dimensions, prefix) != TDICE_SUCCESS)

            return TDICE_FAILURE ;

    FOR_EVERY_ELEMENT_IN_LIST_FORWARD (InspectionPoint, step, analysis->InspectionPointListStep)

        if (generate_inspection_point_header (step, dimensions, prefix) != TDICE_SUCCESS)

            return TDICE_FAILURE ;

    return TDICE_SUCCESS ;
}

/******************************************************************************/

Error_t generate_analysis_output
(
    Analysis             *analysis,
    Dimensions           *dimensions,
    double               *temperatures,
    OutputInstanceType_t  output_instance
)
{
    double current_time = get_simulated_time (analysis) ;

    InspectionPoint *list ;

    if (output_instance == TDICE_OUTPUT_FINAL)

        list = analysis->InspectionPointListFinal ;

    else if (output_instance == TDICE_OUTPUT_STEP)

        list = analysis->InspectionPointListStep ;

    else if (output_instance == TDICE_OUTPUT_FINAL)

        list = analysis->InspectionPointListSlot ;

    else

        return EXIT_FAILURE ;

    FOR_EVERY_ELEMENT_IN_LIST_FORWARD (InspectionPoint, ipoint, list)

        if (generate_inspection_point_output (ipoint, dimensions, temperatures, current_time) != TDICE_SUCCESS)

            return TDICE_FAILURE ;

   return TDICE_SUCCESS ;
}

/******************************************************************************/
