/******************************************************************************
 * This file is part of 3D-ICE, version 2.2.7 .                               *
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
 *          Giseong Bak                                                       *
 *          Martino Ruggiero                                                  *
 *          Thomas Brunschwiler                                               *
 *          David Atienza                                                     *
 *                                                                            *
 * For any comment, suggestion or request  about 3D-ICE, please  register and *
 * write to the mailing list (see http://listes.epfl.ch/doc.cgi?liste=3d-ice) *
 * Any usage  of 3D-ICE  for research,  commercial or other  purposes must be *
 * properly acknowledged in the resulting products or publications.           *
 *                                                                            *
 * EPFL-STI-IEL-ESL                                                           *
 * Batiment ELG, ELG 130                Mail : 3d-ice@listes.epfl.ch          *
 * Station 11                                  (SUBSCRIPTION IS NECESSARY)    *
 * 1015 Lausanne, Switzerland           Url  : http://esl.epfl.ch/3d-ice.html *
 ******************************************************************************/

#include <string.h> // For the string function strcpy
#include <stdlib.h> // For the memory function free

#include "string_t.h"

/******************************************************************************/

void string_init (String_t *string)
{
    *string = NULL ;
}

/******************************************************************************/

void string_copy (String_t *dst, String_t *src)
{
    string_destroy (dst) ;

    *dst = *src == NULL ? NULL : strdup (*src) ;
}

/******************************************************************************/

void string_copy_cstr (String_t *dst, char *src)
{
    string_destroy (dst) ;

    *dst = strdup (src) ;
}

/******************************************************************************/

bool string_equal (String_t *string, String_t *other)
{
   return strcmp (*string, *other) == 0 ? true : false ;
}

/******************************************************************************/

void string_destroy (String_t *string)
{
    if (*string != NULL)

        free (*string) ;

    string_init (string) ;
}

/******************************************************************************/
