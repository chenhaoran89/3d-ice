/******************************************************************************
 * Source file "3D-ICe/sources/powers_queue.c"                                *
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

#include "powers_queue.h"

/******************************************************************************/

void init_powers_queue (PowersQueue* queue)
{
  queue->Head = NULL ;
  queue->Tail = NULL ;
  queue->Length = 0 ;
}

/******************************************************************************/

PowersQueue* alloc_and_init_powers_queue (void)
{
  PowersQueue* queue = (PowersQueue*) malloc (sizeof(PowersQueue)) ;

  if (queue != NULL)  init_powers_queue (queue) ;

  return queue ;
}

/******************************************************************************/

Bool_t is_empty_powers_queue (PowersQueue* queue)
{
  return (queue->Length == 0) ;
}

/******************************************************************************/

void free_powers_queue (PowersQueue* queue)
{
  while (! is_empty_powers_queue(queue) )

    pop_from_powers_queue(queue) ;

  free (queue) ;
}

/******************************************************************************/

void put_into_powers_queue (PowersQueue* queue, Power_t power)
{
  PowerNode* tmp = queue->Tail ;

  queue->Tail = (PowerNode*) malloc ( sizeof(PowerNode) ) ;

  if ( queue->Tail == NULL )
  {
    fprintf (stderr, "malloc power node error !!\n") ;
    return ;
  }

  queue->Tail->Value = power ;
  queue->Tail->Next  = NULL ;

  if (queue->Head == NULL)

    queue->Head = queue->Tail ;

  else

    tmp->Next = queue->Tail ;

  queue->Length++;
}

/******************************************************************************/

Power_t get_from_powers_queue (PowersQueue* queue)
{
  return queue->Head != NULL ? queue->Head->Value : (Power_t) 0 ; // FIXME
}

/******************************************************************************/

void pop_from_powers_queue (PowersQueue* queue)
{
  PowerNode* tmp = queue->Head->Next ;

  free(queue->Head) ;

  queue->Head = tmp ;

  queue->Length--;
}

/******************************************************************************/

void print_powers_queue (FILE* stream, String_t prefix, PowersQueue* queue)
{
  PowerNode* tmp;
  fprintf(stream, "%s [%d] ", prefix, queue->Length);
  for (tmp = queue->Head ; tmp != NULL ; tmp = tmp->Next)
    fprintf(stream, "%6.4f ", tmp->Value) ;
  fprintf(stream, "\n");
}

/******************************************************************************/

void print_formatted_powers_queue (FILE* stream, PowersQueue* queue)
{
  PowerNode* tmp;
  for (tmp = queue->Head ; tmp != NULL ; tmp = tmp->Next)
    fprintf(stream, "%6.4f ", tmp->Value) ;
}

/******************************************************************************/
