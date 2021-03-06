
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

#include "../../imagedata/simpletools/ArrayGridTools.h"
#include "../../imagetools/tools/NumberGridTools.h"
#include "PyramidComplex.h"

namespace stira {
namespace steerable {

using namespace fouriertools;
using namespace imagedata;
using namespace imagetools;
using namespace std;

PyramidComplex::PyramidComplex( ArrayGrid<double>* pGridIn, int myNrScales, int myNrOrientations)
 : PyramidMaster< complex< double > >( pGridIn, myNrScales, myNrOrientations )
{
}

//------------------------------------------------------------------------------------------

PyramidComplex::~PyramidComplex()
{
}

//------------------------------------------------------------------------------------------

bool PyramidComplex::Decompose()
{
   bool needsResidualScale = true;
   bool isSubsampled = true;
   mIsForwardTransform = true;
   FFTBand* pFFTInputBand = new FFTBand( mpSourceGrid );
   pFFTInputBand->ApplyForwardTransform();
   SetFFTBand( pFFTInputBand );     // do input FFT just once instead for each band separately
   
   mCurrentScale = 0;
   if (mpPyramid != 0) {delete mpPyramid;}
   mpPyramid = new Pyramid< complex<double> >( mpSourceGrid, mNrScales, mNrOrientations, isSubsampled, needsResidualScale );
   
   //decompose residual part: highest frequency bands, denoted by Portilla as A_K
   //////////////////////////////////////////////////////////////////////////////

   int tmpWidth  = pFFTInputBand->GetWidth();
   int tmpHeight = pFFTInputBand->GetHeight();
   mpTmpHighpassGrid = TransferFunctionGenerator::GenerateHighPassTransferFunction( tmpWidth, tmpHeight, tmpWidth / 4, tmpWidth / 2);
   for ( mCurrentOrientation = 0; mCurrentOrientation < mNrOrientations; mCurrentOrientation++ )
   {
      FFTBand* pFFTB0Band = ExtractB0( common::MODE_COMPLEX );
      ArrayGrid< complex<double> >* pResultingGrid = pFFTB0Band->ConvertToComplexGrid( );
      delete pFFTB0Band;
      mpPyramid->GetResidualScale()->AddOrientedBand( pResultingGrid );
   }
   mpPyramid->SetLowpassResidual( ExtractL0() );
   CleanFFTBand();
   delete mpTmpHighpassGrid;
   
   //decompose recursive part: denoted by Portilla as B_K
   //////////////////////////////////////////////////////
   
   for ( mCurrentScale = 0; mCurrentScale < mNrScales; mCurrentScale++)
   {
      FFTBand* pFFTInputBandRecursive = new FFTBand( mpPyramid->GetLowpassResidual( ) );
      pFFTInputBandRecursive->ApplyForwardTransform();
      SetFFTBand( pFFTInputBandRecursive );
      
      tmpWidth = pFFTInputBandRecursive->GetWidth();
      tmpHeight = pFFTInputBandRecursive->GetHeight();
      mpTmpHighpassGrid = TransferFunctionGenerator::GenerateHighPassTransferFunction(tmpWidth, tmpHeight, tmpWidth / 8, tmpWidth / 4);
      for ( mCurrentOrientation = 0; mCurrentOrientation < mNrOrientations; mCurrentOrientation++ )
      {
         FFTBand* pFFTBBand = ExtractB( common::MODE_COMPLEX );
      
         ArrayGrid< complex<double> >* pResultingGrid = pFFTBBand->ConvertToComplexGrid( );
         delete pFFTBBand;
         mpPyramid->GetRecursiveScale( mCurrentScale )->AddOrientedBand( pResultingGrid );
      }
      ArrayGrid<double>* pNextL = ExtractL( );
      ArrayGrid<double>* pNextLbis = ArrayGridTools<double>::DownSampleGrid( pNextL ); delete pNextL;
      mpPyramid->SetLowpassResidual( pNextLbis );
      delete mpTmpHighpassGrid;
   }
   CleanFFTBand();
   return true;
}

//------------------------------------------------------------------------------------------

bool PyramidComplex::VisualizeComplexBandpass( ArrayGrid< complex<double> >* pGrid, string bandType, int scaleNr, int orientationNr )
{
   stringstream ss;

   ss << bandType << string("-") << scaleNr << "-" << orientationNr;
   
   string fileName = ss.str();
   
   cout << "While inspecting " << fileName << endl << flush;
   
   ImageIO::WritePGM( pGrid, fileName, ImageIO::GRADIENT_OUT);
   return true;
}

//------------------------------------------------------------------------------------------

bool PyramidComplex::VisualizeReconstructedBandpass( fouriertools::FFTBand* pFftBandIn, string bandType, int scaleNr, int orientationNr )
{
   stringstream ss;
   ImageIO::outputType outtype;
   FFTBand* pCopyBand = pFftBandIn->Clone();
   if ( bandType == string("L"))
   {
      outtype = ImageIO::NORMAL_OUT;
      if (orientationNr == -1)
      {
         ss << bandType << scaleNr << string(".pgm");
      }
      else
      {
         ss << bandType << string("-") << scaleNr << string(".pgm");
      }
   }
   else
   { 
      outtype = ImageIO::GRADIENT_OUT;
      ss << bandType << string("Reconst-") << scaleNr << string("-") << orientationNr << string(".pgm");
   }
   string fileName = ss.str();
   
   cout << "While inspecting " << fileName << endl << flush;

   pCopyBand->ApplyInverseTransform();
   pCopyBand->SwitchQuadrants();
   ArrayGrid<complex<double> >* pResultGrid = pCopyBand->ConvertToComplexGrid( );
   
   delete pCopyBand;
   ImageIO::WritePGM( pResultGrid, fileName, outtype);
   delete pResultGrid;
   return true;
}

//------------------------------------------------------------------------------------------

bool PyramidComplex::Reconstruct()
{
   ImageIO io;
   int subbandWidth, subbandHeight;
   
   //reconstruct recursive part
   ////////////////////////////
   
   std::vector< FFTBand* > vReconstructedSubbandsFFT;
   mIsForwardTransform = false;
   
   for ( mCurrentScale = mNrScales-1; mCurrentScale >= 0; mCurrentScale--)
   {
      subbandWidth  = mpPyramid->GetRecursiveScale( mCurrentScale )->GetWidth();
      subbandHeight = mpPyramid->GetRecursiveScale( mCurrentScale )->GetHeight();
      
      mpTmpHighpassGrid = TransferFunctionGenerator::GenerateHighPassTransferFunction( subbandWidth, subbandHeight, subbandWidth / 8, subbandWidth / 4);

      ArrayGrid<double>* pNextL = ArrayGridTools<double>::UpSampleGrid( mpPyramid->GetLowpassResidual(), subbandWidth, subbandHeight );
      vReconstructedSubbandsFFT.push_back( ReconstructL( pNextL ) );
      delete pNextL;
      
      for ( mCurrentOrientation = 0; mCurrentOrientation < mNrOrientations; mCurrentOrientation++)
      {
         FFTBand* tmpFFTBand = ReconstructB( mpPyramid->GetRecursiveScale( mCurrentScale )->GetOrientedBand( mCurrentOrientation ), common::MODE_COMPLEX );
         vReconstructedSubbandsFFT.push_back( tmpFFTBand );
      }
      
      if ( mCurrentScale > 0)
      {
         ArrayGrid<double>* pResultFFTRec = MergeAndReconstructFFTBands( vReconstructedSubbandsFFT );
         ClearFFTVector( vReconstructedSubbandsFFT );
         mpPyramid->SetLowpassResidual( pResultFFTRec );
      }
      else
      {
         FFTBand* pRec0 = ReconstructL0( vReconstructedSubbandsFFT );
         ClearFFTVector( vReconstructedSubbandsFFT );
         vReconstructedSubbandsFFT.push_back( pRec0 );
      }
      delete mpTmpHighpassGrid;
   }
   
   //reconstruct residual part: apply L0 to rec0 and add A_k's
   ///////////////////////////////////////////////////////////
   
   mCurrentScale = 0;
   subbandWidth  = mpPyramid->GetResidualScale()->GetOrientedBand( 0 )->GetWidth();
   subbandHeight = mpPyramid->GetResidualScale()->GetOrientedBand( 0 )->GetHeight();
   
   mpTmpHighpassGrid = TransferFunctionGenerator::GenerateHighPassTransferFunction( subbandWidth, subbandHeight, subbandWidth / 4, subbandWidth / 2);
   for ( mCurrentOrientation = 0; mCurrentOrientation < mNrOrientations; mCurrentOrientation++)
   {
      FFTBand* tmpFFTBand = ReconstructB0( mpPyramid->GetResidualScale()->GetOrientedBand( mCurrentOrientation ), common::MODE_COMPLEX);
      vReconstructedSubbandsFFT.push_back( tmpFFTBand );
   }
   
   if (mpReconstructedGrid != 0)
   {
      delete mpReconstructedGrid;
      mpReconstructedGrid = 0;
   }
   
   mpReconstructedGrid = MergeAndReconstructFFTBands( vReconstructedSubbandsFFT );
   ClearFFTVector( vReconstructedSubbandsFFT );
   
   //io.WritePGM( mpReconstructedGrid, string("FFTPyrReconstOriented.ppm"), ImageIO::NULL_OUT);
   
   //ArrayGrid<double>* pDiffGridFFT = NumberGridTools<double>::CreateSquaredErrorGrid( mpReconstructedGrid, mpSourceGrid, string("FFTOrientedDecRec"));
   
   //io.WritePGM( (ArrayGrid<double>*)(pDiffGridFFT), string("DiffOriGrid.ppm"), ImageIO::NORMAL_OUT);
   //delete pDiffGridFFT;
   delete mpTmpHighpassGrid;

   return true;
}

//------------------------------------------------------------------------------------------

bool PyramidComplex::Diagnose()
{
   int nrLevels = mpPyramid->GetNumberOfScales();
   
   for (int levelNr = 0; levelNr < nrLevels; levelNr++)
   {
      int nrOrientations = mpPyramid->GetRecursiveScale( levelNr )->GetNumberOfOrientations();
      for (int orientationNr = 0; orientationNr < nrOrientations; orientationNr++)
      {
         stringstream ss;
         ss << "BP-" <<  nrLevels << "-" << orientationNr;
         std::string fileName = ss.str();
         ArrayGrid<double>* pGrid = NumberGridTools<double>::CreateDoubleGridFromComplexGrid( mpPyramid->GetRecursiveScale( levelNr )->GetOrientedBand( orientationNr ) );
         GridStatistics<double>::DiagnoseReal( pGrid, fileName );
         delete pGrid;
      }
   }
   if ( mpPyramid->HasResidualScale() )
   {
      int nrOrientations = mpPyramid->GetResidualScale( )->GetNumberOfOrientations();
      for (int orientationNr = 0; orientationNr < nrOrientations; orientationNr++)
      {
         stringstream ss;
         ss << "BP-Residual-" << orientationNr;
         std::string fileName = ss.str();
         ArrayGrid<double>* pGrid = NumberGridTools<double>::CreateDoubleGridFromComplexGrid( mpPyramid->GetResidualScale( )->GetOrientedBand( orientationNr ) );
         GridStatistics<double>::DiagnoseReal( pGrid, fileName );
         delete pGrid;
      }
   }
   if ( mpPyramid->GetLowpassResidual( ) != 0)
   {
      ArrayGrid<double>* pGrid = mpPyramid->GetLowpassResidual( );
      GridStatistics<double>::DiagnoseReal( pGrid, std::string("Lowpassresidual" ) );
   }
   return true;
}


}
}
