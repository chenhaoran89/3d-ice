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

#ifndef _3DICE_CHANNEL_H_
#define _3DICE_CHANNEL_H_

/*! \file channel.h */

#ifdef __cplusplus
extern "C"
{
#endif

/******************************************************************************/

#include <stdio.h>

#include "types.h"

#include "dimensions.h"
#include "material.h"
#include "system_matrix.h"
#include "thermal_cell.h"

/******************************************************************************/

    /*! \struct Channel
     *
     *  \brief Structure used to store data about the channel that compose the 2D/3D stack.
     *
     *  Channel is one of the elements that can be used to build a 3d stack
     */

    struct Channel
    {
        /*! The channel model (rm4 - rm2) */

        ChannelModel_t ChannelModel ;

        /*! The height of the channel in \f$ \mu m \f$ (1 cell) */

        double Height ;

        /*! The length of the channel in \f$ \mu m \f$ */

        double Length ;

        /*! The pitch of the channel in \f$ \mu m \f$ */

        double Pitch ;

        /*! Porosity */

        double Porosity ;

        /*!  Number of channels (per cavity) along chip length */

        uint32_t NChannels ;

        /*! The number of layer composing the channel */

        uint32_t NLayers ;

        /*! The offset (\# layers) of the source layer within the channel */

        uint32_t SourceLayerOffset ;

        /*! The properties of the fluid used as coolant */

        Coolant_t Coolant ;

        /*! The flow rate per channel layer of the incolimg liquid.
         *  Specified in \f$ \frac{ml}{min} \f$ but stored in \f$ \frac{\mu m^3}{sec} \f$.
         *  Shared by all the channels for each layer in the 3DStack.
         */

        double CoolantFR ;

        /*! Darcy Velocity \f$ \frac{\mu m}{sec} \f$ */

        double DarcyVelocity ;

        /*! The material composing the wall */

        Material *WallMaterial ;

    } ;

    /*! Definition of the type Channel */

    typedef struct Channel Channel ;

/******************************************************************************/



    /*! Sets all the fields of \a channel to a default value (zero or \c NULL ).
     *
     * \param channel the address of the channel to initialize
     */

    void init_channel (Channel *channel) ;



    /*! Allocates a channel in memory and sets its fields to their default
     *  value with #init_channel
     *
     * \return the pointer to a new Channel
     * \return \c NULL if the memory allocation fails
     */

    Channel *alloc_and_init_channel (void) ;


    /*! Frees the memory related to \a channel
     *
     * The parametrer \a channel must be a pointer previously obtained with
     * #alloc_and_init_channel
     *
     * \param channel the address of the channel structure to free
     */

    void free_channel (Channel *channel) ;



    /*! Prints the channel as it looks in the stack file
     *
     * \param stream  the output stream (must be already open)
     * \param prefix  a string to be printed as prefix at the beginning of each line
     * \param channel the channel to print
     * \param dimensions pointer to the structure storing the dimensions
     */

    void print_formatted_channel

        (FILE *stream, char *prefix, Channel *channel, Dimensions *dimensions) ;



    /*! Prints detailed information about all the fields of a channel
     *
     * \param stream the output stream (must be already open)
     * \param prefix a string to be printed as prefix at the beginning of each line
     * \param channel the channel to print
     */

    void print_detailed_channel

        (FILE *stream, char *prefix, Channel *channel) ;

    /*! Fills the thermal cells corresponding to a channel
     *
     *  \param thermal_cells pointer to the first thermal cell in the 3d stack
     *  \param delta_time    the time resolution of the thermal simulation
     *  \param dimensions    pointer to the structure storing the dimensions
     *  \param layer_index   offset (\# layers) of the channel within the stack
     *  \param channel       pointer to the channel
     */

    void fill_thermal_cell_channel
    (
        ThermalCell     *thermal_cells,
        double           delta_time,
        Dimensions      *dimensions,
        uint32_t         layer_index,
        Channel         *channel
    ) ;



    /*! Fills the source vector corresponding to a channel
     *
     *  \param sources     pointer to the first element in the source vector
     *  \param dimensions  pointer to the structure storing the dimensions
     *  \param layer_index offset (\# layers) of the channel within the stack
     *  \param channel     pointer to the channel
     */

    void fill_sources_channel
    (
        double     *sources,
        Dimensions *dimensions,
        uint32_t    layer_index,
        Channel    *channel
    ) ;



    /*! Fills the system matrix
     *
     *  \param channel       pointer to the channel
     *  \param dimensions    pointer to the structure storing the dimensions
     *  \param thermal_cells pointer to the first thermal cell in the 3d stack
     *  \param layer_index   offset (\# layers) of the channel within the stack
     *  \param system_matrix copy of the system matrix structure
     *
     *  \return A matrix partially filled (FIXME)
     */

    SystemMatrix fill_system_matrix_channel
    (
        Channel      *channel,
        Dimensions   *dimensions,
        ThermalCell  *thermal_cells,
        uint32_t      layer_index,
        SystemMatrix  system_matrix
    ) ;

/******************************************************************************/

#ifdef __cplusplus
}
#endif

#endif /* _3DICE_CHANNEL_H_ */
