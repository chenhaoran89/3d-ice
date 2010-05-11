/******************************************************************************
 *                                                                            *
 * Source file "Source/build_system_matrix.c"                                 *
 *                                                                            *
 *                                          -- alessandro.vincenzi@epfl.ch -- *
 ******************************************************************************/

#include <stdlib.h>

#include "resistances.h"
#include "system_matrix.h"

#define PARALLEL(x,y)      ( (x * y) / ( x + y) )
#define LAYER_OFFSET(dim)  (dim->Grid.NRows * dim->Grid.NColumns)
#define ROW_OFFSET(dim)    (dim->Grid.NColumns)
#define COLUMN_OFFSET(dim) (1)

int
alloc_system_matrix (SystemMatrix *matrix, int nvalues, int nnz)
{
  if (matrix == NULL) return 0 ;

  matrix->Size = nvalues ;
  matrix->NNz  = nnz ;

  matrix->Columns = (int *) malloc (sizeof(int) * nvalues + 1 ) ;

  if (matrix->Columns == NULL)
  {
    return 0 ;
  }

  matrix->Rows = (int *) malloc (sizeof(int) * nnz ) ;

  if (matrix->Rows == NULL)
  {
    free (matrix->Columns) ;
    return 0 ;
  }

  matrix->Values = (double *) malloc (sizeof(double) * nnz ) ;

  if (matrix->Values == NULL)
  {
    free (matrix->Columns) ;
    free (matrix->Rows) ;
    return 0 ;
  }

  return 1 ;
}

/******************************************************************************/
/******************************************************************************/
/******************************************************************************/

void free_system_matrix (SystemMatrix *matrix)
{
  if (matrix == NULL) return ;

  free (matrix->Columns) ;
  free (matrix->Rows) ;
  free (matrix->Values) ;
}

/******************************************************************************/
/******************************************************************************/
/******************************************************************************/

void
fill_system_matrix
(
  StackDescription *stkd,
  SystemMatrix *matrix,
  Resistances *resistances,
  double *capacities
)
{
  fill_system_matrix_stack_description (stkd,
    resistances, capacities,
    matrix->Columns, matrix->Rows, matrix->Values) ;
}

/******************************************************************************/
/******************************************************************************/
/******************************************************************************/

