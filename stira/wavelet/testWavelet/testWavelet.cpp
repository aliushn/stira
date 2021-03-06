
/***********************************************************************************
 *   Copyright (C) 2008 by Filip Rooms                                             *
 *                                                                                 *
 *  Terms and conditions for using this software in any form are provided in the   *
 *  file COPYING, which can be found in the root directory of this project.        *
 *                                                                                 *
 *   Contact data: filip.rooms@gmail.com                                           *
 *                 http://www.filiprooms.be/                                       *
 *                                                                                 *
 ***********************************************************************************/

#include "../../imagetools/tools/NumberGridTools.h"
#include "../../imagedata/datastructures/Image.h"
#include "../../imagetools/tools/ImageIO.h"
#include "../../imagetools/tools/PyramidTools.h"
#include "../../imagetools/imagegenerator/GridGenerator.h"
#include "../../common/common/ExecutionTimer.h"
#include "../wavelet/WaveletTapGenerator.h"
#include "../wavelet/CriticallySubsampledTransform.h"
#include "../wavelet/SubsampledTransform.h"
#include "../wavelet/NoiseEstimator.h"
#include "../wavelet/NonSubsampledTransform.h"

#define VISUALIZE_WAVELETS
#define TIME_PERFORMANCE

using namespace stira;
using namespace stira::common;
using namespace stira::imagedata;
using namespace stira::imagetools;
using namespace stira::wavelet;
using namespace std;

//============================================================================

bool WaveletNoiseEstimationTest(Image* pImage)
{
   Image* pImageNoise = pImage->Clone();
   double sigmaNoise = 15.0;
   bool isColoredNoise = true;
   pImageNoise->AddGaussianNoise( sigmaNoise, isColoredNoise);

   ImageIO::WriteOpenCV(pImageNoise, std::string("Noisy.pgm"));

   ImageIO::WriteOpenCV(pImage, std::string("Input.pgm"));

   double sigmaEstimated = NoiseEstimator::Run( pImageNoise->GetBands()[0] );

   //#ifdef VISUALIZE_WAVELETS
      cout << "Noise estimation: real sigma = " << sigmaNoise  << ", estimated sigma = " << sigmaEstimated << "." << endl << flush;
   //#endif
   delete pImageNoise;

   if ( fabs( sigmaNoise - sigmaEstimated ) < 1.0 )
   {
      return true;
   }
   else
   {
      return false;
   }
}

//============================================================================

bool CriticallySubsampledWaveletTest( Image* pImage, double hardThreshold, int nrScales )
{
   CriticallySubsampledTransform wdr;

   ArrayGrid<double>* pGridIn = pImage->GetBands()[0]; // just take the red band for a test

   wdr.Decompose( pGridIn, nrScales );

   #ifdef VISUALIZE_WAVELETS
      // visualize bands as separate PGM files
      //wdr.GetPyramid()->ExportBandsToPGM();

      // visualize all bands together in single image
      Image* pCritical = PyramidTools::VisualizeRealPyramid( wdr.GetPyramid() );
      ImageIO::Write( pCritical, string("HaarDecomposeBands.pgm"), ImageIO::NULL_OUT );
      delete pCritical;
   #endif

   // apply hard threshold to all bands and reconstruct
   wdr.Reconstruct( hardThreshold );
   ArrayGrid<double>* pOutGrid = wdr.GetCopyOfReconstructedGrid();
   double psnr = NumberGridTools<double>::ComputePSNR( pGridIn, pOutGrid );

   #ifdef VISUALIZE_WAVELETS
      cout << "CriticallySubsampledWaveletTest: PSNR after reconstruction is " << psnr << endl << flush;
      ImageIO::WritePGM( pOutGrid, string("HaarDecomposeReconstruct.pgm"), ImageIO::NULL_OUT );
   #endif
   delete pOutGrid;

   if ( psnr > 150.0 )
   {
      return true;
   }
   else if (hardThreshold > 0.0)
   {
      cout << "Low quality reconstruction (PSNR=" << psnr << "); SET hardThreshold to 0!!! Is now " << hardThreshold << endl << flush;
      return false;
   }
   else
   {
      return false;
   }
}

//============================================================================

