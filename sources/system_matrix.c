/******************************************************************************
 * Source file "3D-ICe/sources/system_matrix.c"                               *
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

#include "conductances.h"
#include "system_matrix.h"

#define PARALLEL(x,y)      ( (x * y) / ( x + y) )
#define LAYER_OFFSET(dim)  (dim->Grid.NRows * dim->Grid.NColumns)
#define ROW_OFFSET(dim)    (dim->Grid.NColumns)
#define COLUMN_OFFSET(dim) (1)

/******************************************************************************/

int alloc_system_matrix
(
  SystemMatrix*   matrix,
  enum MatrixStorage_t storage,
  Quantity_t      nvalues,
  int             nnz
)
{
  matrix->Size    = nvalues ;
  matrix->NNz     = nnz ;
  matrix->Storage = storage ;

  if (storage == TL_CCS_MATRIX)
  {
    matrix->columns_size = nvalues + 1 ;
    matrix->rows_size    = nnz ;
  }
  else if (storage == TL_CRS_MATRIX)
  {
    matrix->columns_size = nnz ;
    matrix->rows_size    = nvalues + 1 ;
  }
  else
  {
    fprintf (stderr, "Matrix storage type %d not supported\n", storage) ;
    return 0 ;
  }

  matrix->Columns = (int*) malloc (sizeof(int) * matrix->columns_size ) ;

  if (matrix->Columns == NULL)
  {
    return 0 ;
  }

  matrix->Rows = (int*) malloc (sizeof(int) * matrix->rows_size ) ;

  if (matrix->Rows == NULL)
  {
    free (matrix->Columns) ;
    return 0 ;
  }

  matrix->Values = (double*) malloc (sizeof(double) * nnz ) ;

  if (matrix->Values == NULL)
  {
    free (matrix->Columns) ;
    free (matrix->Rows) ;
    return 0 ;
  }

  return 1 ;
}

/******************************************************************************/

void free_system_matrix (SystemMatrix* matrix)
{
  free (matrix->Columns) ;
  free (matrix->Rows) ;
  free (matrix->Values) ;
}

/******************************************************************************/

void fill_system_matrix
(
  StackDescription* stkd,
  SystemMatrix*     matrix,
  Conductances*     conductances,
  Capacity_t*       capacities
)
{
  if (matrix->Storage == TL_CCS_MATRIX)

    fill_ccs_system_matrix_stack_description
    (
      stkd,
      conductances,
      capacities,
      matrix->Columns,
      matrix->Rows,
      matrix->Values
    ) ;

  else if (matrix->Storage == TL_CRS_MATRIX)

    fill_crs_system_matrix_stack_description
    (
      stkd,
      conductances,
      capacities,
      matrix->Rows,
      matrix->Columns,
      matrix->Values
    ) ;

  else

    fprintf (stderr, "Unsupported matrix type\n") ;
}

/******************************************************************************/