int
add_solid_column
(
#ifdef DEBUG_FILL_SYSTEM_MATRIX
  FILE        *debug,
#endif
  Dimensions  *dim,
  Resistances *resistances,
  double      *capacities,
  int         current_layer,
  int         current_row,
  int         current_column,
  int         *columns,
  int         *rows,
  double      *values
)
{
  double resistance        = 0.0 ;
  double diagonal_value    = 0.0 ;
  double *diagonal_pointer = NULL ;
  int    added             = 0 ;

  int current_cell
    = current_layer * LAYER_OFFSET(dim)
      + current_row * ROW_OFFSET(dim)
      + current_column ;

#ifdef DEBUG_FILL_SYSTEM_MATRIX
  fpos_t diag_fposition, last_fpos ;
  fprintf (debug,
    "%p %p %p %p %p add_solid_column  (l %2d r %5d c %5d) -> %5d\n",
    resistances, capacities, columns, rows, values,
    current_layer, current_row, current_column, current_cell) ;
#endif

  *columns = *(columns - 1) ;

  if ( current_layer > 0 )   /* BOTTOM */
  {
    *rows++ = current_cell - LAYER_OFFSET(dim) ;

    resistance = PARALLEL (resistances->Bottom,
                           (resistances - LAYER_OFFSET(dim))->Top) ;

    *values++       = -resistance ;
    diagonal_value +=  resistance ;

    (*columns)++ ;
    added++;

#ifdef DEBUG_FILL_SYSTEM_MATRIX
    fprintf (debug,
      "  bottom  \t%d\t%.5e = %.5e || %.5e\n",
      *(rows-1), *(values-1),
      resistances->Bottom, (resistances - LAYER_OFFSET(dim))->Top) ;
#endif
  }

  if ( current_row > 0 )   /* SOUTH */
  {
    *rows++ = current_cell - ROW_OFFSET(dim) ;

    resistance = PARALLEL (resistances->South,
                           (resistances - ROW_OFFSET(dim))->North) ;

    *values++       = -resistance ;
    diagonal_value +=  resistance ;

    (*columns)++ ;
    added++ ;

#ifdef DEBUG_FILL_SYSTEM_MATRIX
    fprintf (debug,
      "  south   \t%d\t%.5e = %.5e || %.5e\n",
      *(rows-1), *(values-1),
      resistances->South, (resistances - ROW_OFFSET(dim))->North) ;
#endif
  }

  if ( current_column > 0 )   /* WEST */
  {
    *rows++ = current_cell - COLUMN_OFFSET(dim) ;

    resistance = PARALLEL (resistances->West,
                           (resistances - COLUMN_OFFSET(dim))->East) ;

    *values++       = -resistance ;
    diagonal_value +=  resistance ;

    (*columns)++ ;
    added++ ;

#ifdef DEBUG_FILL_SYSTEM_MATRIX
    fprintf (debug,
      "  west    \t%d\t%.5e = %.5e || %.5e\n",
      *(rows-1), *(values-1),
      resistances->West, (resistances - COLUMN_OFFSET(dim))->East) ;
#endif
    }

  /* DIAGONAL */

  *rows++          = current_cell ;
  *values          = *capacities ;
  diagonal_pointer = values++ ;

  (*columns)++ ;
  added ++ ;

#ifdef DEBUG_FILL_SYSTEM_MATRIX
  fprintf (debug, "  diagonal\t%d\t ", *(rows-1)) ;
  fgetpos (debug, &diag_fposition) ;
  fprintf (debug, "           \n") ;
#endif

  if ( current_column < dim->Grid.NColumns - 1 )   /* EAST */
  {
    *rows++ = current_cell + COLUMN_OFFSET(dim) ;

    resistance = PARALLEL (resistances->East,
                           (resistances + COLUMN_OFFSET(dim))->West) ;

    *values++       = -resistance ;
    diagonal_value +=  resistance ;

    (*columns)++ ;
    added ++ ;

#ifdef DEBUG_FILL_SYSTEM_MATRIX
    fprintf (debug,
      "  east    \t%d\t%.5e = %.5e || %.5e\n",
      *(rows-1), *(values-1),
      resistances->East, (resistances + COLUMN_OFFSET(dim))->West) ;
#endif
  }

  if ( current_row < dim->Grid.NRows - 1 )   /* NORTH */
  {
    *rows++ = current_cell + ROW_OFFSET(dim) ;

    resistance = PARALLEL (resistances->North,
                           (resistances + ROW_OFFSET(dim))->South) ;

    *values++       = -resistance ;
    diagonal_value +=  resistance ;

    (*columns)++ ;
    added++ ;

#ifdef DEBUG_FILL_SYSTEM_MATRIX
    fprintf (debug,
      "  north   \t%d\t%.5e = %.5e || %.5e\n",
      *(rows-1), *(values-1),
      resistances->North, (resistances + ROW_OFFSET(dim))->South) ;
#endif
  }

  if ( current_layer < dim->Grid.NLayers - 1) /* TOP */
  {
    *rows++ = current_cell + LAYER_OFFSET(dim) ;

    resistance = PARALLEL (resistances->Top,
                           (resistances + LAYER_OFFSET(dim))->Bottom) ;

    *values++       = -resistance ;
    diagonal_value +=  resistance ;

    (*columns)++ ;
    added++ ;

#ifdef DEBUG_FILL_SYSTEM_MATRIX
    fprintf (debug,
      "  top     \t%d\t%.5e = %.5e || %.5e\n",
      *(rows-1), *(values-1),
      resistances->Top, (resistances + LAYER_OFFSET(dim))->Bottom) ;
#endif
  }

  /* DIAGONAL ELEMENT */

  *diagonal_pointer += diagonal_value ;

#ifdef DEBUG_FILL_SYSTEM_MATRIX
  fgetpos (debug, &last_fpos) ;
  fsetpos (debug, &diag_fposition) ;
  fprintf (debug, "%.5e", *diagonal_pointer) ;
  fsetpos (debug, &last_fpos) ;

  fprintf (debug, "  %d (+%d)\n", *columns, added) ;
#endif

  return added ;
}

/******************************************************************************/
/******************************************************************************/
/******************************************************************************/

