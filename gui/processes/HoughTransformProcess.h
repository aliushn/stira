
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

#ifndef STIRAGUI_PROCESSES_HOUGHTRANSFORM_H
#define STIRAGUI_PROCESSES_HOUGHTRANSFORM_H

#include "Process.h"

class HoughTransformProcess : public Process
{
    Q_OBJECT

public:
   /** \brief constructor
     * \param pImage input image */
   HoughTransformProcess( stira::imagedata::Image* pImage );

   virtual ~HoughTransformProcess();

   int GetMaximalRadius();
   void SetMaximalRadius( int maxRadius );

   int GetMinimalRadius();
   void SetMinimalRadius( int maxRadius );

   bool GetChooseCircles();
   void SetChooseCircles( bool chooseCircles );

   void run();

private:
   stira::imagedata::Image* RunLines();
   stira::imagedata::Image* RunCircles();
   int mMaximalRadius;
   int mMinimalRadius;
   bool mChooseCircles;
};

#endif // HOUGHTRANSFORMPROCESS_H