int                    add_ccs_solid_column
(
  Dimensions*          dimensions,
  Conductances* conductances,
  Capacity_t*          capacities,
  LayerIndex_t         current_layer,
  RowIndex_t           current_row,
  ColumnIndex_t        current_column,
  int*                 columns,
  int*                 rows,
  double*              values
)
{
  double  conductance        = 0.0 ;
  double  diagonal_value    = 0.0 ;
  double* diagonal_pointer = NULL ;
  int     added             = 0 ;

  int current_cell
    = current_layer * LAYER_OFFSET(dimensions)
      + current_row * ROW_OFFSET(dimensions)
      + current_column ;

#ifdef PRINT_SYSTEM_MATRIX
  fpos_t diag_fposition, last_fpos ;
  fprintf
  (
    stderr,
    "add_ccs_solid_column  (l %2d r %5d c %5d) -> %5d\n",
    current_layer, current_row, current_column, current_cell
  ) ;
#endif

  *columns = *(columns - 1) ;

  if ( current_layer > 0 )   /* BOTTOM */
  {
    *rows++ = current_cell - LAYER_OFFSET(dimensions) ;

    conductance = PARALLEL (conductances->Bottom,
                           (conductances - LAYER_OFFSET(dimensions))->Top) ;

    *values++       = -conductance ;
    diagonal_value +=  conductance ;

    (*columns)++ ;
    added++;

#ifdef PRINT_SYSTEM_MATRIX
    fprintf
    (
      stderr,
      "  bottom  \t%d\t%.5e = %.5e || %.5e\n",
      *(rows-1), *(values-1),
      conductances->Bottom, (conductances - LAYER_OFFSET(dimensions))->Top
    ) ;
#endif
  }

  if ( current_row > 0 )   /* SOUTH */
  {
    *rows++ = current_cell - ROW_OFFSET(dimensions) ;

    conductance = PARALLEL (conductances->South,
                           (conductances - ROW_OFFSET(dimensions))->North) ;

    *values++       = -conductance ;
    diagonal_value +=  conductance ;

    (*columns)++ ;
    added++ ;

#ifdef PRINT_SYSTEM_MATRIX
    fprintf (stderr,
      "  south   \t%d\t%.5e = %.5e || %.5e\n",
      *(rows-1), *(values-1),
      conductances->South, (conductances - ROW_OFFSET(dimensions))->North) ;
#endif
  }

  if ( current_column > 0 )   /* WEST */
  {
    *rows++ = current_cell - COLUMN_OFFSET(dimensions) ;

    conductance = PARALLEL (conductances->West,
                           (conductances - COLUMN_OFFSET(dimensions))->East) ;

    *values++       = -conductance ;
    diagonal_value +=  conductance ;

    (*columns)++ ;
    added++ ;

#ifdef PRINT_SYSTEM_MATRIX
    fprintf (stderr,
      "  west    \t%d\t%.5e = %.5e || %.5e\n",
      *(rows-1), *(values-1),
      conductances->West, (conductances - COLUMN_OFFSET(dimensions))->East) ;
#endif
    }

  /* DIAGONAL */

  *rows++          = current_cell ;
  *values          = *capacities ;
  diagonal_pointer = values++ ;

  (*columns)++ ;
  added ++ ;

#ifdef PRINT_SYSTEM_MATRIX
  fprintf (stderr, "  diagonal\t%d\t ", *(rows-1)) ;
  fgetpos (stderr, &diag_fposition) ;
  fprintf (stderr, "           \n") ;
#endif

  if ( current_column < dimensions->Grid.NColumns - 1 )   /* EAST */
  {
    *rows++ = current_cell + COLUMN_OFFSET(dimensions) ;

    conductance = PARALLEL (conductances->East,
                           (conductances + COLUMN_OFFSET(dimensions))->West) ;

    *values++       = -conductance ;
    diagonal_value +=  conductance ;

    (*columns)++ ;
    added ++ ;

#ifdef PRINT_SYSTEM_MATRIX
    fprintf (stderr,
      "  east    \t%d\t%.5e = %.5e || %.5e\n",
      *(rows-1), *(values-1),
      conductances->East, (conductances + COLUMN_OFFSET(dimensions))->West) ;
#endif
  }

  if ( current_row < dimensions->Grid.NRows - 1 )   /* NORTH */
  {
    *rows++ = current_cell + ROW_OFFSET(dimensions) ;

    conductance = PARALLEL (conductances->North,
                           (conductances + ROW_OFFSET(dimensions))->South) ;

    *values++       = -conductance ;
    diagonal_value +=  conductance ;

    (*columns)++ ;
    added++ ;

#ifdef PRINT_SYSTEM_MATRIX
    fprintf (stderr,
      "  north   \t%d\t%.5e = %.5e || %.5e\n",
      *(rows-1), *(values-1),
      conductances->North, (conductances + ROW_OFFSET(dimensions))->South) ;
#endif
  }

  if ( current_layer < dimensions->Grid.NLayers - 1) /* TOP */
  {
    *rows++ = current_cell + LAYER_OFFSET(dimensions) ;

    conductance = PARALLEL (conductances->Top,
                           (conductances + LAYER_OFFSET(dimensions))->Bottom) ;

    *values++       = -conductance ;
    diagonal_value +=  conductance ;

    (*columns)++ ;
    added++ ;

#ifdef PRINT_SYSTEM_MATRIX
    fprintf (stderr,
      "  top     \t%d\t%.5e = %.5e || %.5e\n",
      *(rows-1), *(values-1),
      conductances->Top, (conductances + LAYER_OFFSET(dimensions))->Bottom) ;
#endif
  }

  /* DIAGONAL ELEMENT */

  *diagonal_pointer += diagonal_value ;

#ifdef PRINT_SYSTEM_MATRIX
  fgetpos (stderr, &last_fpos) ;
  fsetpos (stderr, &diag_fposition) ;
  fprintf (stderr, "%.5e", *diagonal_pointer) ;
  fsetpos (stderr, &last_fpos) ;

  fprintf (stderr, "  %d (+%d)\n", *columns, added) ;
#endif

  return added ;
}

