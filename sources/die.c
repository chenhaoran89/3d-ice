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

#include "die.h"
#include "macros.h"

/******************************************************************************/

void init_die (Die* die)
{
  die->Id          = STRING_I ;
  die->Used        = QUANTITY_I ;
  die->NLayers     = GRIDDIMENSION_I ;
  die->LayersList  = NULL ;
  die->SourceLayer = NULL ;
  die->Next        = NULL ;
}

/******************************************************************************/

Die* alloc_and_init_die (void)
{
  Die* die = (Die*) malloc ( sizeof(Die) ) ;

  if (die != NULL) init_die (die) ;

  return die ;
}

/******************************************************************************/

void free_die (Die* die)
{
  free_layers_list (die->LayersList) ;
  free (die->Id) ;
  free (die) ;
}

/******************************************************************************/

void free_dies_list (Die* list)
{
  FREE_LIST (Die, list, free_die) ;
}

/******************************************************************************/

void print_die (FILE* stream, String_t prefix, Die* die)
{
  fprintf (stream,
    "%sDie %s:\n",                prefix, die->Id) ;
  fprintf (stream,
    "%s  Number of layers  %d\n", prefix, die->NLayers) ;

  fprintf (stream,
    "%s  Source layer is layer #%d\n", prefix, die->SourceLayer->Offset) ;

  String_t new_prefix = (String_t) malloc (sizeof(char)*(strlen(prefix) + 2)) ;
  // FIXME typeof(pointed by string)

  strcpy (new_prefix, prefix) ;
  strcat (new_prefix, "  ") ;

  print_layers_list (stream, new_prefix, die->LayersList) ;

  free (new_prefix) ;
}

/******************************************************************************/

void print_dies_list (FILE* stream, String_t prefix, Die* list)
{
  FOR_EVERY_ELEMENT_IN_LIST (Die, die, list)

    print_die (stream, prefix, die) ;
}

/******************************************************************************/

Die* find_die_in_list (Die* list, String_t id)
{
  FOR_EVERY_ELEMENT_IN_LIST (Die, die, list)

    if (strcmp(die->Id, id) == 0) break ;

  return die ;
}

/******************************************************************************/

void fill_thermal_grid_data_die
(
  ThermalGridData* thermalgriddata,
  GridDimension_t  layer_index,
  Die*             die
)
{
# ifdef PRINT_THERMAL_GRID_DATA
  fprintf (stderr, "\n#%d\tDie     [%s]\n\n", layer_index, die->Id) ;
# endif

  FOR_EVERY_ELEMENT_IN_LIST (Layer, layer, die->LayersList)

    fill_thermal_grid_data_layer
    (
      thermalgriddata,
      layer_index++,
      layer
    ) ;
}

/******************************************************************************/

Source_t* fill_sources_die
(
  GridDimension_t       layer_index,
  Die*                  die,
  ConventionalHeatSink* conventionalheatsink,
  ThermalGridData*      thermalgriddata,
  Floorplan*            floorplan,
  Source_t*             sources,
  Dimensions*           dimensions
)
{
#ifdef PRINT_SOURCES
  fprintf (stderr,
    "layer_index = %d\tfill_sources_die %s floorplan %s\n",
    layer_index, die->Id, floorplan->FileName) ;
#endif

  FOR_EVERY_ELEMENT_IN_LIST (Layer, layer, die->LayersList)
  {
    if ( die->SourceLayer == layer )

      sources = fill_sources_active_layer
                (
#                 ifdef PRINT_SOURCES
                  layer,
#                 endif
                  layer_index + layer->Offset,
                  conventionalheatsink,
                  thermalgriddata,
                  floorplan,
                  sources,
                  dimensions
                ) ;

    else

      sources = fill_sources_empty_layer
                (
#                 ifdef PRINT_SOURCES
                  layer,
#                 endif
                  layer_index + layer->Offset,
                  conventionalheatsink,
                  thermalgriddata,
                  sources,
                  dimensions
                ) ;

  } // FOR_EVERY_ELEMENT_IN_LIST

  return sources ;
}

/******************************************************************************/

SystemMatrix fill_system_matrix_die
(
  Die*                  die,
  Dimensions*           dimensions,
  ThermalGridData*      thermalgriddata,
  GridDimension_t       layer_index,
  SystemMatrix          system_matrix
)
{
# ifdef PRINT_SYSTEM_MATRIX
  fprintf (stderr, "(l %2d) fill_system_matrix_die\n", layer_index) ;
# endif

  FOR_EVERY_ELEMENT_IN_LIST (Layer, layer, die->LayersList)
  {
    system_matrix = fill_system_matrix_layer
                    (
#                     ifdef PRINT_SYSTEM_MATRIX
                      layer,
#                     endif
                      dimensions, thermalgriddata,
                      layer_index + layer->Offset,
                      system_matrix
                     ) ;

  }  // FOR_EVERY_ELEMENT_IN_LIST

  return system_matrix ;
}

/******************************************************************************/