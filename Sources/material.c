/******************************************************************************
 *                                                                            *
 * Source file "Sources/material.c"                                           *
 *                                                                            *
 * EPFL-STI-IEL-ESL                                                           *
 * Bâtiment ELG, ELG 130                                                      *
 * Station 11                                                                 *
 * 1015 Lausanne, Switzerland                    alessandro.vincenzi@epfl.ch  *
 ******************************************************************************/

#include <stdlib.h>
#include <string.h>

#include "material.h"

void
init_material (Material * material)
{
  if (material == NULL) return ;

  material->Id                  = NULL ;
  material->SpecificHeat        = 0.0 ;
  material->ThermalConductivity = 0.0 ;
  material->Next                = NULL ;
}

/******************************************************************************/
/******************************************************************************/
/******************************************************************************/

Material *
alloc_and_init_material (void)
{
  Material *material = (Material *) malloc (sizeof (Material)) ;

  init_material (material) ;

  return material ;
}

/******************************************************************************/
/******************************************************************************/
/******************************************************************************/

void
free_material (Material * material)
{
  if (material == NULL) return ;

  free (material->Id) ;
  free (material) ;
}

/******************************************************************************/
/******************************************************************************/
/******************************************************************************/

void
free_materials_list (Material * list)
{
  Material *next_material ;

  for (; list != NULL; list = next_material)
  {
      next_material = list->Next ;

      free_material (list) ;
  }
}

/******************************************************************************/
/******************************************************************************/
/******************************************************************************/

void
print_material (FILE * stream, char *prefix, Material * material)
{
  fprintf (stream,
           "%sMaterial %s:\n",                prefix,
                                              material->Id) ;
  fprintf (stream,
           "%s  Specific Heat        %.4e\n", prefix,
                                              material->SpecificHeat) ;
  fprintf (stream,
           "%s  Thermal Conductivity %.4e\n", prefix,
                                              material->ThermalConductivity) ;
}

/******************************************************************************/
/******************************************************************************/
/******************************************************************************/

void
print_materials_list (FILE * stream, char *prefix, Material * list)
{
  for (; list != NULL; list = list->Next)
  {
    print_material (stream, prefix, list) ;
  }
}

/******************************************************************************/
/******************************************************************************/
/******************************************************************************/

Material *
find_material_in_list (Material * list, char *id)
{
  for (; list != NULL; list = list->Next)
  {
    if (strcmp (list->Id, id) == 0)
    {
      break ;
    }
  }

  return list ;
}