/******************************************************************************/

int                    add_crs_solid_column
(
  Dimensions*          dimensions,
  Conductances* conductances,
  Capacity_t*          capacities,
  EnvironmentHeatSink* environmentheatsink,
  LayerIndex_t         current_layer,
  RowIndex_t           current_row,
  ColumnIndex_t        current_column,
  int*                 rows,
  int*                 columns,
  double*              values
)
{
  double  conductance        = 0.0 ;
  double  diagonal_value    = 0.0 ;
  double* diagonal_pointer = NULL ;
  int     added             = 0 ;

  int current_cell
    = current_layer * LAYER_OFFSET(dimensions)
      + current_row * ROW_OFFSET(dimensions)
      + current_column ;

#ifdef PRINT_SYSTEM_MATRIX
  fpos_t diag_fposition, last_fpos ;
  fprintf (stderr,
    "add_crs_solid_column  (l %2d r %5d c %5d) -> %5d\n",
    current_layer, current_row, current_column, current_cell) ;
#endif

  *rows = *(rows - 1) ;

  if ( current_layer < dimensions->Grid.NLayers - 1) /* TOP */
  {
    *columns++ = current_cell + LAYER_OFFSET(dimensions) ;

    conductance = PARALLEL (conductances->Top,
                           (conductances + LAYER_OFFSET(dimensions))->Bottom) ;

    *values++       = -conductance ;
    diagonal_value +=  conductance ;

    (*rows)++ ;
    added++ ;

#ifdef PRINT_SYSTEM_MATRIX
    fprintf (stderr,
      "  top     \t%d\t%.5e = %.5e || %.5e\n",
      *(columns-1), *(values-1),
      conductances->Top, (conductances + LAYER_OFFSET(dimensions))->Bottom) ;
#endif
  }
  else
  {
    if (environmentheatsink != NULL)

      diagonal_value += conductances->Top ;
  }

  if ( current_row < dimensions->Grid.NRows - 1 )   /* NORTH */
  {
    *columns++ = current_cell + ROW_OFFSET(dimensions) ;

    conductance = PARALLEL (conductances->North,
                           (conductances + ROW_OFFSET(dimensions))->South) ;

    *values++       = -conductance ;
    diagonal_value +=  conductance ;

    (*rows)++ ;
    added++ ;

#ifdef PRINT_SYSTEM_MATRIX
    fprintf (stderr,
      "  north   \t%d\t%.5e = %.5e || %.5e\n",
      *(columns-1), *(values-1),
      conductances->North, (conductances + ROW_OFFSET(dimensions))->South) ;
#endif
  }

  if ( current_column < dimensions->Grid.NColumns - 1 )   /* EAST */
  {
    *columns++ = current_cell + COLUMN_OFFSET(dimensions) ;

    conductance = PARALLEL (conductances->East,
                           (conductances + COLUMN_OFFSET(dimensions))->West) ;

    *values++       = -conductance ;
    diagonal_value +=  conductance ;

    (*rows)++ ;
    added ++ ;

#ifdef PRINT_SYSTEM_MATRIX
    fprintf (stderr,
      "  east    \t%d\t%.5e = %.5e || %.5e\n",
      *(columns-1), *(values-1),
      conductances->East, (conductances + COLUMN_OFFSET(dimensions))->West) ;
#endif
  }

  /* DIAGONAL */

  *columns++       = current_cell ;
  *values          = *capacities ;
  diagonal_pointer = values++ ;

  (*rows)++ ;
  added ++ ;

#ifdef stderr_SYSTEM_MATRIX
  fprintf (stderr, "  diagonal\t%d\t ", *(columns-1)) ;
  fgetpos (stderr, &diag_fposition) ;
  if (environmentheatsink != NULL)
    fprintf (stderr, "           + ehtc %.5e\n", conductances->Top) ;
  else
    fprintf (stderr, "           \n") ;
#endif

  if ( current_column > 0 )   /* WEST */
  {
    *columns++ = current_cell - COLUMN_OFFSET(dimensions) ;

    conductance = PARALLEL (conductances->West,
                           (conductances - COLUMN_OFFSET(dimensions))->East) ;

    *values++       = -conductance ;
    diagonal_value +=  conductance ;

    (*rows)++ ;
    added++ ;

#ifdef PRINT_SYSTEM_MATRIX
    fprintf (stderr,
      "  west    \t%d\t%.5e = %.5e || %.5e\n",
      *(columns-1), *(values-1),
      conductances->West, (conductances - COLUMN_OFFSET(dimensions))->East) ;
#endif
  }

  if ( current_row > 0 )   /* SOUTH */
  {
    *columns++ = current_cell - ROW_OFFSET(dimensions) ;

    conductance = PARALLEL (conductances->South,
                           (conductances - ROW_OFFSET(dimensions))->North) ;

    *values++       = -conductance ;
    diagonal_value +=  conductance ;

    (*rows)++ ;
    added++ ;

#ifdef PRINT_SYSTEM_MATRIX
    fprintf (stderr,
      "  south   \t%d\t%.5e = %.5e || %.5e\n",
      *(columns-1), *(values-1),
      conductances->South, (conductances - ROW_OFFSET(dimensions))->North) ;
#endif
  }

  if ( current_layer > 0 )   /* BOTTOM */
  {
    *columns++ = current_cell - LAYER_OFFSET(dimensions) ;

    conductance = PARALLEL (conductances->Bottom,
                           (conductances - LAYER_OFFSET(dimensions))->Top) ;

    *values++       = -conductance ;
    diagonal_value +=  conductance ;

    (*rows)++ ;
    added++;

#ifdef PRINT_SYSTEM_MATRIX
    fprintf (stderr,
      "  bottom  \t%d\t%.5e = %.5e || %.5e\n",
      *(columns-1), *(values-1),
      conductances->Bottom, (conductances - LAYER_OFFSET(dimensions))->Top) ;
#endif
  }

  /* DIAGONAL ELEMENT */

  *diagonal_pointer += diagonal_value ;

#ifdef PRINT_SYSTEM_MATRIX
  fgetpos (stderr, &last_fpos) ;
  fsetpos (stderr, &diag_fposition) ;
  fprintf (stderr, "%.5e", *diagonal_pointer) ;
  fsetpos (stderr, &last_fpos) ;

  fprintf (stderr, "  %d (+%d)\n", *rows, added) ;
#endif

  return added ;
}

