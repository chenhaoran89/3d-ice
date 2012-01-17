/******************************************************************************
 * This file is part of 3D-ICE, version 2.0 .                                 *
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

#include <string.h>

#include "macros.h"
#include "network_message.h"

/******************************************************************************/

void init_network_message (NetworkMessage *message)
{
    memset (message->Memory, 0u, MAX_LENGTH * sizeof(MessageWord_t)) ;

    message->MaxLength = MAX_LENGTH ;

    message->Length    = message->Memory ;

    message->Type      = message->Length + 1u ;

    message->Content   = message->Type   + 1u ;
}

/******************************************************************************/

void build_message_head (NetworkMessage *message, MessageType_t type)
{
    *message->Length = (MessageWord_t) 2u ;

    *message->Type   = (MessageWord_t) type ;
}

/******************************************************************************/

#include <stdio.h>

Error_t insert_message_word
(
    NetworkMessage *message,
    void           *word
)
{
    if (*message->Length == message->MaxLength)

        return TDICE_FAILURE ;

    MessageWord_t *toinsert = message->Memory + *message->Length ;

    memcpy (toinsert, word, sizeof (MessageWord_t)) ;

    (*message->Length)++ ;

    return TDICE_SUCCESS ;
}

/******************************************************************************/

Error_t extract_message_word
(
    NetworkMessage *message,
    void           *word,
    Quantity_t      index
)
{
    if (index >= (message->MaxLength - 2))

        return TDICE_FAILURE ;

    memcpy (word, message->Content + index, sizeof (MessageWord_t)) ;

    return TDICE_SUCCESS ;
}
