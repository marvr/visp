/****************************************************************************
 *
 * $Id:  $
 *
 * Copyright (C) 1998-2010 Inria. All rights reserved.
 *
 * This software was developed at:
 * IRISA/INRIA Rennes
 * Projet Lagadic
 * Campus Universitaire de Beaulieu
 * 35042 Rennes Cedex
 * http://www.irisa.fr/lagadic
 *
 * This file is part of the ViSP toolkit
 *
 * This file may be distributed under the terms of the Q Public License
 * as defined by Trolltech AS of Norway and appearing in the file
 * LICENSE included in the packaging of this file.
 *
 * Licensees holding valid ViSP Professional Edition licenses may
 * use this file in accordance with the ViSP Commercial License
 * Agreement provided with the Software.
 *
 * This file is provided AS IS with NO WARRANTY OF ANY KIND, INCLUDING THE
 * WARRANTY OF DESIGN, MERCHANTABILITY AND FITNESS FOR A PARTICULAR PURPOSE.
 *
 * Contact visp@irisa.fr if any conditions of this licensing are
 * not clear to you.
 *
 * Description:
 * Detection of planar surface using Ferns classifier.
 *
 * Authors:
 * Romain Tallonneau
 *
 *****************************************************************************/
/*!
  \file planarObjectDetector.cpp

  \brief Tracking of planar surface using Ferns classifier.
*/

/*!
  \example planarObjectDetector.cpp

  Tracking of planar surface using Ferns classifier.
*/

#include <visp/vpConfig.h>
#include <visp/vpDebug.h>

#if ((defined (VISP_HAVE_X11) || defined(VISP_HAVE_GTK) || defined(VISP_HAVE_GDI)) && (VISP_HAVE_OPENCV_VERSION >= 0x020000))

#include <iostream>
#include <stdlib.h>
#include <visp/vpPlanarObjectDetector.h>
#include <visp/vpParseArgv.h>
#include <visp/vpConfig.h>
#include <visp/vpOpenCVGrabber.h>
#include <visp/vpImage.h>
#include <visp/vpDisplayX.h>
#include <visp/vpHomography.h>
#include <visp/vpImageIo.h>
#include <visp/vpIoTools.h>
#include <visp/vpTime.h>

#define GETOPTARGS  "hlcdb:"

/*!

  Print the program options.

  \param name : Program name.
  \param badparam : Bad parameter name.

*/
void usage(const char *name, const char *badparam)
{
  fprintf(stdout, "\n\
Test of detection of planar surface using Fenrs classifier. The object needs \
  first to be learned (-l option). This learning process will create a file used\
  to detect the object.\n\
\n\
SYNOPSIS\n\
  %s [-l] [-h] [-b] [-c] [-d] [-p]\n", name);

  fprintf(stdout, "\n\
OPTIONS:                                               \n\
  -l\n\
     learn an object.\n\
\n\
  -b\n\
     database filename to use (default is ./data).\n\
\n\
  -c\n\
     Disable the mouse click. Usefull to automaze the \n\
     execution of this program without humain intervention.\n\
\n\
  -d \n\
     Turn off the display.\n\
\n\
  -p \n\
     display points of interest.\n\
\n\
  -h\n\
     Print this help.\n");

  if (badparam)
    fprintf(stdout, "\nERROR: Bad parameter [%s]\n", badparam);

}

/*!

  Set the program options.

  \param argc : Command line number of parameters.
  \param argv : Array of command line parameters.
  \param isLearning : learning surface activation.
  \param dataFile : filename of the database.
  \param click_allowed : Mouse click activation.
  \param display : Display activation.
  \param displayPoints : Display points of interests activation.

  \return false if the program has to be stopped, true otherwise.

*/
bool getOptions(int argc, const char **argv,
        bool &isLearning, std::string& dataFile, bool& click_allowed, bool& display, bool& displayPoints)
{
  const char *optarg;
  int   c;
  while ((c = vpParseArgv::parse(argc, argv, GETOPTARGS, &optarg)) > 1) {

    switch (c) {
    case 'c': click_allowed = false; break;
    case 'd': display = false; break;
    case 'l': isLearning = true; break;
    case 'h': usage(argv[0], NULL); return false; break;
    case 'b': dataFile = optarg; break;
    case 'p': displayPoints = true; break;
    default:
      usage(argv[0], optarg);
      return false; break;
    }
  }

  if ((c == 1) || (c == -1)) {
    // standalone param or error
    usage(argv[0], NULL);
    std::cerr << "ERROR: " << std::endl;
    std::cerr << "  Bad argument " << optarg << std::endl << std::endl;
    return false;
  }

  return true;
}