/******************************************************************************/

int add_ccs_liquid_column
(
  Dimensions*   dimensions,
  Conductances* conductances,
  Capacity_t*   capacities,
  LayerIndex_t  current_layer,
  RowIndex_t    current_row,
  ColumnIndex_t current_column,
  int*          columns,
  int*          rows,
  double*       values
)
{
  double  conductance      = 0.0 ;
  double  diagonal_value   = 0.0 ;
  double* diagonal_pointer = NULL ;
  int     added            = 0 ;
  int current_cell
    = current_layer * LAYER_OFFSET(dimensions)
      + current_row * ROW_OFFSET(dimensions)
      + current_column ;

#ifdef PRINT_SYSTEM_MATRIX
  fpos_t diag_fposition, last_fpos ;
  fprintf (stderr,
    "add_ccs_liquid_column  (l %2d r %5d c %5d) -> %5d\n",
    current_layer, current_row, current_column, current_cell) ;
#endif

  *columns = *(columns - 1) ;

  if ( current_layer > 0 )   /* BOTTOM */
  {
    *rows++ = current_cell - LAYER_OFFSET(dimensions) ;

    conductance = PARALLEL (conductances->Bottom,
                           (conductances - LAYER_OFFSET(dimensions))->Top) ;

    *values++       = -conductance ;
    diagonal_value +=  conductance ;

    (*columns)++ ;
    added++ ;

#ifdef PRINT_SYSTEM_MATRIX
    fprintf (stderr,
      "  bottom  \t%d\t%.5e = %.5e || %.5e\n",
      *(rows-1), *(values-1),
      conductances->Bottom, (conductances - LAYER_OFFSET(dimensions))->Top) ;
#endif
  }

  if ( current_row > 0 )   /* SOUTH */
  {
    *rows++   = current_cell - ROW_OFFSET(dimensions) ;
    *values++ = conductances->North ; // == (C)

    (*columns)++ ;
    added++ ;

#ifdef PRINT_SYSTEM_MATRIX
    fprintf (stderr,
      "  south   \t%d\t%.5e\n", *(rows-1), *(values-1)) ;
#endif
  }

  if ( current_column > 0 )   /* WEST */
  {
    *rows++ = current_cell - COLUMN_OFFSET(dimensions) ;

    conductance = PARALLEL (conductances->West,
                           (conductances - COLUMN_OFFSET(dimensions))->East) ;

    *values++        = -conductance ;
    diagonal_value  +=  conductance ;

    (*columns)++ ;
    added++;

#ifdef PRINT_SYSTEM_MATRIX
    fprintf (stderr,
      "  west    \t%d\t%.5e = %.5e || %.5e\n",
      *(rows-1), *(values-1),
      conductances->West, (conductances - COLUMN_OFFSET(dimensions))->East) ;
#endif
  }

  /* DIAGONAL */

  *rows++          = current_cell ;
  *values          = *capacities ;
  diagonal_pointer = values++ ;

  (*columns)++ ;
  added++;

#ifdef PRINT_SYSTEM_MATRIX
  fprintf (stderr, "  diagonal\t%d\t ", *(rows-1)) ;
  fgetpos (stderr, &diag_fposition) ;
  fprintf (stderr, "           \n") ;
#endif

  if ( current_column < dimensions->Grid.NColumns - 1 )    /* EAST */
  {
    *rows++ = current_cell + COLUMN_OFFSET(dimensions) ;

    conductance = PARALLEL (conductances->East,
                           (conductances + COLUMN_OFFSET(dimensions))->West) ;

    *values++       = -conductance ;
    diagonal_value +=  conductance ;

    (*columns)++ ;
    added++;

#ifdef PRINT_SYSTEM_MATRIX
    fprintf (stderr,
      "  east    \t%d\t%.5e = %.5e || %.5e\n",
      *(rows-1), *(values-1),
      conductances->East, (conductances + COLUMN_OFFSET(dimensions))->West) ;
#endif
  }

  if ( current_row < dimensions->Grid.NRows - 1 )   /* NORTH */
  {
    *rows++   = current_cell + ROW_OFFSET(dimensions) ;
    *values++ = conductances->South ; // == -C

    (*columns)++ ;
    added ++ ;

#ifdef PRINT_SYSTEM_MATRIX
    fprintf (stderr,
      "  north   \t%d\t%.5e\n", *(rows-1), *(values-1)) ;
#endif
  }

  if ( current_layer < dimensions->Grid.NLayers - 1)  /* TOP */
  {
    *rows++ = current_cell + LAYER_OFFSET(dimensions) ;

    conductance = PARALLEL (conductances->Top,
                           (conductances + LAYER_OFFSET(dimensions))->Bottom) ;

    *values++       = -conductance ;
    diagonal_value +=  conductance ;

    (*columns)++ ;
    added++;

#ifdef PRINT_SYSTEM_MATRIX
    fprintf (stderr,
      "  top     \t%d\t%.5e = %.5e || %.5e\n",
      *(rows-1), *(values-1),
      conductances->Top, (conductances + LAYER_OFFSET(dimensions))->Bottom) ;
#endif
  }

  /* DIAGONAL ELEMENT */

  *diagonal_pointer += diagonal_value ;

  if (current_row == 0 || current_row == dimensions->Grid.NRows - 1)

    *diagonal_pointer += conductances->North ; // == (C)

#ifdef PRINT_SYSTEM_MATRIX
  fgetpos (stderr, &last_fpos) ;
  fsetpos (stderr, &diag_fposition) ;
  fprintf (stderr, "%.5e", *diagonal_pointer) ;
  fsetpos (stderr, &last_fpos) ;

  fprintf (stderr, "  %d (+%d)\n", *columns, added) ;
#endif

  return added ;
}