int
add_liquid_column
(
#ifdef DEBUG_FILL_SYSTEM_MATRIX
  FILE        *debug,
#endif
  Dimensions  *dim,
  Resistances *resistances,
  double      *capacities,
  int         current_layer,
  int         current_row,
  int         current_column,
  int         *columns,
  int         *rows,
  double      *values
)
{
  double resistance        = 0.0 ;
  double diagonal_value    = 0.0 ;
  double *diagonal_pointer = NULL ;
  int    added             = 0 ;
  int current_cell
    = current_layer * LAYER_OFFSET(dim)
      + current_row * ROW_OFFSET(dim)
      + current_column ;

#ifdef DEBUG_FILL_SYSTEM_MATRIX
  fpos_t diag_fposition, last_fpos ;
  fprintf (debug,
    "%p %p %p %p %p add_liquid_column  (l %2d r %5d c %5d) -> %5d\n",
    resistances, capacities, columns, rows, values,
    current_layer, current_row, current_column, current_cell) ;
#endif

  *columns = *(columns - 1) ;

  if ( current_layer > 0 )   /* BOTTOM */
  {
    *rows++ = current_cell - LAYER_OFFSET(dim) ;

    resistance = PARALLEL (resistances->Bottom,
                           (resistances - LAYER_OFFSET(dim))->Top) ;

    *values++       = -resistance ;
    diagonal_value +=  resistance ;

    (*columns)++ ;
    added++ ;

#ifdef DEBUG_FILL_SYSTEM_MATRIX
    fprintf (debug,
      "  bottom  \t%d\t%.5e = %.5e || %.5e\n",
      *(rows-1), *(values-1),
      resistances->Bottom, (resistances - LAYER_OFFSET(dim))->Top) ;
#endif
  }

  if ( current_row > 0 )   /* SOUTH */
  {
    *rows++   = current_cell - ROW_OFFSET(dim) ;
    *values++ = resistances->North ; // == (C)

    (*columns)++ ;
    added++ ;

#ifdef DEBUG_FILL_SYSTEM_MATRIX
    fprintf (debug,
      "  south   \t%d\t%.5e\n", *(rows-1), *(values-1)) ;
#endif
  }

  if ( current_column > 0 )   /* WEST */
  {
    *rows++ = current_cell - COLUMN_OFFSET(dim) ;

    resistance = PARALLEL (resistances->West,
                           (resistances - COLUMN_OFFSET(dim))->East) ;

    *values++        = -resistance ;
    diagonal_value  +=  resistance ;

    (*columns)++ ;
    added++;

#ifdef DEBUG_FILL_SYSTEM_MATRIX
    fprintf (debug,
      "  west    \t%d\t%.5e = %.5e || %.5e\n",
      *(rows-1), *(values-1),
      resistances->West, (resistances - COLUMN_OFFSET(dim))->East) ;
#endif
  }

  /* DIAGONAL */

  *rows++          = current_cell ;
  *values          = *capacities ;
  diagonal_pointer = values++ ;

  (*columns)++ ;
  added++;

#ifdef DEBUG_FILL_SYSTEM_MATRIX
  fprintf (debug, "  diagonal\t%d\t ", *(rows-1)) ;
  fgetpos (debug, &diag_fposition) ;
  fprintf (debug, "           \n") ;
#endif

  if ( current_column < dim->Grid.NColumns - 1 )    /* EAST */
  {
    *rows++ = current_cell + COLUMN_OFFSET(dim) ;

    resistance = PARALLEL (resistances->East,
                           (resistances + COLUMN_OFFSET(dim))->West) ;

    *values++       = -resistance ;
    diagonal_value +=  resistance ;

    (*columns)++ ;
    added++;

#ifdef DEBUG_FILL_SYSTEM_MATRIX
    fprintf (debug,
      "  east    \t%d\t%.5e = %.5e || %.5e\n",
      *(rows-1), *(values-1),
      resistances->East, (resistances + COLUMN_OFFSET(dim))->West) ;
#endif
  }

  if ( current_row < dim->Grid.NRows - 1 )   /* NORTH */
  {
    *rows++   = current_cell + ROW_OFFSET(dim) ;
    *values++ = resistances->South ; // == -C

    (*columns)++ ;
    added ++ ;

#ifdef DEBUG_FILL_SYSTEM_MATRIX
    fprintf (debug,
      "  north   \t%d\t%.5e\n", *(rows-1), *(values-1)) ;
#endif
  }

  if ( current_layer < dim->Grid.NLayers - 1)  /* TOP */
  {
    *rows++ = current_cell + LAYER_OFFSET(dim) ;

    resistance = PARALLEL (resistances->Top,
                           (resistances + LAYER_OFFSET(dim))->Bottom) ;

    *values++       = -resistance ;
    diagonal_value +=  resistance ;

    (*columns)++ ;
    added++;

#ifdef DEBUG_FILL_SYSTEM_MATRIX
    fprintf (debug,
      "  top     \t%d\t%.5e = %.5e || %.5e\n",
      *(rows-1), *(values-1),
      resistances->Top, (resistances + LAYER_OFFSET(dim))->Bottom) ;
#endif
  }

  /* DIAGONAL ELEMENT */

  *diagonal_pointer += diagonal_value ;

  if (current_row == 0 || current_row == dim->Grid.NRows - 1)

    *diagonal_pointer += resistances->North ; // == (C)

#ifdef DEBUG_FILL_SYSTEM_MATRIX
  fgetpos (debug, &last_fpos) ;
  fsetpos (debug, &diag_fposition) ;
  fprintf (debug, "%.5e", *diagonal_pointer) ;
  fsetpos (debug, &last_fpos) ;

  fprintf (debug, "  %d (+%d)\n", *columns, added) ;
#endif

  return added ;
}

/******************************************************************************/
/******************************************************************************/
/******************************************************************************/