int 
main(int argc, const char** argv)
{
  bool isLearning = false;
  std::string dataFile("./data");
  bool opt_click_allowed = true;
  bool opt_display = true;
  std::string objectName("object");
  bool displayPoints = false;

  // Read the command line options
	if (getOptions(argc, argv,
         isLearning, dataFile, opt_click_allowed, opt_display, displayPoints) == false) {
    exit (-1);
  }
  
  // Declare two images, these are gray level images (unsigned char)
  vpImage <unsigned char> I;
  vpImage <unsigned char> Iref;
  
  // Declare a framegrabber
  vpOpenCVGrabber g;
  
  try{
    g.open(I);
  }
  catch(...){
    std::cout << "problem to initialise the framegrabber" << std::endl;
    exit(-1);
  }
  g.acquire(I);
  // initialise the reference image
  g.acquire(Iref);

#if defined VISP_HAVE_X11
  vpDisplayX display;
#elif defined VISP_HAVE_GTK
  vpDisplayGTK display;
#elif defined VISP_HAVE_GDI
  vpDisplayGDI display;
#endif
  
#if defined VISP_HAVE_X11
  vpDisplayX displayRef;
#elif defined VISP_HAVE_GTK
  vpDisplayGTK displayRef;
#elif defined VISP_HAVE_GDI
  vpDisplayGDI displayRef;
#endif

  // declare a planar object detector
  vpPlanarObjectDetector planar;

//  planar.setDetectorParameters(2/*15*/, 1000, 2, 100, 11, 2, 32, 7, 100);
//  planar.useBlur(true);

  if(isLearning){  
    if(opt_display){
      displayRef.init(Iref, 100, 100, "Test vpPlanarObjectDetector reference image") ;
      vpDisplay::display(Iref);
      vpDisplay::flush(Iref);
    }
    vpImagePoint corners[2];
    if (opt_display && opt_click_allowed){
      std::cout << "Click on the top left and the bottom right corners to define the reference plane" << std::endl;
      for (int i=0 ; i < 2 ; i++){
        vpDisplay::getClick(Iref, corners[i]);
        std::cout << corners[i] << std::endl;
      }
    }
    else{
      corners[0].set_ij(1,1);
      corners[1].set_ij(I.getHeight()-2,I.getWidth()-2);
    }

    if (opt_display) {
      //Display the rectangle which defines the part of the image where the reference points are computed.
      vpDisplay::displayRectangle(Iref, corners[0], corners[1], vpColor::green);
      vpDisplay::flush(Iref);
    }

    if (opt_click_allowed){
      std::cout << "Click on the image to continue" << std::endl;
      vpDisplay::getClick(Iref);
    }  
  
    vpRect roi(corners[0], corners[1]);
  
    std::cout << "> train the classifier on the selected plane." << std::endl;
    if(opt_display) {
      vpDisplay::display(Iref);
      vpDisplay::flush(Iref);
    }
    planar.buildReference(Iref, roi); 
    planar.recordDetector(objectName, dataFile);
  }
  else{
    if(!vpIoTools::checkFilename(dataFile)){
      vpERROR_TRACE("cannot load the database with the specified name. Has the object been learned with the -l option? ");
      exit(-1);
    }
    try{
      // load a previously recorded file
      planar.load(dataFile, objectName);
    }
    catch(...){
      vpERROR_TRACE("cannot load the database with the specified name. Has the object been learned with the -l option? ");
      exit(-1);
    }
  }
  
  
  if(opt_display){
    display.init(I, 100, 100, "Test vpPlanarObjectDetector") ;
    vpDisplay::display(I);
    vpDisplay::flush(I);
  }
  
  while(1){
    // acquire a new image
    g.acquire(I);
    
    if(opt_display){
      vpDisplay::display(I);
    }
    
    double t0 = vpTime::measureTimeMs (); 
    // detection  of the reference planar surface
    bool isDetected = planar.matchPoint(I);
    std::cout << "matching: " << vpTime::measureTimeMs () - t0 << " ms" << std::endl;
    
    if(isDetected){
      vpHomography H;
      planar.getHomography(H);    
      std::cout << " computed homography:" << std::endl << H << std::endl;
      if(opt_display){
        planar.display(Iref, I); 
      }
    }
    else{
      std::cout << " reference is not detected in the image" << std::endl;
    }
    if(opt_display){
      vpDisplay::flush(I);
      if(vpDisplay::getClick(I, false)){
        break;
      }
    }
  }
  
  return 0;
}

#else
int
main()
{
#if ( ! (defined (VISP_HAVE_X11) || defined(VISP_HAVE_GTK) || defined(VISP_HAVE_GDI)) ) 
  vpERROR_TRACE("You do not have X11, GTK or GDI display functionalities...");
#else
  vpERROR_TRACE("You do not have OpenCV-2.0.0 or a more recent release...");
#endif
}

#endif