/******************************************************************************/

int                    add_crs_liquid_column
(
  Dimensions*          dimensions,
  Conductances* conductances,
  Capacity_t*          capacities,
  LayerIndex_t         current_layer,
  RowIndex_t           current_row,
  ColumnIndex_t        current_column,
  int*                 rows,
  int*                 columns,
  double*              values
)
{
  double  conductance      = 0.0 ;
  double  diagonal_value   = 0.0 ;
  double* diagonal_pointer = NULL ;
  int     added            = 0 ;
  int current_cell
    = current_layer * LAYER_OFFSET(dimensions)
      + current_row * ROW_OFFSET(dimensions)
      + current_column ;

#ifdef PRINT_SYSTEM_MATRIX
  fpos_t diag_fposition, last_fpos ;
  fprintf (stderr,
    "add_crs_liquid_column  (l %2d r %5d c %5d) -> %5d\n",
    current_layer, current_row, current_column, current_cell) ;
#endif

  *rows = *(rows - 1) ;

  if ( current_layer < dimensions->Grid.NLayers - 1)  /* TOP */
  {
    *columns++ = current_cell + LAYER_OFFSET(dimensions) ;

    conductance = PARALLEL (conductances->Top,
                           (conductances + LAYER_OFFSET(dimensions))->Bottom) ;

    *values++       = -conductance ;
    diagonal_value +=  conductance ;

    (*rows)++ ;
    added++;

#ifdef PRINT_SYSTEM_MATRIX
    fprintf (stderr,
      "  top     \t%d\t%.5e = %.5e || %.5e\n",
      *(columns-1), *(values-1),
      conductances->Top, (conductances + LAYER_OFFSET(dimensions))->Bottom) ;
#endif
  }

  if ( current_row < dimensions->Grid.NRows - 1 )   /* NORTH */
  {
    *columns++ = current_cell + ROW_OFFSET(dimensions) ;
    *values++  = conductances->North ; // == -C

    (*rows)++ ;
    added ++ ;

#ifdef PRINT_SYSTEM_MATRIX
    fprintf (stderr,
      "  north   \t%d\t%.5e\n", *(columns-1), *(values-1)) ;
#endif
  }

  if ( current_column < dimensions->Grid.NColumns - 1 )    /* EAST */
  {
    *columns++ = current_cell + COLUMN_OFFSET(dimensions) ;

    conductance = PARALLEL (conductances->East,
                           (conductances + COLUMN_OFFSET(dimensions))->West) ;

    *values++       = -conductance ;
    diagonal_value +=  conductance ;

    (*rows)++ ;
    added++;

#ifdef PRINT_SYSTEM_MATRIX
    fprintf (stderr,
      "  east    \t%d\t%.5e = %.5e || %.5e\n",
      *(columns-1), *(values-1),
      conductances->East, (conductances + COLUMN_OFFSET(dimensions))->West) ;
#endif
  }

  /* DIAGONAL */

  *columns++       = current_cell ;
  *values          = *capacities ;
  diagonal_pointer = values++ ;

  (*rows)++ ;
  added++;

#ifdef PRINT_SYSTEM_MATRIX
  fprintf (stderr, "  diagonal\t%d\t ", *(columns-1)) ;
  fgetpos (stderr, &diag_fposition) ;
  fprintf (stderr, "           \n") ;
#endif

  if ( current_column > 0 )   /* WEST */
  {
    *columns++ = current_cell - COLUMN_OFFSET(dimensions) ;

    conductance = PARALLEL (conductances->West,
                           (conductances - COLUMN_OFFSET(dimensions))->East) ;

    *values++        = -conductance ;
    diagonal_value  +=  conductance ;

    (*rows)++ ;
    added++;

#ifdef PRINT_SYSTEM_MATRIX
    fprintf (stderr,
      "  west    \t%d\t%.5e = %.5e || %.5e\n",
      *(columns-1), *(values-1),
      conductances->West, (conductances - COLUMN_OFFSET(dimensions))->East) ;
#endif
  }

  if ( current_row > 0 )   /* SOUTH */
  {
    *columns++ = current_cell - ROW_OFFSET(dimensions) ;
    *values++  = conductances->South ; // == (C)

    (*rows)++ ;
    added++ ;

#ifdef PRINT_SYSTEM_MATRIX
    fprintf (stderr,
      "  south   \t%d\t%.5e\n", *(columns-1), *(values-1)) ;
#endif
  }

  if ( current_layer > 0 )   /* BOTTOM */
  {
    *columns++ = current_cell - LAYER_OFFSET(dimensions) ;

    conductance = PARALLEL (conductances->Bottom,
                           (conductances - LAYER_OFFSET(dimensions))->Top) ;

    *values++       = -conductance ;
    diagonal_value +=  conductance ;

    (*rows)++ ;
    added++ ;

#ifdef PRINT_SYSTEM_MATRIX
    fprintf (stderr,
      "  bottom  \t%d\t%.5e = %.5e || %.5e\n",
      *(columns-1), *(values-1),
      conductances->Bottom, (conductances - LAYER_OFFSET(dimensions))->Top) ;
#endif
  }

  /* DIAGONAL ELEMENT */

  *diagonal_pointer += diagonal_value ;

  if (current_row == 0 || current_row == dimensions->Grid.NRows - 1)

    *diagonal_pointer += conductances->North ; // == (C)

#ifdef PRINT_SYSTEM_MATRIX
  fgetpos (stderr, &last_fpos) ;
  fsetpos (stderr, &diag_fposition) ;
  fprintf (stderr, "%.5e", *diagonal_pointer) ;
  fsetpos (stderr, &last_fpos) ;

  fprintf (stderr, "  %d (+%d)\n", *rows, added) ;
#endif

  return added ;
}

/******************************************************************************/