bool SubsampledWaveletTest( Image* pImage, double hardThreshold, int nrScales )
{
   WaveletType myWaveletTapType = DAUBECHIES_8;
   SubsampledTransform st( myWaveletTapType );
   int width  = pImage->GetWidth();
   int height = pImage->GetHeight();

   assert ( width == height );

   ArrayGrid<double>* pSourceGrid = pImage->GetBands()[0]->Clone();

   #ifdef TIME_PERFORMANCE
      ExecutionTimer et;
      et.StartTimer();
   #endif

   st.Decompose( pSourceGrid, nrScales );

   #ifdef VISUALIZE_WAVELETS
      ArrayGrid<double>* pDecomposedGrid = st.GetCopyOfReconstructedGrid();
      ImageIO::WritePGM( pDecomposedGrid, std::string("SubsampledDecomposeBands-ClassicalView.pgm"), ImageIO::GRADIENT_OUT );

      Image* pRedundant = PyramidTools::VisualizeRealPyramid( st.GetPyramid() );
      ImageIO::Write( pRedundant, string("SubsampledDecomposeBands.pgm"), ImageIO::NULL_OUT );

      Image* pClassic = PyramidTools::VisualizeClassicWavelet( st.GetPyramid() );
      ImageIO::Write( pClassic, string("SubsampledDecomposeClassicView2.pgm"), ImageIO::NULL_OUT );
      delete pRedundant;
      delete pDecomposedGrid;
   #endif

   st.Reconstruct( hardThreshold );

   ArrayGrid<double>* pReconstructedGrid = st.GetCopyOfReconstructedGrid();

   #ifdef TIME_PERFORMANCE
      et.StopTimer();
      et.PrintTimeAsUserAndSystemTimes();
   #endif

   #ifdef VISUALIZE_WAVELETS
      ImageIO::WritePGM( pReconstructedGrid, std::string("SubsampledDecomposeReconstruct.pgm"), ImageIO::NULL_OUT );
   #endif

   double psnr = NumberGridTools<double>::ComputePSNR( pReconstructedGrid, pSourceGrid );
   cout << "SubsampledWaveletTest Decompose / reconstruct -> PSNR = " << psnr << endl << flush;

   delete pReconstructedGrid;
   delete pSourceGrid;
   if ( psnr > 150.0 )
   {
      cout << "High quality reconstruction (PSNR=" << psnr << ")" << endl << flush;
      return true;
   }
   else if (hardThreshold > 0.0)
   {
      cout << "Low quality reconstruction (PSNR=" << psnr << "); SET hardThreshold to 0!!! Is now " << hardThreshold << endl << flush;
      return false;
   }
   else
   {
      cout << "Low quality reconstruction (PSNR=" << psnr << ")" << endl << flush;
      return false;
   }
}

//============================================================================

bool RedundantWaveletTest( Image* pImage, double hardThreshold, int nrScales )
{
   WaveletType myWaveletTapType = SYMLET_8;
   NonSubsampledTransform wdr( myWaveletTapType );

   ArrayGrid<double>* pGridIn = pImage->GetBands()[0]; // just take the red band for a test

   #ifdef TIME_PERFORMANCE
      ExecutionTimer et;
      et.StartTimer();
   #endif

   wdr.Decompose( pGridIn, nrScales);

   #ifdef VISUALIZE_WAVELETS
      // visualize bands as separate PGM files
      //wdr.GetPyramid()->ExportBandsToPGM();

      // visualize all bands together in single image
      Image* pRedundant = PyramidTools::VisualizeRealPyramid( wdr.GetPyramid() );
      ImageIO::Write( pRedundant, string("RedundantDecomposeBands.pgm"), ImageIO::NULL_OUT );
      delete pRedundant;
   #endif

   // apply hard threshold to all bands and reconstruct
   wdr.Reconstruct( hardThreshold );
   ArrayGrid<double> * pOutGrid = wdr.GetCopyOfReconstructedGrid();

   #ifdef TIME_PERFORMANCE
      et.StopTimer();
      et.PrintTimeAsUserAndSystemTimes();
   #endif

   double psnr = NumberGridTools<double>::ComputePSNR( pGridIn, pOutGrid );
   cout << "RedundantWaveletTest: PSNR after reconstruction is " << psnr << endl << flush;

   #ifdef VISUALIZE_WAVELETS
      ImageIO::WritePGM( pOutGrid, string("RedundantDecomposeReconstruct.pgm"), ImageIO::NORMAL_OUT );
   #endif
   delete pOutGrid;

   if ( psnr > 150.0 )
   {
      cout << "High quality reconstruction (PSNR=" << psnr << ")" << endl << flush;
      return true;
   }
   else if (hardThreshold > 0.0)
   {
      cout << "Low quality reconstruction (PSNR=" << psnr << "); SET hardThreshold to 0!!! Is now " << hardThreshold << endl << flush;
      return false;
   }
   else
   {
      cout << "Low quality reconstruction (PSNR=" << psnr << ")" << endl << flush;
      return false;
   }
}

//============================================================================

int main(int argc, char *argv[])
{
   Image* pImage = 0;
   string fileNamePPM;
   if (argc < 2)
   {
      fileNamePPM = "../../../../stira/stira/testdata/lena512.pgm";
      cout << "File name is " << fileNamePPM << endl << flush;
   }
   else
   {
      fileNamePPM = string( argv[1] );
   }
   pImage = ImageIO::Read( fileNamePPM );

   double hardThreshold = 0.0;
   int nrScales = 3;

   if (CriticallySubsampledWaveletTest( pImage, hardThreshold, nrScales ) == true )
   {
      cout << "CriticallySubsampledWavelet test success!!" << endl << flush;
   }
   else
   {
      cout << "CriticallySubsampledWavelet test FAILED!!" << endl << flush;
   }

   if (WaveletNoiseEstimationTest( pImage ) == true )
   {
      cout << "WaveletNoiseEstimationTest success!!" << endl << flush;
   }
   else
   {
      cout << "WaveletNoiseEstimationTest FAILED!!" << endl << flush;
   }

   if ( RedundantWaveletTest( pImage, hardThreshold, nrScales ) == true )
   {
      cout << "RedundantWaveletTest success!!" << endl << flush;
   }
   else
   {
      cout << "RedundantWaveletTest FAILED!!" << endl << flush;
   }

   if ( SubsampledWaveletTest( pImage, hardThreshold, nrScales ) == true )
   {
      cout << "SubsampledWaveletTest success!!" << endl << flush;
   }
   else
   {
      cout << "SubsampledWaveletTest FAILED!!" << endl << flush;
   }

   delete pImage;
   return 1;
}
