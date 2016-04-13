
/***********************************************************************************
 *   Copyright (C) 2016 by Filip Rooms                                             *
 *                                                                                 *
 *  Terms and conditions for using this software in any form are provided in the   *
 *  file COPYING, which can be found in the root directory of this project.        *
 *                                                                                 *
 *   Contact data: filip.rooms@gmail.com                                           *
 *                 http://www.filiprooms.be/                                       *
 *                                                                                 *
 ***********************************************************************************/

#ifndef STIRA_STEERABLE_PYRAMID_PYRAMIDBURTADELSON_H
#define STIRA_STEERABLE_PYRAMID_PYRAMIDBURTADELSON_H
#include "../../image/datastructures/Image.h"
#include "../../image/datastructures/Pyramid.h"
#include "../../filter/filter/SeparableFilter.h"


namespace stira {
namespace steerable {
/** \brief class implementing the Laplacian pyramid accoring to Burt and Adelson */
class PyramidBurtAdelson
{
public:
    /** \brief constructor */
    PyramidBurtAdelson();

    /** \brief destructor */
    ~PyramidBurtAdelson();

    /** \brief decomposes an image in given number of scales
      * \param pImage input image
      * \param nrScales number of scales to decompose into*/
    bool Decompose( image::Image* pImage, int nrScales );

    /** \brief reconstructs pyramid back to image */
    image::ArrayGrid<double>* Reconstruct( );

    /** \brief gets pointer to pyramid data structure */
    image::Pyramid<double>* GetPyramid();

private:
    /** \brief decomposes a single scale of the pyramid
      * \param pGridIn input grid to decompose to next scale
      * \param scale number of current scale */
    image::ArrayGrid<double>* DecomposeSingleScale( image::ArrayGrid<double>* pGridIn, int scale );

    /** \brief reconstructs a single scale of the pyramid
      * \param pGridIn input grid to reconstruct to next scale
      * \param scale number of current scale  */
    image::ArrayGrid<double>* ReconstructSingleScale( image::ArrayGrid<double>* pGridIn, int scale );

    int mNrScales;  ///< number of scales in the pyramid
    filter::SeparableFilter* mSeparableFilter;  ///< object to filter an image
    image::Pyramid<double>* mpPyramid;  ///< data structure for the image pyramid
};

}
}

#endif // PYRAMIDBURTADELSON_H
